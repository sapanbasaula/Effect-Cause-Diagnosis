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

int gate_type=-1;

//Routine to check if the node is of type NOT, BUFF or other gates
int Gate_node(int Type){
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

//Routine to simulate the fault for given pattern and print the detected faults in the faultlist file
void faultSimulation(NODE *graph,int Max,char *testFile,char *faultFile){
	int i,current_node;
	int temp,id[Mnod],num=0;
	int N=0,G=0,j,iteration;
	int a,b,faults[Mpo][Nfaults],specific_fault[Mpo][Nfaults];  //faults saves the fault detected and specific_fault saves the type of the faulty gate 
	int faultcount[Mpo],faulttype;  //faultcount saves the count of fault detected in each primary input
	char *vector;
	char *faultfree_pattern,*faulty_pattern;
	FILE *ffault,*fpattern;
	vector=malloc(Mpi*sizeof(char));
	faultfree_pattern=malloc(Mpo*sizeof(char));
	faulty_pattern=malloc(Mpo*sizeof(char));
	
	temp=0;
	ffault=fopen(faultFile,"w");
	fpattern=fopen(testFile,"r");
	for(i=0;i<=Max;i++){
		if(Gate_node(graph[i].Type)==2)
			N++;
		if (Gate_node(graph[i].Type)==1)
			G++;
		if (Gate_node(graph[i].Type)==1||Gate_node(graph[i].Type)==2){
			id[temp]=i;
			temp++;
		}
	}
	printf("\n<<<<<<<<<<<<<<<Fault Simulation in Progress>>>>>>>>>>>>>>>\n");
	while(read_testpatterns(fpattern,vector)==1){  //get the input pattern from Testpatterns file
		bzero(faultcount,Mpo);
		faultPatternSimulation(graph,vector,Max,faultfree_pattern);  //simulate the pattern on the fault free circuit
		fprintf(ffault,"%s:\n",vector);
		iteration=num=0;
		gate_type=-1;
		for(j=0;j<(5*N+G);j++){
			if(iteration==5){
				iteration=0;
				gate_type=-1;
				num++;
			}
			if(iteration<5){
				current_node= id[num];
				if(Gate_node(graph[current_node].Type)==1){
					iteration=5;
					gate_type=0;
				}
				if(Gate_node(graph[current_node].Type)==2){
					gate_type++;
					iteration++;
				}
			}
			faulttype=faultyCircuitSimulation(graph,Max,vector,current_node,faulty_pattern);  //simulate the pattern on the faulty circuit
			i=0;		
			while(i<strlen(faulty_pattern)){  //compare the fault free and faulty output of the circuit
				if(faultfree_pattern[i]!=faulty_pattern[i]){	
					faults[i][faultcount[i]]=current_node;
					specific_fault[i][faultcount[i]]=faulttype;
					faultcount[i]++;
				}
				i++;
			}
		}

		for(a=0;a<strlen(faulty_pattern);a++){
			fprintf(ffault,"Out[%d]:",a);
			for(b=0;b<faultcount[a];b++)
				fprintf(ffault,"%d/%d ",faults[a][b],specific_fault[a][b]);  //print the detected fault in the faultlist file
			fprintf(ffault,"\n");
		}
	}
	fclose(fpattern);
	fclose(ffault);
	free(vector);
	free(faultfree_pattern);
	free(faulty_pattern);
	printf("\n<<<<<<<<<<<<<<<Simulation Complete>>>>>>>>>>>>>>>\n");
}

//Routine to read the testpatterns from the file one line at a time
int read_testpatterns(FILE *fpattern,char *pattern){
	char line[Mlin];
	bzero(line,strlen(line));
	do{
		if((fgets(line,Mlin,fpattern))==NULL) return 0;
	}while(line[0]=='*');
	bzero(pattern,Mpi);
	sscanf(line,"%s",pattern);
	return 1;
}	

//Routine to simulate the pattern in the faulty circuit
int faultyCircuitSimulation(NODE *graph,int Max,char *vector,int current_node,char *faulty_pattern){
	int i,faulty_gatetype,temp;
	for(i=0;i<=Max;i++){
		if(i==current_node){  //if the node is the current node inject the faulty gate 
			temp=graph[i].Type;
			graph[i].Type=Faulty_gate_injection(graph[i].Type);
			faulty_gatetype=graph[i].Type;
		}
	}
	faultPatternSimulation(graph,vector,Max,faulty_pattern);
	graph[current_node].Type=temp;	
	return faulty_gatetype;
}

//Routine to inject the faults in the circuit
int Faulty_gate_injection(int Type){
	int new_type;
	switch(Type){
		case NAND : new_type= G_NAND[gate_type];
			    break;
		case AND : new_type= G_AND[gate_type];
			    break;
		case OR : new_type= G_OR[gate_type];
			    break;
		case NOR : new_type= G_NOR[gate_type];
			    break;
		case XOR : new_type= G_XOR[gate_type];
			    break;
		case XNOR : new_type= G_XNOR[gate_type];
			    break;
		case NOT : new_type= G_NOT[gate_type];
			    break;
		case BUFF : new_type= G_BUFF[gate_type];
			    break;
	}
	return new_type;
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
			performLogicSimulation(graph,i,and);
			graph[i].Cval = not[graph[i].Cval];
			break;
		case OR:
			performLogicSimulation(graph,i,or);
			break;
		case NOR:
			performLogicSimulation(graph,i,or);
			graph[i].Cval = not[graph[i].Cval];
			break;
		case XOR:
			performLogicSimulation(graph,i,xor);
			break;
		case XNOR:
			performLogicSimulation(graph,i,xor);
			graph[i].Cval = not[graph[i].Cval];
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

