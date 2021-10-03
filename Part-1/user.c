//Programmed by: SAPAN BASAULA
//		 #856489979

#include "graph.h"

//The faulty gate injection arrays for different gates
int G_NAND[5]={AND,OR,NOR,XOR,XNOR};  //array of faulty gate to inject if the current node is NAND
int G_AND[5]={NAND,OR,NOR,XOR,XNOR};  //array of faulty gate to inject if the current node is AND
int G_OR[5]={AND,NAND,NOR,XOR,XNOR};  //array of faulty gate to inject if the current node is OR
int G_NOR[5]={AND,OR,NAND,XOR,XNOR};  //array of faulty gate to inject if the current node is NOR
int G_XNOR[5]={AND,OR,NOR,XOR,NAND};  //array of faulty gate to inject if the current node is XNOR
int G_XOR[5]={AND,OR,NOR,NAND,XNOR};  //array of faulty gate to inject if the current node is XOR
int G_NOT[1]={BUFF};			//array of faulty gate to inject if the current node is NOT
int G_BUFF[1]={NOT};			//array of faulty gate to inject if the current node is BUFF

//gate_type is the count for faulty gate to replace in the array of faulty gate
//k is the count of the number of faults injected in the circuit
//slength is the test pattern length 
//cnt is the total count of the number of fault injected circuit to skip
//skip_lines is the array of fault injected circuit to skip due to no test pattern detection
int gate_type=-1,k=0,cnt=0,slength=0,skip_lines[Mnod];	
int not_detected[Mnod],not_detected_gatetype[Mnod];
char ***test_array; // 3-dimensional array to store the test patterns from the .test files.

//***Routine for the call to the atalanta tool for test pattern generation************
void ATPG(char *benchfile, char *faultfile){
	char cmd[Mlin]="/root/Atalanta-master/atalanta -D 100 -f ";   //returns only 100 test patterns for the given fault
	//char cmd[Mlin]="/opt/net/apps/atalanta/atalanta -D 100 -f ";
	strcat(cmd,faultfile);
	strcat(cmd," ");
	strcat(cmd,benchfile);
	system(cmd);
}

//******Routine to write a line in the .bench file *************************
void write_line_to_file(NODE *graph,FILE *fbench,char *gate,int i){
	int fanin=0;
	LIST *temp1,*temp2;
	fprintf(fbench,"%d = %s(",i,gate);
	temp1=graph[i].Fin;
	while(temp1!=NULL){
		if(graph[temp1->id].Type==FROM){ // if the gate type is 'from' - the fanin of the gate node is the fanin of the from node
			temp2=graph[temp1->id].Fin;
			fanin=temp2->id;
		}else
			fanin=temp1->id;
		fprintf(fbench,"%d, ",fanin);
		temp1=temp1->next;
	}
	fseek(fbench,-2,SEEK_CUR);
	fprintf(fbench,")\n");
}

//*******Routine to write the circuit information in the bench file*********************
void write_to_file(NODE *graph,FILE *fbench,int Max){
	int i;
	for(i=0;i<=Max;i++){
		if (graph[i].Type==AND)
			write_line_to_file(graph,fbench,"AND",i);
		if (graph[i].Type==NAND)
			write_line_to_file(graph,fbench,"NAND",i);
		if (graph[i].Type==OR)
			write_line_to_file(graph,fbench,"OR",i);
		if (graph[i].Type==NOR)
			write_line_to_file(graph,fbench,"NOR",i);
		if (graph[i].Type==XOR)
			write_line_to_file(graph,fbench,"XOR",i);
		if (graph[i].Type==XNOR)
			write_line_to_file(graph,fbench,"XNOR",i);
		if (graph[i].Type==NOT)
			write_line_to_file(graph,fbench,"NOT",i);
		if (graph[i].Type==BUFF)
			write_line_to_file(graph,fbench,"BUFF",i);
	}
}

//*****************Routine to determine the type of the gate **********************
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

//**********Routine to create the fault injected bench files, invoke test pattern generation tool *****************
void create_bench_file(FILE *fbench,NODE *graph,int Max,char *testFile){
	int i,nodenum,count=0,*primary_inputs,*nodelist,fanin=0,fanout=0,new_nodes[Mnew],current_node;//nodelist stores the new node numbers for the different gates
	int temp=0,id[Mnod],num=0,*ln;  //ln stores the number of test patterns in .test file in the kth fault injected circuit 
	int N=0,G=0,j,iteration=0,org_node=0,original_node[Mnod],newnod_count;
	FILE *ffault,*ftest,*fpattern;

	clock_t start,end;
	double timeElapsed;

	test_array=(char ***)malloc(Mnod*sizeof(char ***));
	for(int h=0;h<Mnod;h++){
		test_array[h]=(char **) malloc(Mpat*sizeof(char *));
		for (int l=0;l<Mpat;l++){
			test_array[h][l]=(char *) malloc(Mpi*sizeof(char));
		}
	}
	primary_inputs=(int *)malloc(Mpi*sizeof(int));
	nodelist=(int *) malloc(Mnod*sizeof(int));
	ln=(int *)malloc(Mnod*sizeof(int));
	bzero(primary_inputs,Mpi);
	bzero(nodelist,Mnod);
	bzero(ln,Mnod);
	//**Print the iscas89 netlist format**
	for(i=0;i<=Max;i++){   //count the number of NOT,BUFF gates (G) and NAND,AND,OR,XOR,NOR,XNOR gates (N) in the circuit
		if(Gate_node(graph[i].Type)==2)
			N++;
		if (Gate_node(graph[i].Type)==1)
			G++;
	}
	start=clock();
	for(j=0;j<(5*N+G);j++){
		fbench=fopen("circuit_conv.bench","w");
		fprintf(fbench,"#Modified_benchmark circuit\n");
		nodenum=Max; //store the number of nodes in the previous graph
		temp=count=0;
		if(iteration==5){
			iteration=0;
			gate_type=-1;
			num++;
		}
		for(i=0;i<=Max;i++){
			if (graph[i].Type==INPT){ 
				primary_inputs[count]=i;
				fprintf(fbench,"INPUT(%d)\n",primary_inputs[count]);	
				count++;
			} else {
				nodelist[i]=nodenum++; //store the new nodelist number by increasing the Max value of the previous graph
			}
			if (Gate_node(graph[i].Type)==1||Gate_node(graph[i].Type)==2){
				id[temp]= nodelist[i];
				original_node[temp]=i;
				temp++;
			}
			
		}
		write_to_file(graph,fbench,Max);	//write only the gates of the original graph structure information in the bench file
		if(iteration<5){		//Hold the current value of node for 5 iterations for other gates and once for NOT and BUFF gates
			current_node= id[num];
			org_node= original_node[num];
			if(Gate_node(graph[org_node].Type)==1){
				iteration=5;
				gate_type=0;
			}
			if(Gate_node(graph[org_node].Type)==2){
				gate_type++;
				iteration++;
			}
		}
		newnod_count=duplicate_graph(fbench,graph,Max,nodelist,primary_inputs,count,nodenum,current_node); //duplicate, print the duplicated graph and return the new node count
		count=0;
		for(i=0;i<=Max;i++){
			if(graph[i].Po==1){
				if(graph[i].Type!=INPT){
					new_nodes[count]=newnod_count++;
					fprintf(fbench,"%d = XOR(%d, %d)\n",new_nodes[count],i,nodelist[i]); // add new XOR gates for the primary o/ps
				} else
					new_nodes[count]=i;
				count++;
			}
		}
		fprintf(fbench,"%d = OR(",newnod_count++); //OR the o/p of all the XOR gates
		for(i=0;i<count;i++)
			fprintf(fbench,"%d, ",new_nodes[i]);
		fseek(fbench,-2,SEEK_CUR);
		fprintf(fbench,")\n");	
		fprintf(fbench,"OUTPUT(%d)\n",--newnod_count);
		fclose(fbench);
		ffault=fopen("circuit_conv.flt","w");
		fprintf(ffault,"%d /0\n",newnod_count);	//Insert fault Stuck-at-0 at the last OR output gate
		fclose(ffault);
		ATPG("circuit_conv.bench","circuit_conv.flt");  //Get the test patterns from the atalanta tool
		ln[k]=read_patterns(fpattern,"circuit_conv.test",current_node,gate_type);
		k++;
	}
	write_to_testfile(ftest,testFile,ln); // write the test patterns in the .test file
	end=clock();
	timeElapsed=((double) (end-start))/ CLOCKS_PER_SEC;
	printf("The Total Time Elapsed is: %f\n",timeElapsed);
	for(i=1;i<cnt;i++){
			printf("ATPG: No test patterns detected for node %d / gate type %d\n",not_detected[i],not_detected_gatetype[i]);
	}
	printf("The number of undetected fault:%d",cnt);
	free(primary_inputs);
	free(nodelist);
	free(ln);
}

//********Routine to write the test sets *********************
void write_to_testfile(FILE *ftest,char *filename,int *ln){
	int random_n,i,j,n,g,m;
	char temp[slength];
	srand(time(0));
	ftest=fopen(filename,"w");
	for(g=0;g<1;g++){
		fprintf(ftest,"**************GROUP %d**************\n",g);
		for(n=0;n<2;n++){
			fprintf(ftest,"*******TEST SET %d***********\n",n);
			for(i=0;i<k;i++){
				for(m=0;m<=g;m++){
					if(check_empty(i)==1) break;
					random_n=rand() % ln[i];   // Random pattern line number  for the ith fault injected circuit 
					for(j=0;j<slength;j++){
						fprintf(ftest,"%c",test_array[i][random_n][j]);
					}
					fprintf(ftest,"\n");
				}
			}
			if(n==1) break;
		}
		if(g==0) break;
	}
	fclose(ftest);
	for(int h=0;h<Mnod;h++){  //free the allocated memory for the test patterns
		for (int l=0;l<Mpat;l++)
			free(test_array[h][l]);
		free(test_array[h]);
	}
	free(test_array);
}

//*************Routine to check if the test pattern generation tool returns any pattern or not************
int check_empty(int num){
	int i;
	for(i=0;i<cnt;i++){
		if(skip_lines[i]==num) return 1;
	}
	return 0;
}

//*****************Routine to read the test patterns generated by ATPG ******************************************
int read_patterns(FILE *fpattern,char *filename,int current_node,int gate_type){
	char line[Mpi],str1[Mlin],str2[Mpi],ch;
	int out,i,j=0;
	fpattern=fopen(filename,"r");
	srand(time(0));
	for(;;){
		fgets(line,Mpi,fpattern);
		sscanf(line,"%c %s",&ch,str1);
		if(strcmp(str1,"Test")==0) break;
	}
	bzero(line,strlen(line));
	do{
		fgets(line,Mpi,fpattern);
	}while(line[0]=='\n');
	bzero(line,strlen(line));
	if(fgets(line,Mpi,fpattern)==NULL){
		skip_lines[cnt]=k;         //store the kth fault injected circuit in skip_lines array if no test patterns are found
		cnt++;
		not_detected[cnt]=current_node;
		not_detected_gatetype[cnt]=gate_type;
	}
	while(!feof(fpattern)){
		bzero(str1,strlen(str1));
		bzero(str2,strlen(str2));
		sscanf(line,"%s %s %d",str1,str2,&out);
		slength=strlen(str2);		//store the number of characters in a test pattern
		for(i=0;i<slength;i++){
			if(str2[i]=='x') {str2[i]= rand() % 2 + '0';} //random assignment of 1 or 0 for the dont care value
			test_array[k][j][i]=str2[i];  // store the test patterns in the jth row for the kth fault injected circuit
		}
		bzero(line,strlen(line));
		fgets(line,Mpi,fpattern);
		j++;	
	}
	fclose(fpattern);
	return j;
}	

//****************Routine to count the number of fanins for the 2-input XOR gate conversion***********************
int check_nfanin(int i, NODE *graph){
	int count=0;
	LIST *temp;
	temp=graph[i].Fin;
	while(temp!=NULL){
		count++;
		temp=temp->next;
	}
	return count;
}

//****************Routine to update the circuit if more than 2 input XOR gate is present in the circuit*********************
void update_circuit(NODE *graph, NODE *temp,int *nodelist,int *primary_inputs,int count,int i,int Max,int gtype){
	int nf,nfin[Mpi],num=0,j,nfin_c=0,a,offset,temp_nodelist;
	LIST *temp1,*temp2;
	temp2=graph[i].Fin;
	while(temp2!=NULL){
		nfin[num]=temp2->id;
		temp2=temp2->next;
		num++;
	}
	temp_nodelist=nodelist[i];
	for(j=0;j<num-1;j++){
		if(j==0){ //for the first iteration keep the 2 inputs 
			nf=0;
			temp[temp_nodelist].Fin=NULL;
			while(nf<2){	
				if(check_primaryip(nfin[nfin_c],primary_inputs,count)==1)
					InsertList(&temp[temp_nodelist].Fin,nfin[nfin_c]);
				else
					InsertList(&temp[temp_nodelist].Fin,nodelist[(nfin[nfin_c])]);
				nf++;
				nfin_c++;
			}
		}else{  //for other cases the fanout of the previous XOR is fanin of the next XOR
			a=temp_nodelist;
			temp_nodelist++;
			temp[temp_nodelist].Fin=NULL;
			strcpy(temp[temp_nodelist].Name,"Extra");
			temp[temp_nodelist].Type=gtype;
			temp[temp_nodelist].Nfi=2;
			temp[temp_nodelist].Nfo=1;
			InsertList(&temp[temp_nodelist].Fin,a);
			if(check_primaryip(nfin[nfin_c],primary_inputs,count)==1)
				InsertList(&temp[temp_nodelist].Fin,nfin[nfin_c]);
			else
				InsertList(&temp[temp_nodelist].Fin,nodelist[(nfin[nfin_c])]);
			nfin_c++;
		}
		temp[temp_nodelist].Fot=NULL;
		if(j!=num-2)
			InsertList(&temp[temp_nodelist].Fot,(temp_nodelist+1));
			
	}
	offset=num-2;
	temp1=graph[i].Fot;
	while(temp1!=NULL){ //for the last XOR gate the fanout is the fanout of the original multi input XOR Gate
		InsertList(&temp[temp_nodelist].Fot,nodelist[((temp1->id)+offset)]);
		temp1=temp1->next;
	}
		
}
//**********************Routine to duplicate the circuit information by updating the node number and injecting a faulty gate*************
int duplicate_graph(FILE *fbench,NODE *graph,int Max,int *nodelist,int *primary_inputs,int count,int nodenum,int current_node){
	int nfanin=0,entered;
	NODE temp[2*Mnod];
	LIST *temp1;
	int i,j,offset,fot_value,gtype;
	for(i=0;i<=Max;i++){
		entered=0;
		if(graph[i].Type==INPT)
			temp[i]=graph[i];
		else{
			strcpy(temp[nodelist[i]].Name,graph[i].Name);
			temp[nodelist[i]].Type=graph[i].Type;
			temp[nodelist[i]].Nfo=graph[i].Nfo;
			temp[nodelist[i]].Nfi=graph[i].Nfi;
			temp[nodelist[i]].Po=graph[i].Po;
			if (nodelist[i]==current_node){    //If the node is the current node for which the faulty gates are to be injected
				temp[nodelist[i]].Type=Faulty_gate_injection(temp[nodelist[i]].Type); //Inject a faulty circuit element
				gtype=temp[nodelist[i]].Type;
				if (gtype == XOR || gtype== XNOR){ //if the injected gate is XNOR or XOR
					nfanin=check_nfanin(i,graph); //check the number of fanins
					if(nfanin>2){  //if the number of fanin is greater than 2
						entered=1;
						update_circuit(graph,temp,nodelist,primary_inputs,count,i,Max,gtype); //update the circuit by replacing multi input XOR gates with (number of fanins - 2) number of 2-input XOR gates
						offset=nfanin-2;
						nodenum+=offset; //increase the number of nodes by the number of gates added
						for(j=0;j<=Max;j++){
							if(temp[nodelist[j]].Type==FROM){
								temp1=temp[nodelist[j]].Fot;
								fot_value=temp1->id;
								if(fot_value > current_node)
									temp1->id = fot_value + offset;
							}
							if(j>=i)
								nodelist[j]+= offset;  //increase the nodes after the added gates by the offset
						}
					}
				}
			}
			if(entered==0){
				temp1=graph[i].Fin;
				temp[nodelist[i]].Fin=NULL;
				while (temp1!=NULL){
					if(check_primaryip(temp1->id,primary_inputs,count)==1)
						InsertList(&temp[nodelist[i]].Fin,temp1->id);
					else
						InsertList(&temp[nodelist[i]].Fin,nodelist[temp1->id]);
					temp1=temp1->next;
				}
				temp1=graph[i].Fot;
				temp[nodelist[i]].Fot=NULL;
				while(temp1!=NULL){
					InsertList(&temp[nodelist[i]].Fot,nodelist[temp1->id]);
					temp1=temp1->next;
				}
			}
		}
	}
	write_to_file(temp,fbench,nodenum); //write the duplicated circuit with one injected faulty gate in the .bench file
	ClearCircuit(temp,2*Mnod);
	return nodenum;	
}

//*******Routine to inject a faulty gate in the duplicate circuit************************
int Faulty_gate_injection(int Type){
	int new_type=0;
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

//**************Routine to check if a node in the graph structure is primary input or not********************
int check_primaryip(int id_node, int *primary_inputs, int count){
	int i;
	for(i=0;i<=count;i++){
		if(id_node==primary_inputs[i])
			return 1;
	}
	return 0;
}


