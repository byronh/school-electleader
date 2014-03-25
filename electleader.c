/*
Byron Henze
66809088
Assignment 3 - leader election using Hirshberg-Sinclair (HS) algorithm

Algorithm pseudo-code from http://www.cs.rug.nl/~eirini/DS_slides/leader_election.pdf

To initiate an election (phase 0):
	send(ELECTION, my_id, 0, 0) to left and right

Upon receiving a message ELECTION, j, k, d from left (right):
	if ((j > my_id) && (d <= 2^k)):
		send(ELECTION, j, k, d + 1) to right (left)
	if ((j > my_id) && (d == 2^k)):
		send(REPLY, j, k) to left (right)
	if (my_id = j):
		announce self as leader

Upon receiving a message REPLY, j, k from left (right):
	if (my_id != j):
		send(REPLY, j, k) to right (left)
	else if (already received REPLY, j, k):
		send(ELECTION, j, k + 1, 1) to left and right;
*/

#include "mpi.h"
#include <stdio.h>
#include <string.h>

#define bool int
#define TRUE 1
#define FALSE 0

#define DEBUG 0

#define T_NULL 0
#define T_ELECTION 1
#define T_REPLY 2
#define T_LEADER 3

// TYPE = message type, J = uid, K = phase, D = distance
#define I_TYPE 0
#define I_J 1
#define I_K 2
#define I_D 3

// Smelly global variables... but who cares, it's C !
int uid, lrank, rrank;
int msgs_sent = 0;
int msgs_recvd = 0;
int lsent = 0;
int rsent = 0;
bool election_complete = FALSE;
bool ldone = FALSE;
bool rdone = FALSE;
MPI_Request lreq, rreq;
MPI_Status lstat, rstat;

// Integer power function - from http://stackoverflow.com/questions/101439/
int ipow(int base, int exp);

// Sends a message to the process in the ring that is to the left of this process
void send_left(int* data);

// Sends a message to the process in the ring that is to the right of this process
void send_right(int* data);

// Sends a message to the process with rank equal to 'destination'
void send_msg(int* data, int destination, MPI_Request request, MPI_Status status);

// Receives a message from the process with rank equal to 'source'
void recv_msg(int* data, int source, MPI_Status status);

// Prints debug information about a sent message
void print_sent_msg(int send[], int dest);

// Prints debug information about a received message
void print_recv_msg(int recv[], int source);

// Usage: mpiexec -n NUM ./electleader PNUM
int main(int argc, char* argv[]) {
	int rank, num;

	// Initialize rank and uid
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num);
	int pnum = atoi(argv[argc-1]);
	//uid = ((rank + 1) * pnum) % num;
	uid = rank;

	// Initialize neighbor ranks
	lrank = (rank - 1) % num;
	if (lrank < 0) lrank += num;
	rrank = (rank + 1) % num;
	if (rrank < 0) rrank += num;

	int leader = -1;
	int phase = 0;
	int dist = 0;

	// Initialize 2-dimensional array to keep track of replies(j,k) already received
	int replies[num][num];
	memset(replies, FALSE, sizeof replies);

	// Begin phase 0 election
	int lsend[4] = {T_ELECTION, uid, phase, dist};
	int rsend[4] = {T_ELECTION, uid, phase, dist};
	int lrecv[4] = {0, 0, 0, 0};
	int rrecv[4] = {0, 0, 0, 0};
	send_msg(lsend, lrank, lreq, lstat);
	send_msg(rsend, rrank, rreq, rstat);

	int t, j, k, d;

	while (!election_complete) {
		lsent = 0;
		rsent = 0;

		if (ldone || rdone) election_complete = TRUE;

		if (!election_complete) {
			recv_msg(lrecv, lrank, lstat);
			
			t = lrecv[I_TYPE];
			j = lrecv[I_J];
			k = lrecv[I_K];
			d = lrecv[I_D];

			// Handle messages from the left
			if (t == T_LEADER && leader == -1) {
				ldone = TRUE;
				leader = j;
				if (DEBUG) printf("** %d acknowledges leader %d\n", uid, leader);
				if (!rdone) {
					int data[4] = {T_LEADER, leader, -1, -1};
					send_right(data);
				}
			} else if (t == T_ELECTION) {
				if (!rdone && (j > uid) && (d <= ipow(2, k))) {
					int election[4] = {T_ELECTION, j, k, d + 1};
					send_right(election);
				}
				else if (!ldone && (j > uid) && (d == ipow(2, k))) {
					int reply[4] = {T_REPLY, j, k, -1};
					send_left(reply);
				}
				if (uid == j) {
					leader = uid;
					int announce[4] = {T_LEADER, uid, -1, -1};
					send_left(announce);
					send_right(announce);
					ldone = TRUE;
					rdone = TRUE;
				}
			} else if (t == T_REPLY) {
				if (uid != j) {
					replies[j][k] = TRUE;
					int reply[4] = {T_REPLY, j, k, -1};
					if (!rdone) send_right(reply);
				}
				if (replies[j][k] == TRUE) {
					printf("%d to next phase\n", uid);
					int election[4] = {T_ELECTION, j, k + 1, 1};
					if (!ldone) send_left(election);
					if (!rdone) send_right(election);
				}
			}
		}

		if (ldone || rdone) election_complete = TRUE;

		if (!election_complete) {
			recv_msg(rrecv, rrank, rstat);
			t = rrecv[I_TYPE];
			j = rrecv[I_J];
			k = rrecv[I_K];
			d = rrecv[I_D];

			// Handle messages from the right
			if (t == T_LEADER && leader == -1) {
				rdone = TRUE;
				leader = j;
				if (DEBUG) printf("** %d acknowledges leader %d\n", uid, leader);
				if (!ldone) {
					int data[4] = {T_LEADER, leader, -1, -1};
					send_left(data);
				}
			} else if (t == T_ELECTION) {
				if (!ldone && (j > uid) && (d <= ipow(2, k))) {
					int election[4] = {T_ELECTION, j, k, d + 1};
					send_left(election);
				}
				else if (!rdone && (j > uid) && (d == ipow(2, k))) {
					int reply[4] = {T_REPLY, j, k, -1};
					send_right(reply);
				}
				if (uid == j) {
					leader = uid;
					int announce[4] = {T_LEADER, uid, -1, -1};
					send_left(announce);
					send_right(announce);
					ldone = TRUE;
					rdone = TRUE;
				}
			} else if (t == T_REPLY) {
				if (uid != j) {
					replies[j][k] = TRUE;
					int reply[4] = {T_REPLY, j, k, -1};
					if (!ldone) send_left(reply);
				}
				if (replies[j][k] == TRUE) {
					printf("%d to next phase\n", uid);
					int election[4] = {T_ELECTION, j, k + 1, 1};
					if (!ldone) send_left(election);
					if (!rdone) send_right(election);
				}
			}
		}
	}

	// Election complete
	printf("rank=%d, id=%d, leader=%d, mrcvd=%d, msent=%d\n", rank, uid, leader == uid ? 1 : 0, msgs_recvd, msgs_sent);

	// Another messaging round to send total messages to leader



	MPI_Finalize();
	return 0;

}

// Integer power function - from http://stackoverflow.com/questions/101439/
int ipow(int base, int exp) {
    int result = 1;
    while (exp) {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }
    return result;
}

// Sends a message to the process in the ring that is to the left of this process
void send_left(int* data) {
	send_msg(data, lrank, lreq, lstat);
}

// Sends a message to the process in the ring that is to the right of this process
void send_right(int* data) {
	send_msg(data, rrank, rreq, rstat);
}

// Sends a message to the process with rank equal to 'destination'
void send_msg(int* data, int destination, MPI_Request request, MPI_Status status) {
	if (destination == lrank) lsent++;
	else if (destination == rrank) rsent++;
	print_sent_msg(data, destination);
	MPI_Isend(data, 4, MPI_INT, destination, 0, MPI_COMM_WORLD, &request);
	MPI_Wait(&request, &status);
	msgs_sent++;
}

// Receives a message from the process with rank equal to 'source'
void recv_msg(int* data, int source, MPI_Status status) {
	MPI_Recv(data, 4, MPI_INT, source, 0, MPI_COMM_WORLD, &status);
	print_recv_msg(data, source);
	msgs_recvd++;
}

// Prints debug information about a sent message
void print_sent_msg(int send[], int dest) {
	if (!DEBUG || send[I_TYPE] == T_NULL) return;
	printf("%d sent to %d %s: {%d, uid:%d, phase:%d, dist:%d}\n", uid, dest,
		dest == lrank ? "L" : "R",
		send[I_TYPE], send[I_J], send[I_K], send[I_D]);
}

// Prints debug information about a received message
void print_recv_msg(int recv[], int source) {
	if (!DEBUG || recv[I_TYPE] == T_NULL) return;
	printf("%d received from %d %s: {%d, uid:%d, phase:%d, dist:%d}\n", uid, source,
		source == lrank ? "L" : "R",
		recv[I_TYPE], recv[I_J], recv[I_K], recv[I_D]);
}