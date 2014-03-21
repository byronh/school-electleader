#include "mpi.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
	
	int world_rank, world_size, name_len;
	char name[MPI_MAX_PROCESSOR_NAME];

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Get_processor_name(name, &name_len);

	int token;
	if (world_rank != 0) {
		MPI_Recv(&token, 1, MPI_INT, world_rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("Process %d received token %d from process %d\n", world_rank, token, world_rank - 1);
	} else {
		token = -1;
	}

	MPI_Send(&token, 1, MPI_INT, (world_rank + 1) % world_size, 0, MPI_COMM_WORLD);

	if (world_rank == 0) {
		MPI_Recv(&token, 1, MPI_INT, world_size - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("Process %d received token %d from process %d\n", world_rank, token, world_size - 1);
	}

	printf("%s[%d]: rank %d\n", name, world_size, world_rank);

	MPI_Finalize();
	return 0;

}