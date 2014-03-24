/*
Byron Henze
66809088
Assignment 3 - leader election using Hirshberg-Sinclair (HS) algorithm
*/

#include "mpi.h"
#include <stdio.h>

#define T_ELECTION 0
#define T_REPLY 1

// Usage: mpiexec -n NUM ./electleader PNUM
int main(int argc, char* argv[]) {
	
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

	int lsend[3] = {uid, phase, dist};
	int rsend[3] = {uid, phase, dist};
	int lrecv[3] = {0, 0, 0};
	int rrecv[3] = {0, 0, 0};

	MPI_Request lreq, rreq;
	MPI_Status lstat, rstat;

	// Phase 0 election
	MPI_Isend(&lsend, 3, MPI_INT, lrank, T_ELECTION, MPI_COMM_WORLD, &lreq);
	MPI_Isend(&rsend, 3, MPI_INT, rrank, T_ELECTION, MPI_COMM_WORLD, &rreq);
	msgs_sent += 2;
	MPI_Wait(&lreq, &lstat);
	MPI_Wait(&rreq, &rstat);

	MPI_Recv(&lrecv, 3, MPI_INT, lrank, T_ELECTION, MPI_COMM_WORLD, &lstat);
	MPI_Recv(&rrecv, 3, MPI_INT, rrank, T_ELECTION, MPI_COMM_WORLD, &rstat);
	msgs_recvd += 2;
	printf("%d received L election from %d: {%d, %d, %d}\n", rank, lrank, lrecv[0], lrecv[1], lrecv[2]);
	printf("%d received R election from %d: {%d, %d, %d}\n", rank, rrank, rrecv[0], rrecv[1], rrecv[2]);


	// Election complete
	//printf("rank=%d, id=%d, leader=0, mrcvd=%d, msent=%d\n", rank, uid, msgs_recvd, msgs_sent);

	MPI_Finalize();
	return 0;

}