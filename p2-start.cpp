/**
 * @author Divya Sheri
 * @cwid   123 45 678
 * @class  CSci 530, Summer 2015
 * @ide    Eclipse
 * @date   June 26, 2015
 * @assg   prog-02
 *
 * @description This program implements the Banker's algorithm for performing deadlock avoidance.  It
 *     reads in a file describing the initial state of the system, and a request from a process for
 *     some additional resource allocations.  It uses's the algorithm to determine whether the
 *     resulting state would be safe or unsafe to determine whether the request should be granted.
 */
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
using namespace std;

// global constants
const int MAX_PROCESSES = 10;
const int MAX_RESOURCES = 10;

// system resource & process state structure
typedef struct {
	int numResources; // index m in book, our resources are labeled from 0 to m-1 in this program
	int numProcesses; // index n in book, our processes are labeled form 0 to n-1 in this program
	int resource[MAX_RESOURCES];
	int available[MAX_RESOURCES];
	int claim[MAX_PROCESSES][MAX_RESOURCES];
	int alloc[MAX_PROCESSES][MAX_RESOURCES];
} State;

/** Read state and allocation request from file.
 * Given a file, read the current system state and request for a new
 * allocation from the file. You need to pass in 3 parameters that
 * are actually used to hold the results of calling this function.
 * As a side effect of calling this function, the state, process and request
 * values will be filled in and returned to the caller.
 *
 * @param simfilename The name of the file to open and read state & request
 *         from.
 * @param state A return value, will hold the state we read in from the file.
 * @param process A return value, will hold the process that is making an
 *         allocation request.
 * @param request A return value, the vector will hold the request that is
 *         being made by the process.
 *
 */
void readSystemState(char* simfilename, State* state, int* process,
		int request[]) {
	ifstream simstatefile(simfilename);
	int r, p;

	// If we can't open file, abort and let the user know problem
	if (!simstatefile.is_open()) {
		cout << "Error: could not open system state file: " << simfilename
				<< endl;
		exit(1);
	}

	// Format of file is this (where m = numResource n = numProcesses
	//                         R = resource vector, V = available vector
	//                         C = claim matrix and A = allocation matrix and
	//                         p = process (0 -- n-1) making new request
	//                         Q = request vector)
	// m n
	// R1 R2 R3 ... Rm
	// V1 V2 V3 ... Vm
	// C11 C12 ... C1m
	// C21 C22 ... C2m
	// ...
	// Cn1 Cn2 ... Cnm
	// A11 A12 ... A1m
	// A21 A22 ... A2m
	// ...
	// An1 An2 ... Anm
	// p
	// Q1 Q2 Q3 ... Qm

	// First line, get m (numResources) and n (numProcesses)
	simstatefile >> state->numResources >> state->numProcesses;

	// Next line contains the Resource vector R
	for (r = 0; r < state->numResources; r++) {
		simstatefile >> state->resource[r];
	}

	// Next line contains the available vector V
	for (r = 0; r < state->numResources; r++) {
		simstatefile >> state->available[r];
	}

	// Next n lines contain the claim matrix C
	for (p = 0; p < state->numProcesses; p++) {
		for (r = 0; r < state->numResources; r++) {
			simstatefile >> state->claim[p][r];
		}
	}

	// Next n lines contain the allocation matrix A
	for (p = 0; p < state->numProcesses; p++) {
		for (r = 0; r < state->numResources; r++) {
			simstatefile >> state->alloc[p][r];
		}
	}

	// Next line contains the process id making a new request, and the
	// vector of the resources it is requesting
	simstatefile >> *process;
	for (r = 0; r < state->numResources; r++) {
		simstatefile >> request[r];
	}

}

/** Display a vector
 * Display a state vector to standard output
 *
 * @param len The number of items in the vector
 * @param v An array of integers of len items
 */
void displayVector(int len, int v[]) {
	int i;

	// Display a header
	for (i = 0; i < len; i++) {
		cout << "R" << i << " ";
	}
	cout << endl;

	// Display values
	for (i = 0; i < len; i++) {
		cout << setw(2) << v[i] << " ";
	}
	cout << endl;
}

/** Display a matrix
 * Display a state matrix to standard output
 *
 * @param rows The number of rows in the matrix
 * @param cols The number of cols in the matrix
 * @param m A 2 dimensional array of rows x cols integers
 */
void displayMatrix(int rows, int cols, int v[MAX_PROCESSES][MAX_RESOURCES]) {
	int r, c;

	// display column headers
	cout << "   "; // extra space over for row labels
	for (c = 0; c < cols; c++) {
		cout << "R" << c << " ";
	}
	cout << endl;

	// now display data in matrix
	for (r = 0; r < rows; r++) {
		cout << "P" << r << " ";
		for (c = 0; c < cols; c++) {
			cout << setw(2) << v[r][c] << " ";
		}
		cout << endl;
	}
	cout << endl;
}

/** Display state
 * Display the values of the resource vectors and matrices in the indicated
 * state structure
 *
 * @param state A State struct whose info we should display on stdout.
 */
void displayState(State s) {
	cout << "numResources (m) = " << s.numResources << " ";
	cout << "numProcesses (n) = " << s.numProcesses << endl << endl;

	cout << "Resource vector R:" << endl;
	displayVector(s.numResources, s.resource);
	cout << endl;

	cout << "Available vector V:" << endl;
	displayVector(s.numResources, s.available);
	cout << endl;

	cout << "Claim matrix C: " << endl;
	displayMatrix(s.numProcesses, s.numResources, s.claim);
	cout << endl;

	cout << "Allocation matrix A: " << endl;
	displayMatrix(s.numProcesses, s.numResources, s.alloc);
	cout << endl;

}

/** Newly created functions
 *
 * Checks if alloc + request > claim
 *
 *@param State s is the state structure to check if it is Error state
 *@param int request[] is request vector to check with the state
 *@param int process is the process number the vector related to
 */
bool isError(State s, int request[], int process) {
	bool returnB = false;

	//Iterate through resource of particular process represented by 'process' variable
	for (int i = 0; i < s.numResources; i++) {
		//checks if alloc + request > claim
		if ((request[i] + s.alloc[process][i]) > s.claim[process][i]) {
			returnB = true;
			break;
		}
	}

	return returnB;
}

/**
 *
 * Checks if request > available
 *
 *@param State s is the state to check is it is in Suspend state with the provided vector
 *@param int request[] keep the provided vector to be checked agains available vector
 */
bool isSuspend(State s, int request[]) {
	bool returnB = false;

	//Iterate through available resources
	for (int i = 0; i < s.numResources; i++) {
		//checks if request > available
		if (request[i] > s.available[i]) {
			returnB = true;
			break;
		}
	}

	return returnB;
}

/**
 *
 * Checks if request > available
 *
 *@param State s is checked for safe state or not
 */
bool isSafe(State s) {
	//Iterates through each process
	for (int i = 0; i < s.numProcesses; i++) {
		//processed variable is used to check if alloc vector is already set to zero or not
		int processed = 0;
		// found variable is used to check if resources can be allocated or not.
		bool found = true;
		//Iterates through the resources of current processor represented by process number in 'i' variable
		for (int j = 0; j < s.numResources; j++) {
			//alloc values are added to the process variable for finding any non zero values
			processed += s.alloc[i][j];
			//checks if resources allocated can be freed or not
			if (s.claim[i][j] - s.alloc[i][j] > s.available[j]) {
				found = false;
			}
		}

		//if resource allocated for the process can be freed and it is not already processed then..
		//free the resources of the process
		if (found && processed > 0) {

			//iterate through the resources to free it
			for (int j = 0; j < s.numResources; j++) {
				//allocated resources is freed to available resource vectors
				s.available[j] += s.alloc[i][j];
				//processes' allocation is set to zero to represent is is already freed
				s.alloc[i][j] = 0;

			}

			//reset process number to start checking from first process again.
			i = -1;
		}
	}

	//Checks if all the alloc vectors are zero which represent all resources are freed
	//iterate through processes
	for (int i = 0; i < s.numProcesses; i++) {
		//iterate though resources
		for (int j = 0; j < s.numResources; j++) {
			//checks if the allocation is zero or not
			if (s.alloc[i][j] > 0) {

				return false;

			}
		}
	}

	return true;
}

/** Main entry point of Banker's Algorithm
 * The main entry point of the implementation of the Banker's Algorithm
 * assignment.  We expect the name of a file that is formatted to contain
 * the resources (R), available (A), claim (C) and allocation (A) description
 * of a proposed new system state.  We read in the file to a State
 * structure, then perform the Banker's Algorithm analysis on this state.
 *
 * @param argc The argument count
 * @param argv The command line argument values. We expect argv[1] to be the
 *              name of a file in the current directory holding a description
 *              of a proposed system state to analyze.
 */
int main(int argc, char** argv) {
	State state;
	int process;
	int request[MAX_RESOURCES];

	// If file name not provided, abort and let user know of problem
	if (argc != 2) {
		cout
				<< "Error: expecting system state file as first command line parameter"
				<< endl;
		cout << "Usage: " << argv[0] << " state.sim" << endl;
		exit(1);
	}

	// read system state and process allocation request from file
	readSystemState(argv[1], &state, &process, request);

	// The following displays the read in state and request, it is to help you
	// understand what was just read it, it should be commented out
	// before you submit your assignment to eCollege.
	cout << "Current System State:" << endl;
	displayState(state);
	cout << "Process request: P" << process << endl;
	cout << "Requested resources:" << endl;
	displayVector(state.numResources, request);

	// perform Banker's algorithm analysis on state here (fig 6.9 parts b & c).
	// if alloc + request > claim
	// {
	//   cout << "ERROR" << endl;
	// }
	// else if request > available
	// {
	//   cout << "SUSPEND" << endl;
	// }
	// else
	// {
	//   define newstate
	//   if (safe(newstate))
	//   {
	//     cout << "SAFE" << endl;
	//   }
	//   else
	//   {
	//     cout << "UNSAFE" << endl;
	//   }
	//

	if (isError(state, request, process)) {
		cout << "ERROR" << endl;
	} else if (isSuspend(state, request)) {
		cout << "SUSPEND" << endl;
	} else {
		// New state
		for (int i = 0; i < state.numResources; i++) {
			state.alloc[process][i] += request[i];
			state.available[i] -= request[i];
		}

		if (isSafe(state)) {
			cout << "SAFE" << endl;
		} else {
			cout << "UNSAFE" << endl;
		}

	}

}
