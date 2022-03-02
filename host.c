#include <dpu.h>
#include <stdlib.h>
#include <stdio.h>
#include "mylib.h"
#include <time.h>


static inline double my_clock(void) {
  	struct timespec t;
  	clock_gettime(CLOCK_MONOTONIC_RAW, &t);
  	return (1.0e-9 * t.tv_nsec + t.tv_sec);
}

/*******************************************
******************DPU***********************
*******************************************/

static void free_dpus(struct dpu_set_t set){
	DPU_ASSERT(dpu_free(set));
}

static void alloc_dpus(struct dpu_set_t *set, uint32_t *nr_dpus){
	DPU_ASSERT(dpu_alloc(NR_DPUS, NULL, set));
  	printf("DPU allocated\n");
	DPU_ASSERT(dpu_load(*set, DPU_BINARY, NULL));
	DPU_ASSERT(dpu_get_nr_dpus(*set, nr_dpus));
	
}

/*******************************************
*****************MATRIX*********************
*******************************************/

static void init_buffers(uint32_t *A, uint32_t **B){
	srand(0);
	for (int i=0; i< SIZE_VECTOR; i++){
		A[i]= rand()% 2;
	}
	printf("yes");
	srand(0);
	for (int i=0; i< ROW_SIZE; i++){
		for(int j=0; j< COL_SIZE; j++){
			B[i][j] = rand() % 3;
		} 	
	}
}

static void free_buffers( struct svector *B, uint32_t **C, struct smatrix **D,struct svector **E, uint32_t *F){

	free(B);
	for(int i=0; i<ROW_SIZE;i++){
		free(C[i]);
	}
	for(int i=0; i<COL_SIZE;i++){
		free(D[i]);
	}
	free(C);
	free(D);
	for(int i=0; i<COL_SIZE;i++){
		free(E[i]);
	}
	free(E);
	free(F);
}


/************************************************
*******************MAIN**************************
************************************************/

int main() {

	/***************** DPU ALLOCATION*************************************************/
	
	struct dpu_set_t set, dpu;
	uint32_t nr_dpus, each_dpu;
	alloc_dpus(&set, &nr_dpus);
	printf("allocated dpu = %u:\n", nr_dpus);
	
	/***************** BUFFER ALLOCATION **********************************************/
	
	struct smatrix **input_smatrix= malloc(sizeof(struct smatrix *) * COL_SIZE);
	struct svector **output_svector=malloc(sizeof(struct smatrix *) * NR_DPUS);
	int32_t input_vector[SIZE_VECTOR]; 
	uint32_t *output_vector,size_vec[2];
	uint32_t **input_matrix=malloc(sizeof(struct smatrix*) * ROW_SIZE);
	uint64_t nr_cols[COL_SIZE];
	
	for(int i=0;i<nr_dpus;i++){
		 output_svector[i] = malloc(sizeof(struct smatrix ) * COL_SIZE/NR_DPUS);
	}
	for(int i=0; i<ROW_SIZE ; i++){
		input_matrix[i] = malloc(sizeof(struct smatrix)*(COL_SIZE));
	}
	
	/*************** INPUTS IN MATRIX AND VECTOR PRESENTATION **************************/
	srand(0);
	for (int i=0; i< SIZE_VECTOR; i++){
		input_vector[i]= (rand()% 3);
		
	}
	printf("\nVECTOR:\n");
	for (int i=0; i< SIZE_VECTOR; i++){
		printf ("%d  ",input_vector[i]);
		
	}

	printf("\nMATRIX:\n");
	srand(0);
	for (int i=0; i< ROW_SIZE; i++){
		for(int j=0; j< COL_SIZE; j++){
			input_matrix[i][j] = rand() % 3;
		} 	
	}
	for (int i=0; i< ROW_SIZE; i++){
		for(int j=0; j< COL_SIZE; j++){
			printf("%d  ",input_matrix[i][j]);
		} 	
		printf("\n");
	}

	/*********************CONVERT MATRIX AND VECTOR TO A PRESENTATION WITHOUT 0 **********************/
	
	
	//convert_smatrix(input_matrix, COL_SIZE, ROW_SIZE, &input_smatrix,&nr_cols);
	uint32_t counts=0;
	int max=0;
	for(int i=0; i<COL_SIZE ; i++){
		for(int j=0; j<ROW_SIZE ;j++){
			if(input_matrix[j][i] !=0 ){ counts++;}
		}
		input_smatrix[i] = malloc(sizeof(struct smatrix)* counts);
		counts = 0;
	}		
	counts=0;
	uint32_t sum=0;
	for(int i=0; i< COL_SIZE; i++){
		counts=0;
		for(int j=0; j< ROW_SIZE; j++){
			if(input_matrix[j][i] != 0 ){
				input_smatrix[i][counts].val = input_matrix[j][i];
				input_smatrix[i][counts].row = j;
				counts++;		
			}
		}
		if( counts> max) max=counts;
		nr_cols[i]=counts;
		sum += counts;
	}

	printf("\nSparse Matrix Representation :\n");
	for(int i=0; i<COL_SIZE ;i++){
		for(int j=0; j<nr_cols[i];j++){	
			printf("< %d, %d >", input_smatrix[i][j].row,input_smatrix[i][j].val );
		}
		printf("\n");	
	}
	printf("\n");
	
	//convert_svector(input_vector,SIZE_VECTOR,&input_svector,size_vec);
	
	uint32_t count=0;
	for(int i=0; i<SIZE_VECTOR;i++){
		if(input_vector[i] !=0 ){ count++;}
	}
	size_vec[0] = count;
	//printf("vector\n count:%d",size_vec);
	
	struct svector *input_svector = malloc(sizeof(struct svector) * count);	
	count=0;	
	for(int i=0; i<size_vec[0]+1;i++){
		if(input_vector[i] !=0 ){
			input_svector[count].val=input_vector[i];
			input_svector[count].col=i;
			count ++;}
	}
	printf("\nSparse Vector Representation :\n");
	for(int i=0; i<count ;i++){	
		printf("< %d, %d >\n", input_svector[i].col ,input_svector[i].val);	
	}
	printf("alloc buffer\n");
		
	/************************COPY_TO_MRAM********/
	

	DPU_FOREACH(set,dpu,each_dpu){//each_dpu , dpu A[EACH]
		DPU_ASSERT(dpu_copy_to(dpu,"input_vec",0, &input_svector[0], sizeof(struct svector) * (size_vec[0])));	
		DPU_ASSERT(dpu_copy_to(dpu,"input_mat",0, &input_smatrix[each_dpu][0], sizeof(struct smatrix) * (max)));
		DPU_ASSERT(dpu_copy_to(dpu,"nr_cols",0, &nr_cols[each_dpu], sizeof(uint64_t) * (COL_SIZE)));
		DPU_ASSERT(dpu_copy_to(dpu,"vec_size",0, &size_vec[0], sizeof(uint32_t)*2));
	}
	
	printf(" copy input\n");
	
	/*****************DPU LAUNCH******************/
	
	double start = my_clock();
	DPU_ASSERT(dpu_launch(set, DPU_SYNCHRONOUS));
	double end = my_clock();
	
	printf(" run dpus\n");
	
	/***************COPY_FROM_MRAM****************/
	
	//copy output from dpus
	DPU_FOREACH(set,dpu,each_dpu){
		DPU_ASSERT(dpu_copy_from(dpu, "output_vec", 0, &output_svector[each_dpu][0], sizeof(struct svector) *(COL_SIZE/NR_DPUS)));
		DPU_ASSERT(dpu_log_read(dpu, stdout));
	}
	printf("copy output\n");

	/*********************************************/
	
	
	for(int j=0;j< nr_dpus;j++){
		for (int i=0;i< COL_SIZE/NR_DPUS; i++){
		if(output_svector[j][i].val != 0)
		printf("< %d , %d>\n",j,output_svector[j][i].val);
	}}
	
	

	// retrieve number of cycles on DPU
  	uint32_t nb_cycles;
  	DPU_FOREACH(set, dpu) {
    		DPU_ASSERT(dpu_copy_from(dpu, "end_time", 0, &nb_cycles, sizeof(uint32_t)));
  	}

 	 // retrieve DPU frequency
  	uint32_t clocks_per_sec;
  	DPU_FOREACH(set, dpu) {
   		 DPU_ASSERT(dpu_copy_from(dpu, "CLOCKS_PER_SEC", 0, &clocks_per_sec, sizeof(uint32_t)));
	  }
	
	printf("DPU cycles: %u\n", nb_cycles);
	printf("Host elapsed time: %.2e secs.\n", end - start);
  	printf("DPU time: %.2e secs.\n", (double)nb_cycles / clocks_per_sec);

	free_buffers(input_svector,input_matrix,input_smatrix,output_svector,output_vector);
	free_dpus(set);
	

	return 0;
}

