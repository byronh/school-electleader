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

#define bool int
#define TRUE 1
#define FALSE 0

#define T_NULL 0
#define T_ELECTION 1
#define T_REPLY 2
#define T_LEADER 3

#define I_TYPE 0
#define I_UID 1
#define I_PHASE 2
#define I_DIST 3

int uid, lrank, rrank;
int msgs_sent = 0;
int msgs_recvd = 0;
int lsent = 0;
int rsent = 0;
bool election_complete = FALSE;
bool ldone = FALSE;
bool rdone = FALSE;

int ipow(int base, int exp);
void send_msg(int* data, int destination, MPI_Request request, MPI_Status status);
void recv_msg(int* data, int source, MPI_Status status);
void print_sent_msg(int send[], int dest);
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

	int phase = 0;
	int dist = 0;

	int lsend[4] = {T_ELECTION, uid, phase, dist};
	int rsend[4] = {T_ELECTION, uid, phase, dist};
	int lrecv[4] = {0, 0, 0, 0};
	int rrecv[4] = {0, 0, 0, 0};

	MPI_Request lreq, rreq;
	MPI_Status lstat, rstat;

	// Initiate phase 0 election
	int leader = -1;
	send_msg(lsend, lrank, lreq, lstat);
	send_msg(rsend, rrank, rreq, rstat);

	int i=0;
	while (!election_complete) {
		lsent = 0;
		rsent = 0;
		recv_msg(lrecv, lrank, lstat);
		recv_msg(rrecv, rrank, rstat);

		// if (uid == 0 && i>0) {
		// 	int data[4] = {T_LEADER, uid, -1, -1};
		// 	send_msg(data, lrank, lreq, lstat);
		// 	send_msg(data, rrank, rreq, rstat);
		// 	break;
		// }

		// Handle messages from the left
		if (lrecv[I_TYPE] == T_LEADER) {
			ldone = TRUE;
			leader = lrecv[I_UID];
			printf("** %d acknowledges leader %d\n", uid, leader);
			if (!rdone) {
				int data[4] = {T_LEADER, leader, -1, -1};
				send_msg(data, rrank, rreq, rstat);
			}
			//break;
		} else if (lrecv[I_TYPE] == T_ELECTION) {
			if (!rdone && (lrecv[I_UID] > uid) && (lrecv[I_DIST] <= ipow(2, lrecv[I_PHASE]))) {
				int data[4] = {T_ELECTION, uid, phase, dist + 1};
				send_msg(data, rrank, rreq, rstat);
			}
		}

		// Handle messages from the right
		if (rrecv[I_TYPE] == T_LEADER) {
			rdone = TRUE;
			leader = rrecv[I_UID];
			printf("** %d acknowledges leader %d\n", uid, leader);
			if (!ldone) {
				int data[4] = {T_LEADER, leader, -1, -1};
				send_msg(data, lrank, lreq, lstat);
			}
			//break;
		} else if (rrecv[I_TYPE] == T_ELECTION) {
			if (!ldone && (rrecv[I_UID] > uid) && (rrecv[I_DIST] <= ipow(2, rrecv[I_PHASE]))) {
				int data[4] = {T_ELECTION, uid, phase, dist + 1};
				send_msg(data, lrank, lreq, lstat);
			}
		}

		if (ldone || rdone) election_complete = TRUE;
		i++;

		// Send null messages to prevent deadlock
		if (lsent == 0) {
			int data[4] = {T_NULL, uid, 0, 0};
			send_msg(data, lrank, lreq, lstat);
		}
		if (rsent == 0) {
			int data[4] = {T_NULL, uid, 0, 0};
			send_msg(data, rrank, rreq, rstat);
		}
	}

	// Election complete
	//printf("**%d is done\n", uid);
	//printf("rank=%d, id=%d, leader=0, mrcvd=%d, msent=%d\n", rank, uid, msgs_recvd, msgs_sent);

	MPI_Finalize();
	return 0;

}

// Borrowed from http://stackoverflow.com/questions/101439/
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

void send_msg(int* data, int destination, MPI_Request request, MPI_Status status) {
	if (destination == lrank) lsent++;
	else if (destination == rrank) rsent++;
	print_sent_msg(data, destination);
	MPI_Isend(data, 4, MPI_INT, destination, 0, MPI_COMM_WORLD, &request);
	MPI_Wait(&request, &status);
	msgs_sent++;
}

void recv_msg(int* data, int source, MPI_Status status) {
	MPI_Recv(data, 4, MPI_INT, source, 0, MPI_COMM_WORLD, &status);
	print_recv_msg(data, source);
	msgs_recvd++;
}

void print_sent_msg(int send[], int dest) {
	if (send[I_TYPE] == T_NULL) return;
	printf("%d sent to %d %s: {%d, uid:%d, phase:%d, dist:%d}\n", uid, dest,
		dest == lrank ? "L" : "R",
		send[I_TYPE], send[I_UID], send[I_PHASE], send[I_DIST]);
}

void print_recv_msg(int recv[], int source) {
	if (recv[I_TYPE] == T_NULL) return;
	printf("%d received from %d %s: {%d, uid:%d, phase:%d, dist:%d}\n", uid, source,
		source == lrank ? "L" : "R",
		recv[I_TYPE], recv[I_UID], recv[I_PHASE], recv[I_DIST]);
}