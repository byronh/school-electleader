/*
Byron Henze
66809088
Assignment 3 - leader election using Hirshberg-Sinclair (HS) algorithm
*/

#include "mpi.h"
#include <stdio.h>

#define TRUE 1
#define FALSE 0

#define T_ELECTION 0
#define T_REPLY 1
#define T_LEADER 2

#define I_TYPE 0
#define I_UID 1
#define I_PHASE 2
#define I_DIST 3

int uid, lrank, rrank;
int msgs_sent = 0;
int msgs_recvd = 0;

int ipow(int base, int exp);
void send_msg(int* data, int destination, MPI_Request request, MPI_Status status);
void recv_msg(int* data, int source, MPI_Status status);
void print_sent_msg(int send[], int left);
void print_recv_msg(int recv[], int left);

// Usage: mpiexec -n NUM ./electleader PNUM
int main(int argc, char* argv[]) {
	
	int election_complete = FALSE;
	int rank, num;

	// Initialize rank and uid
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num);
	int pnum = atoi(argv[argc-1]);
	uid = ((rank + 1) * pnum) % num;

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
	send_msg(lsend, lrank, lreq, lstat);
	send_msg(rsend, rrank, rreq, rstat);

	while (!election_complete) {
		recv_msg(lrecv, lrank, lstat);
		recv_msg(rrecv, rrank, rstat);

		// Handle messages from the left
		switch (lrecv[I_TYPE]) {
			case T_LEADER:
				election_complete = TRUE;
				break;
			case T_ELECTION:
				if ((lrecv[I_UID] > uid) && (lrecv[I_DIST] <= ipow(2, lrecv[I_PHASE]))) {
					printf("%d sending election to the right\n", uid);
					// rsend = {T_ELECTION, uid, phase, dist + 1};
					// MPI_Isend(&rsend, 4, MPI_INT, rrank, 0, MPI_COMM_WORLD, &rreq);
					// MPI_Wait(&rreq, &rstat);
					// msgs_sent++;
				}
				break;
			default:
				printf("%d received invalid message type\n", uid); print_recv_msg(lrecv);
				break;
		}

		if (election_complete) break;

		// Handle messages from the right
		switch (rrecv[I_TYPE]) {
			case T_LEADER:
				election_complete = TRUE;
				break;
			case T_ELECTION:
				if ((rrecv[I_UID] > uid) && (rrecv[I_DIST] <= ipow(2, rrecv[I_PHASE]))) {
					printf("%d sending election to the left\n", uid);
				}
				break;
			default:
				printf("%d received invalid message type\n", uid); print_recv_msg(rrecv);
				break;
		}

		election_complete = TRUE;
	}

	// Election complete
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
	print_sent_msg(data, destination == lrank);
	MPI_Isend(data, 4, MPI_INT, destination, 0, MPI_COMM_WORLD, &request);
	MPI_Wait(&request, &status);
	msgs_sent++;
}

void recv_msg(int* data, int source, MPI_Status status) {
	MPI_Recv(data, 4, MPI_INT, source, 0, MPI_COMM_WORLD, &status);
	print_recv_msg(data, destination == lrank);
	msgs_recvd++;
}

void print_sent_msg(int send[], int left) {
	printf("%d sent to %s: {%s, uid:%d, phase:%d, dist:%d}\n", uid,
		left ? "L" : "R",
		send[I_TYPE] == T_REPLY ? "REPLY" : send[I_TYPE] == T_ELECTION ? "ELECT" : "LEADER",
		send[I_UID], send[I_PHASE], send[I_DIST]);
}

void print_recv_msg(int recv[], int left) {
	printf("%d received from %s: {%s, uid:%d, phase:%d, dist:%d}\n", uid,
		left ? "L" : "R",
		recv[I_TYPE] == T_REPLY ? "REPLY" : recv[I_TYPE] == T_ELECTION ? "ELECT" : "LEADER",
		recv[I_UID], recv[I_PHASE], recv[I_DIST]);
}