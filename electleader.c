/*
Byron Henze
66809088
Assignment 3 - leader election
*/

#include "mpi.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
	
	int rank, num;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num);
	int pnum = atoi(argv[1]);
	int uid = (rank + 1) * pnum % num;

	/*
	MPI_Send(void* data, int count, MPI_Datatype datatype, int destination,
		int tag, MPI_Comm communicator)
	MPI_Recv(void* data, int count, MPI_Datatype datatype, int source,
        int tag, MPI_Comm communicator, MPI_Status* status)
	Params: data buffer, count of elements in buffer, type of elements in buffer
			rank of sending/receiving process, tag of message
	*/

	int phase = 0;

	// int token;
	// if (world_rank != 0) {
	// 	MPI_Recv(&token, 1, MPI_INT, world_rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	// 	printf("Process %d received token %d from process %d\n", world_rank, token, world_rank - 1);
	// } else {
	// 	token = -1;
	// }

	// MPI_Send(&token, 1, MPI_INT, (world_rank + 1) % world_size, 0, MPI_COMM_WORLD);

	// if (world_rank == 0) {
	// 	MPI_Recv(&token, 1, MPI_INT, world_size - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	// 	printf("Process %d received token %d from process %d\n", world_rank, token, world_size - 1);
	// }

	printf("num %d, rank %d, pnum %d, uid %d\n", num, rank, pnum, uid);

	MPI_Finalize();
	return 0;

}