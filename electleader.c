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

int ipow(int base, int exp);
void printmsg(int recv[]);

// Usage: mpiexec -n NUM ./electleader PNUM
int main(int argc, char* argv[]) {
	
	int election_complete = FALSE;
	int rank, num;
	int msgs_sent = 0;
	int msgs_recvd = 0;

	// Initialize rank and uid
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num);
	int pnum = atoi(argv[argc-1]);
	int uid = ((rank + 1) * pnum) % num;

	// Initialize neighbor ranks
	int lrank = (rank - 1) % num;
	if (lrank < 0) lrank += num;
	int rrank = (rank + 1) % num;
	if (rrank < 0) rrank += num;

	/*
	MPI_Send(void* data, int count, MPI_Datatype datatype, int destination,
		int tag, MPI_Comm communicator)
	MPI_Recv(void* data, int count, MPI_Datatype datatype, int source,
        int tag, MPI_Comm communicator, MPI_Status* status)
	Params: data buffer, count of elements in buffer, type of elements in buffer
			rank of sending/receiving process, tag of message
	*/

	int phase = 0;
	int dist = 0;

	int lsend[4] = {T_ELECTION, uid, phase, dist};
	int rsend[4] = {T_ELECTION, uid, phase, dist};
	int lrecv[4] = {0, 0, 0, 0};
	int rrecv[4] = {0, 0, 0, 0};

	MPI_Request lreq, rreq;
	MPI_Status lstat, rstat;

	// Initiate election
	MPI_Isend(&lsend, 4, MPI_INT, lrank, 0, MPI_COMM_WORLD, &lreq);
	MPI_Isend(&rsend, 4, MPI_INT, rrank, 0, MPI_COMM_WORLD, &rreq);
	msgs_sent += 2;
	MPI_Wait(&lreq, &lstat);
	MPI_Wait(&rreq, &rstat);

	while (!election_complete) {

		MPI_Recv(&lrecv, 4, MPI_INT, lrank, 0, MPI_COMM_WORLD, &lstat);
		MPI_Recv(&rrecv, 4, MPI_INT, rrank, 0, MPI_COMM_WORLD, &rstat);
		msgs_recvd += 2;
		printf("%d received L:", uid); printmsg(lrecv);
		printf("%d received R:", uid); printmsg(rrecv);

		// j = uid, k = phase, d = dist
		// Handle messages from the left
		if (lrecv[I_TYPE] == T_ELECTION) {
			if ((lrecv[I_UID] > uid) && (lrecv[I_DIST] <= ipow(2, lrecv[I_PHASE]))) {
				printf("%d sending election to the right\n", uid);
			}
		}

		// Handle messages from the right
		if (rrecv[I_TYPE] == T_ELECTION) {
			if ((rrecv[I_UID] > uid) && (rrecv[I_DIST] <= ipow(2, rrecv[I_PHASE]))) {
				printf("%d sending election to the left\n", uid);
			}
		}

		// Temp: Always make first guy leader
		election_complete = TRUE;
		// if (uid == 0) {
		// 	MPI_Isend(&rsend, 4, MPI_INT, rrank, 0, MPI_COMM_WORLD, &rreq);
		// 	msgs_sent += 2;
		// 	MPI_Wait(&lreq, &lstat);
		// }
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

void printmsg(int recv[]) {
	printf(" {%s, uid:%d, phase:%d, dist:%d}\n", recv[I_TYPE] == T_REPLY ? "REPLY" : "ELECT",
		recv[I_UID], recv[I_PHASE], recv[I_DIST]);
}