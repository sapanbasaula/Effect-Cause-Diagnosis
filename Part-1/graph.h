/***************************************************************************************************************************
Header Files
****************************************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <time.h>
#include <complex.h>
#include <assert.h>
#include <limits.h>
/***************************************************************************************************************************
Constant Declarations 
****************************************************************************************************************************/
// VARIOUS CONSTANTS
#define Mfnam      20			// max size for a file name
#define Mnod    15000 		        // max number of nodes in a graph/node
#define Mlin      200			// max size of characters in a line
#define Mnam       25			// max size of a node name
#define Mtyp       10			// max type of nodes/gates
#define Mout       16		        // max node out degree (Nfo)
#define Min         9			// max node in degree (Nfi)
#define Mpi       250			// max number of primary inputs
#define Mpo       140			// max number of primary outputs
#define Mpt       50			// max number of input patterns in .vec file
#define Mft       10			// max number of stuck at faults in .faults file
#define Mtp	   50
#define Mpat 	  110			//Max number of test patterns
#define Mnew	  300			//Max number of new nodes addition

// NODE TYPE CONSTANTS 
#define INPT 1				// Primary Input
#define AND  2				// AND 
#define NAND 3				// NAND 
#define OR   4				// OR 
#define NOR  5				// NOR 
#define XOR  6				// XOR 
#define XNOR 7				// XNOR 
#define BUFF 8				// BUFFER 
#define NOT  9				// INVERTER 
#define FROM 10				// STEM BRANCH
/***************************************************************************************************************************
Structure Declarations 
****************************************************************************************************************************/
//1.Stucture declaration for LIST
typedef struct LIST_type {
   int  id;		   //id for current element		
   struct LIST_type *next;  //pointer to next id element( if there is no element, then it will be NULL)		
} LIST;
//2.Stucture declaration for NODE
typedef struct NODE_type
{
  char Name[Mnam];                      //name of the node
  int Type,Nfi,Nfo,Po;                  //type, nooffanins, nooffanouts,primaryo/p
  int Mark,Cval,Fval;                    //marker,correctvalue,faultvalue
  LIST *Fin,*Fot;                      //fanin members, fanout members 
} NODE;
//3.Stucture declaration for PATTERN




//4.Stucture declaration for FAULT




/***************************************************************************************************************************
Functions in given.c
****************************************************************************************************************************/
/***************************************************************************************************************************
LIST Structure Functions
****************************************************************************************************************************/
void InsertList(LIST **,int);
void PrintList(LIST *);
void FreeList(LIST **);
/***************************************************************************************************************************
 NODE Structure Functions
****************************************************************************************************************************/
int ReadIsc(FILE *,NODE *);
void InitializeCircuit(NODE *,int);
int AssignType(char *);
void PrintCircuit(NODE *,int);
void ClearCircuit(NODE *,int);

/***************************************************************************************************************************
 PATTERN Structure Functions
****************************************************************************************************************************/

/***************************************************************************************************************************
User Defined Functions in user.c
****************************************************************************************************************************/
void create_bench_file(FILE *,NODE *,int,char *);
void ATPG(char *,char *);
int duplicate_graph(FILE *,NODE *,int,int *,int *,int,int,int);
int check_primaryip(int, int *,int);
void write_to_file(NODE *, FILE *,int);
int Faulty_gate_injection(int);
int Gate_node(int);
int read_patterns(FILE *,char *,int,int);
void write_to_testfile(FILE *,char *,int *);
int check_empty(int);
void write_line_to_file(NODE *,FILE *,char *,int);
int check_nfanin(int,NODE *);
void update_circuit(NODE *,NODE *, int *,int *,int,int,int,int);
/****************************************************************************************************************************/
