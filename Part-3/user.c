//Programmed by: Sapan Basaula
//Dawg tag: 856489979

#include "graph.h"

//The faulty gate injection arrays for different gates
int G_NAND[5]={AND,OR,NOR,XOR,XNOR};  //array of faulty gate to inject if the current node is NAND    
int G_AND[5]={NAND,OR,NOR,XOR,XNOR};  //array of faulty gate to inject if the current node is AND
int G_OR[5]={AND,NAND,NOR,XOR,XNOR};  //array of faulty gate to inject if the current node is OR
int G_NOR[5]={AND,OR,NAND,XOR,XNOR};  //array of faulty gate to inject if the current node is NOR
int G_XNOR[5]={AND,OR,NOR,XOR,NAND};  //array of faulty gate to inject if the current node is XNOR
int G_XOR[5]={AND,OR,NOR,NAND,XNOR};  //array of faulty gate to inject if the current node is XOR
int G_NOT[1]={BUFF};  //array of faulty gate to inject if the current node is NOT
int G_BUFF[1]={NOT};  //array of faulty gate to inject if the current node is BUFF

//Logic Simulation truth table values for 2-input gates
int and[2][2]={{0,0},{0,1}};  //truth table for and gate 
int nand[2][2]={{1,1},{1,0}};  //truth table for nand gate
int or[2][2]={{0,1},{1,1}};  //truth table for or gate  
int nor[2][2]={{1,0},{0,0}};  //truth table for nor gate
int xor[2][2]={{0,1},{1,0}};  //truth table for xor gate
int xnor[2][2]={{1,0},{0,1}};  //truth table for xnor gate
int not[2]={1,0};  //truth table for not gate
int buff[2]={0,1};  //truth table for buff gate

int GATETYPE=-1;
int NPO;

//Routine to check if the node is of type NOT, BUFF or other gates
int gateNode(int Type){
	int i;
	switch(Type){
		case NAND:
		case AND:
		case OR:
		case NOR:
		case XOR:
		case XNOR:
			i=2;
			break;
		case NOT:
		case BUFF:
			i=1;
			break;
		default:
			i=0;
			break;
	}
	return i;
}

//Routine to save the patterns of the test set and the faults it can find in the circuit
void savePattern(char *pattern, FAULT **fault){
	FAULT *tail,*temp;
	int i;
	temp =(FAULT *) malloc(sizeof(FAULT));
	strcpy(temp->pattern,pattern);
	i=0;
	do{
		temp->outputLine[i]=NULL;
		i++;
	}while(i<NPO);
	temp->next=NULL;
	if(*fault == NULL){ *fault=temp; return;}
	tail = *fault;
	while(tail!=NULL){
		if(strcmp(tail->pattern,pattern)==0) {break;}
		if(tail->next==NULL){ tail->next=temp;}
		tail=tail->next;
	}
}	

//Routine to save the pattern of the test set and eliminate the repititive pattern
void saveTestSet(char *pattern, PATTERN **vector){
	PATTERN *tail,*temp;
	temp=(PATTERN *)malloc(sizeof(PATTERN));
	strcpy(temp->pattern,pattern);
	temp->next=NULL;
	if(*vector ==NULL){ *vector=temp; return;}
	tail=*vector;
	while(tail!=NULL){
		if(strcmp(tail->pattern,pattern) ==0) { break;}
		if(tail->next==NULL) { tail->next=temp;}
		tail=tail->next;
	}
}

//Routine to insert all the faults found in the pattern in which the fault is found
void insertFaultList(FAULT **faultlist,int fault, char *pattern,int i){
	FAULT *tail;
	tail=(*faultlist);
	while(tail!=NULL){
		if(strcmp(tail->pattern,pattern)==0){
			InsertList(&(tail->outputLine[i]),fault);
		}
		tail=tail->next;
	}
}

//Routine to simulate the fault for given pattern and print the detected faults in the faultlist file
void faultSimulation(NODE *graph,int Max,char *testFile,char *diagnosis){
	int i,current_node;
	int temp,id[Mnod],num=0;
	int N=0,G=0,j,iteration,k;
	char *faultfree_pattern,*faulty_pattern,*pattern;
	FILE *ffault,*fpattern,*fdiag;
	FAULT *circuitFaults=NULL,*print,*faultFound=NULL,*faultNotFound=NULL,*temp3;
	PATTERN *vector=NULL,*singlePattern;
	LIST *printList,*faultList=NULL,*temp2;
	int randomFault;
	int resolution,sum;
	int min=0,max=0,set=0;
	double average=0.0;
	srand(42);

	pattern=malloc(Mpi*sizeof(char));
	faultfree_pattern=malloc(Mpo*sizeof(char));
	faulty_pattern=malloc(Mpo*sizeof(char));
	
	temp=0;
	fdiag=fopen(diagnosis,"w");
	fpattern=fopen(testFile,"r");
	for(i=0;i<=Max;i++){
		if(gateNode(graph[i].Type)==2)
			N++;
		if (gateNode(graph[i].Type)==1)
			G++;
		if (gateNode(graph[i].Type)==1||gateNode(graph[i].Type)==2){
			id[temp]=i;
			temp++;
		}
	}
	printf("\n<<<<<<<<<<<<<<< Effect-Cause Diagnosis in Progress>>>>>>>>>>>>>>>\n");
	k=0;
	do{
		randomFault=rand() %(5*N+G);
		InsertList(&faultList,randomFault);
		k++;
	}while(k<30);
	while(readTestPatterns(fpattern,&vector)==1){  //get the input pattern from Testpatterns file
		singlePattern=vector;
		while(singlePattern!=NULL){
			strcpy(pattern,singlePattern->pattern);
			faultPatternSimulation(graph,pattern,Max,faultfree_pattern);  //simulate the pattern on the fault free circuit
			NPO=strlen(faultfree_pattern);
			savePattern(pattern,&circuitFaults);
			iteration=num=0;
			GATETYPE=-1;
		  	for(j=0;j<(5*N+G);j++){
				if(iteration==5){
					iteration=0;
					GATETYPE=-1;
					num++;
				}
				if(iteration<5){
					current_node= id[num];
					if(gateNode(graph[current_node].Type)==1){
						iteration=5;
						GATETYPE=0;
					}
					if(gateNode(graph[current_node].Type)==2){
						GATETYPE++;
						iteration++;
					}
				}
				faultyCircuitSimulation(graph,Max,pattern,current_node,faulty_pattern);  //simulate the pattern on the faulty circuit
				i=0;		
				while(i<NPO){  //compare the fault free and faulty output of the circuit
					if(faultfree_pattern[i]!=faulty_pattern[i]){	
						insertFaultList(&circuitFaults,j,pattern,i);
					}
					i++;
				}
	
		 	}
			singlePattern=singlePattern->next;
		}
		k=0;
		sum=0;
		printList=faultList;
		do{
			randomFault= printList->id;
			printf("\nSearching for Fault Presence for fault %d\n",randomFault);
			searchForFaultPresence(randomFault,&circuitFaults,&faultFound,&faultNotFound);
			printf("\nCalculating Diagnostic Resolution\n");
			diagnosticResolution(&faultFound,&resolution);
			printf("Resolution : %d\n",resolution);
			if(k==0) {min=max=resolution;}
			sum+=resolution;
			if(resolution<min) {min = resolution;}
			if(resolution>max) {max = resolution;}
			clearFault(&faultFound);
			clearFault(&faultNotFound);
			printList=printList->next;
			k++;
		}while(printList!=NULL);
		average=(double)sum/(double)k;
		fprintf(fdiag,"Test Set: %d Min: %d Max: %d Average: %f\n",set,min,max,average);
		set++;
		clearPattern(&vector);
		clearFault(&circuitFaults);
	}
	fclose(fpattern);
	fclose(fdiag);
	FreeList(&faultList);
	free(pattern);
	free(faultfree_pattern);
	free(faulty_pattern);
	printf("\n<<<<<<<<<<<<<<< Effect-Cause Diagnosis Complete>>>>>>>>>>>>>>>\n");
}

//Routine to clear the patterns saved from the test set
void clearPattern(PATTERN **vector){
	PATTERN *temp;
	temp= *vector;
	if(temp==NULL) { return;}
	while((*vector)!=NULL){
		temp=temp->next;
		free(*vector);
		(*vector)=temp;
	}
	(*vector)=NULL;
}

//Routine to clear the circuit faults found
void clearFault(FAULT **faultList){
	FAULT *temp;
	int i;
	temp= (*faultList);
	if(temp==NULL) {return;}
	while((*faultList)!=NULL){
		i=0;
		do{
			FreeList(&(temp->outputLine[i]));
			i++;
		}while(i<NPO);
		temp=temp->next;
		free(*faultList);
		(*faultList)=temp;
	}
	(*faultList)=NULL;
}

//Routine to search for the random fault in the circuit faults list and 
//separately create fault found list and fault not found list.
//Then, update the fault found list by removing all the faults in the fault not found list.
void searchForFaultPresence(int fault, FAULT **circuitFaults, FAULT **faultFound, FAULT **faultNotFound){
	FAULT *temp;
	LIST **faultlist;
	int i;
	temp= *circuitFaults;
	while(temp!=NULL){
		i=0;
		do{
			faultlist=&(temp->outputLine[i]);
			if(checkFault(faultlist,fault)==1)
				copyToNewList(faultFound,faultlist,temp->pattern,i);
			else
				copyToNewList(faultNotFound,faultlist,temp->pattern,i);
			i++;
		}while(i<NPO);
		temp=temp->next;
	}
	updateNewList(faultFound,faultNotFound);
}

//Routine to calculate the diagnostic resolution of the test set
void diagnosticResolution(FAULT **faultFound, int *resolution){
	FAULT *temp;
	LIST *temp1,*temp2;
	int i=0,count=0,check;
	temp=(*faultFound);
	if(temp==NULL) { (*resolution) = count; return;}
	do{
		temp1=temp->outputLine[i];
		if(temp1!=NULL) break;
		i++;
	}while(i<NPO);
	while(temp1!=NULL){
		check=checkForAllPatterns(faultFound,temp1->id);
		if(check==1){ count++;}
		temp1=temp1->next;
	}
	(*resolution)=count;
}

//Routine to check for the given fault throughout the circuit faults list 
int checkForAllPatterns(FAULT **faultFound, int fault){
	FAULT *temp;
	int i,check;
	temp=(*faultFound);
	while(temp!=NULL){
		i=0;
		do{
			if(temp->outputLine[i]!=NULL){
				check=checkFault(&(temp->outputLine[i]),fault);
				if(check==0) { return 0;}
			}
			i++;
		}while(i<NPO);
		temp=temp->next;
	}
	return 1;
}

//Routine to check fault presence in certain primary output 
int checkFault(LIST **faultlist, int fault){
	LIST *temp;
	temp=(*faultlist);
	while(temp!=NULL){
		if(temp->id == fault) {return 1;}
		temp=temp->next;
	}
	return 0;
}

//Routine to make a new  list for the fault found and fault not found
void copyToNewList(FAULT **faultFound, LIST **faultlist, char *pattern, int i){
	FAULT *temp,*tail;
	LIST *temp1=NULL,*temp2=NULL;
	int j;
	temp=(FAULT *)malloc(sizeof(FAULT));
	strcpy(temp->pattern,pattern);
	j=0;
	do{
		temp->outputLine[j] = NULL;
		j++;
	}while(j<NPO);
	temp->next =NULL;
	temp1=(*faultlist);
	while(temp1!=NULL){
		InsertList(&temp2,temp1->id);
		temp1=temp1->next;
	}	
	if(*faultFound==NULL) { 
		temp->outputLine[i] = temp2;
		*faultFound=temp; 
		return; 
	}
	tail=(*faultFound);
	while(tail!=NULL){
		if(strcmp(tail->pattern,pattern)==0){
			tail->outputLine[i] = temp2;
			break;
		}
		if(tail->next==NULL) {tail->next = temp;}
		tail=tail->next;
	}	
}

//Routine to update the fault found list by removing all the elements that match with the fault not found list
void updateNewList(FAULT **faultFound,FAULT **faultNotFound){
	FAULT *temp,*temp1;
	LIST *temp2;
	int i,j;
	if(*faultFound==NULL) return;
	temp1=*faultNotFound;
	while(temp1!=NULL){
		i=0;
		do{
			temp2=temp1->outputLine[i];
			while(temp2!=NULL){
				temp=*faultFound;
				while(temp!=NULL){
					for(j=0;j<NPO;j++){
						deleteFault(&(temp->outputLine[j]),temp2->id);
					}
					temp=temp->next;
				}
				temp2=temp2->next;
			}
			i++;
		}while(i<NPO);
		temp1=temp1->next;
	}		
}

// Routine to delete the specific fault from the fault list provided
void deleteFault(LIST **faultlist, int fault){
	LIST *temp,*previous,*new;
	temp=(*faultlist);
	if(temp!=NULL && temp->id==fault){
		*faultlist = temp->next;
		free(temp);
		return;
	}
	while(temp!=NULL){
		previous=temp;
		temp=temp->next;
		if(temp !=NULL && temp->id==fault){
	 		previous->next = temp->next;
			free(temp);
			return;
		}
	}
}

//Routine to read the testpatterns from the file one test set at a time
int readTestPatterns(FILE *fpattern,PATTERN **vector){
	char line[Mlin];
	char pattern[Mpi];
	bzero(line,strlen(line));
	do{
		if((fgets(line,Mlin,fpattern))==NULL) return 0;
	}while(line[0]=='*');

	while(line[0]!='*'){
		bzero(pattern,Mpi);
		sscanf(line,"%s",pattern);
		saveTestSet(pattern,vector);
		bzero(line,strlen(line));
		if((fgets(line,Mlin,fpattern))==NULL) break;
	}
	return 1;
}	

//Routine to simulate the pattern in the faulty circuit
void faultyCircuitSimulation(NODE *graph,int Max,char *vector,int current_node,char *faulty_pattern){
	int i,temp;
	for(i=0;i<=Max;i++){
		if(i==current_node){  //if the node is the current node inject the faulty gate 
			temp=graph[i].Type;
			graph[i].Type=faultyGateInjection(graph[i].Type);
		}
	}
	faultPatternSimulation(graph,vector,Max,faulty_pattern);
	graph[current_node].Type=temp;	
}

//Routine to inject the faults in the circuit
int faultyGateInjection(int Type){
	int newtype;
	switch(Type){
		case NAND : newtype= G_NAND[GATETYPE];
			    break;
		case AND : newtype= G_AND[GATETYPE];
			    break;
		case OR : newtype= G_OR[GATETYPE];
			    break;
		case NOR : newtype= G_NOR[GATETYPE];
			    break;
		case XOR : newtype= G_XOR[GATETYPE];
			    break;
		case XNOR : newtype= G_XNOR[GATETYPE];
			    break;
		case NOT : newtype= G_NOT[GATETYPE];
			    break;
		case BUFF : newtype= G_BUFF[GATETYPE];
			    break;
	}
	return newtype;
}

//Routine to calculate the value at the ouput of each node for different gate types
void performLogicSimulation(NODE *graph,int i,int logicGate[2][2]){
	int entry=1,old,x,y;
	LIST *temp;
	temp=graph[i].Fin;
	while(temp->next!=NULL){
		x=graph[temp->id].Cval;
		y=graph[temp->next->id].Cval;
		if (entry==1)
			graph[i].Cval=logicGate[x][y];
		else
			graph[i].Cval=logicGate[old][y];
		entry++;
		old=graph[i].Cval;
		temp=temp->next;
	}
}

//Routine to traverse the circuit with the given input patterns
void faultPatternSimulation(NODE *graph,char *vector,int Max,char *pattern){
	int i,count,x,y,old,cnt;
	LIST *temp;
	count=x=y=old=cnt=0;
	bzero(pattern,Mpo);
	for(i=0;i<=Max;i++){
		switch(graph[i].Type){
		case INPT:
			graph[i].Cval=vector[count]-'0';
			count++;
			break;
		case AND:
			performLogicSimulation(graph,i,and);
			break;
		case NAND:
			performLogicSimulation(graph,i,nand);
			break;
		case OR:
			performLogicSimulation(graph,i,or);
			break;
		case NOR:
			performLogicSimulation(graph,i,nor);
			break;
		case XOR:
			performLogicSimulation(graph,i,xor);
			break;
		case XNOR:
			performLogicSimulation(graph,i,xnor);
			break;
		case FROM:
			temp=graph[i].Fin;
			while(temp!=NULL){
				x=graph[temp->id].Cval;
				graph[i].Cval=x;
				temp=temp->next;
			}
			break;
		case NOT:
			temp=graph[i].Fin;
			x=graph[temp->id].Cval;
			graph[i].Cval=not[x];
			break;
		case BUFF:
			temp=graph[i].Fin;
			x=graph[temp->id].Cval;
			graph[i].Cval=buff[x];
			break;
		default: break;
		}
		if(graph[i].Po==1){
			pattern[cnt]=graph[i].Cval+'0';
			cnt++;
		}
	}
	pattern[cnt]='\0';
}

