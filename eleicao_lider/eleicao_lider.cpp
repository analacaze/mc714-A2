/*  Franklin's Algorithm For Undirected Rings
 *  Tasmiyah Iqbal
 */

#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <algorithm>

/*
 * The method aspires to generate a unique random number to be used as process id for each process.
 * P.S. I could have used MPI generated ranks as PIDs but then my logical topology would have been sequential
 * and hence also very predictable.  
 */
int generateRandomNumber(int rank, int size) {
		int s;
		int *rNums;
		rNums = new int[size];

		for(int i=0; i<size; i++)
		{
			rNums[i] = rand(); // aspires to be random number
		}

		s = rNums[rank]; 
		return s;
}

/*
 * The class representing a process in the system. Holds all necessary process info.
 */
class Process {
	private:
		int rank; // MPI generated
		int identifier; // assigned by this code
		bool status; // active or passive
	public:
		Process(int r, int nprocs) {
			rank = r;
			identifier = generateRandomNumber(rank, nprocs);
			status = true;
		}
		
		int getRank() {
			return rank;
		}

		void setRank(int r) {
			rank = r;
		}

		int getIdentifier() {
			return identifier;
		}

		void setIdentifier(int id) {
			identifier = id;
		}

		bool getStatus() {
			return status;
		}

		void setStatus(bool s) {
			status = s;
		}
};

int main(int argc, char** argv) {
    int myrank, nprocs;
	int my_id, left_id, right_id;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

	Process p(myrank, nprocs);
	printf("I am process %d of %d. Unique ID = %d\n", p.getRank(), nprocs, p.getIdentifier());
	my_id = p.getIdentifier();
	myrank = p.getRank();
    
	while(true) // loop to continue until a leader is elected
	{
		if (p.getStatus() == true) { // If an active node			
			if (myrank == 0) // Special boundary handling for first process in ring
			{
				MPI_Send(&my_id, 1, MPI_INT, myrank+1, 0, MPI_COMM_WORLD); // send my id to next process in ring
				MPI_Recv(&right_id, 1, MPI_INT, nprocs-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // recv from last process in ring

				MPI_Send(&my_id, 1, MPI_INT, nprocs-1, 0, MPI_COMM_WORLD); // send my id to previous neighbour in ring, I'm p0 so my prev is nprocs-1
				MPI_Recv(&left_id, 1, MPI_INT, myrank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // recv from next process in ring
			}
			else if (myrank == nprocs-1) // Special boundary handling for last process in ring
			{
				MPI_Send(&my_id, 1, MPI_INT, 0, 0, MPI_COMM_WORLD); // send to next process in ring, since I am last process therefore my next is p0
				MPI_Recv(&right_id, 1, MPI_INT, myrank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // recv from previous neighbour in ring

				MPI_Send(&my_id, 1, MPI_INT, myrank-1, 0, MPI_COMM_WORLD); // send to previous neighbour in ring
				MPI_Recv(&left_id, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // recv from next process in ring, since I am last process therefore my next is p0
			}
			else // Rest of the nodes (e.g p1,p2..)
			{
				MPI_Send(&my_id, 1, MPI_INT, myrank+1, 0, MPI_COMM_WORLD); // send to next process in ring
				MPI_Recv(&right_id, 1, MPI_INT, myrank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // recv from previous neighbour in ring

				MPI_Send(&my_id, 1, MPI_INT, myrank-1, 0, MPI_COMM_WORLD); // send to previous neighbour in ring
				MPI_Recv(&left_id, 1, MPI_INT, myrank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // recv from previous neighbour in ring
			}

			if (std::min(left_id, right_id) > my_id) // compare IDs to see if I am the smallest process in my *active* neighbourhood
			{
				p.setStatus(false); // enter relay mode if my ID is the smallest of the three
			}
			else if (std::min(left_id, right_id) == my_id) // If I am my own left and right neighbour i.e. all others have become passive by now then:
			{
				break; // break out the loop and declare yourself as leader
			}
		}
		
		// If I'm a passive node I will only pass msgs forward and backward in the ring. 
		// Also I will only be triggered by some active node (from the code above) hence I will be waiting for an incoming msg. 
		// Note the order of send() and recv() methods specifically to avoid deadlocked code!
		else if (p.getStatus() == false) { 	
			if (p.getRank() == 0) // Special boundary handling for first process in ring
			{
				MPI_Recv(&right_id, 1, MPI_INT, nprocs-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				MPI_Send(&right_id, 1, MPI_INT, myrank+1, 0, MPI_COMM_WORLD);

				MPI_Recv(&left_id, 1, MPI_INT, myrank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				MPI_Send(&left_id, 1, MPI_INT, nprocs-1, 0, MPI_COMM_WORLD);
			}
			else if (p.getRank() == nprocs-1) // Special boundary handling for last process in ring
			{
				MPI_Recv(&right_id, 1, MPI_INT, myrank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				MPI_Send(&right_id, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

				MPI_Recv(&left_id, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				MPI_Send(&left_id, 1, MPI_INT, myrank-1, 0, MPI_COMM_WORLD);
			}
			else // Rest of the nodes (e.g p1,p2..)
			{
				MPI_Recv(&right_id, 1, MPI_INT, myrank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // recv from previous neighbour
				MPI_Send(&right_id, 1, MPI_INT, myrank+1, 0, MPI_COMM_WORLD); // pass on to next neighbour (relay behaviour)

				MPI_Recv(&left_id, 1, MPI_INT, myrank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // recv from next neighbour
				MPI_Send(&left_id, 1, MPI_INT, myrank-1, 0, MPI_COMM_WORLD); // pass to previous neighbour
			}
		}
	}

    // If any process reaches this area in the code, it has discovered that it is its own left most and right most active neighbour.
	// Broadcast message to all threads to terminate themselves. Declare self as leader.
	printf("Process %d of %d is LEADER with highest Unique ID = %d\n", p.getRank(), nprocs, p.getIdentifier());
	MPI_Abort(MPI_COMM_WORLD, MPI_SUCCESS); // kill all other processes.
	MPI_Finalize(); // prepare to exit
    return 0;
}
