/*
Byron Henze
66809088
Assignment 3 - leader election
*/

#include "mpi.h"
#include <stdio.h>

// Usage: mpiexec -n NUM ./electleader PNUM
int main(int argc, char* argv[]) {
	
	int rank, num;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num);
	int pnum = atoi(argv[argc-1]);
	int uid = ((rank + 1) * pnum) % num;

	int messages_sent = 0;
	int messages_received = 0;

	int rank_left = (rank - 1) % num;
	if (rank_left < 0) rank_left += num;
	int rank_right = (rank + 1) % num;
	if (rank_right < 0) rank_right += num;

	/*
	MPI_Send(void* data, int count, MPI_Datatype datatype, int destination,
		int tag, MPI_Comm communicator)
	MPI_Recv(void* data, int count, MPI_Datatype datatype, int source,
        int tag, MPI_Comm communicator, MPI_Status* status)
	Params: data buffer, count of elements in buffer, type of elements in buffer
			rank of sending/receiving process, tag of message
	*/

	int phase = 0;
	int distance = 0;

	int leftSend[3] = {uid, phase, distance};
	int rightSend[3] = {uid, phase, distance};
	int leftRecv[3] = {0, 0, 0};
	int rightRecv[3] = {0, 0, 0};

	MPI_Request requestLeft, requestRight;
	MPI_Status statusLeft, statusRight;

	// Phase 0 election
	MPI_Isend(&leftSend, 3, MPI_INT, rank_left, 0, MPI_COMM_WORLD, &requestLeft);
	messages_sent++;
	//printf("Process %d send left message to process %d\n", rank, rank_left);
	MPI_Wait(&requestLeft, &statusLeft);

	MPI_Isend(&rightSend, 3, MPI_INT, rank_right, 0, MPI_COMM_WORLD, &requestRight);
	messages_sent++;
	//printf("Process %d send right message to process %d\n", rank, rank_right);
	MPI_Wait(&requestRight, &statusRight);

	MPI_Recv(&leftRecv, 3, MPI_INT, rank_left, 0, MPI_COMM_WORLD, &statusLeft);
	messages_received++;
	printf("Process %d received left message from process %d\n", rank, rank_left);

	MPI_Recv(&rightRecv, 3, MPI_INT, rank_right, 0, MPI_COMM_WORLD, &statusRight);
	printf("Process %d received right message from process %d\n", rank, rank_right);
	messages_received++;


	// End
	printf("rank=%d, id=%d, leader=0, mrcvd=%d, msent=%d\n", rank, uid,
		messages_received, messages_sent);

	MPI_Finalize();
	return 0;

}