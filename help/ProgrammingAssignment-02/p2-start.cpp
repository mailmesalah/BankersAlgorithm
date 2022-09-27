/**
 * @author Jane Student
 * @cwid   123 45 678
 * @class  CSci 530, Summer 2015
 * @ide    Visual Studio Express 2010
 * @date   June 8, 2015
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
typedef struct
{
  int numResources;  // index m in book, our resources are labeled from 0 to m-1 in this program
  int numProcesses;  // index n in book, our processes are labeled form 0 to n-1 in this program
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
void readSystemState(char* simfilename, State* state, 
		     int* process, int request[])
{
  ifstream simstatefile(simfilename);
  int r, p;

  // If we can't open file, abort and let the user know problem
  if (!simstatefile.is_open())
  {
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
  for (r = 0; r < state->numResources; r++)
  {
    simstatefile >> state->resource[r]; 
  }

  // Next line contains the available vector V
  for (r = 0; r < state->numResources; r++)
  {
    simstatefile >> state->available[r]; 
  }
  
  // Next n lines contain the claim matrix C
  for (p = 0; p < state->numProcesses; p++)
  {
    for (r = 0; r < state->numResources; r++)
    {
      simstatefile >> state->claim[p][r];
    }
  }

  // Next n lines contain the allocation matrix A
  for (p = 0; p < state->numProcesses; p++)
  {
    for (r = 0; r < state->numResources; r++)
    {
      simstatefile >> state->alloc[p][r];
    }
  }

  // Next line contains the process id making a new request, and the
  // vector of the resources it is requesting
  simstatefile >> *process;
  for (r = 0; r < state->numResources; r++)
  {
    simstatefile >> request[r]; 
  }
  
}

/** Display a vector
 * Display a state vector to standard output
 *
 * @param len The number of items in the vector
 * @param v An array of integers of len items
 */
void displayVector(int len, int v[])
{
  int i;

  // Display a header
  for (i = 0; i < len; i++)
  {
    cout << "R" << i << " ";
  }
  cout << endl;

  // Display values
  for (i = 0; i < len; i++)
  {
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
void displayMatrix(int rows, int cols, int v[MAX_PROCESSES][MAX_RESOURCES])
{
  int r, c;

  // display column headers
  cout << "   "; // extra space over for row labels
  for (c = 0; c < cols; c++)
  {
    cout << "R" << c << " ";
  }
  cout << endl;
  
  // now display data in matrix
  for (r = 0; r < rows; r++)
  {
    cout << "P" << r << " ";
    for (c = 0; c < cols; c++)
    {
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
void displayState(State s)
{
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
int main(int argc, char** argv)
{
  State state;
  int process;
  int request[MAX_RESOURCES];

  // If file name not provided, abort and let user know of problem
  if (argc != 2)
  {
    cout << "Error: expecting system state file as first command line parameter" 
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
  // }

}
