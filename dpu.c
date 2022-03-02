
#include "perfcounter.h"
#include "stdio.h"
#include "mram.h"
#include "defs.h"
#include "barrier.h"
#include "mylib.h"


#define NR_ELEM COL_SIZE/NR_TASKLETS
__host perfcounter_t end_time;
__mram struct svector input_vec[SIZE_VECTOR];
__mram struct smatrix input_mat[COL_SIZE][ROW_SIZE];
__mram uint64_t nr_cols[COL_SIZE];
__mram uint32_t vec_size[2];
__host struct smatrix output_vec[COL_SIZE],output_vecs[NR_TASKLETS][COL_SIZE],temp[NR_TASKLETS][COL_SIZE];



BARRIER_INIT(my_barrier, NR_TASKLETS);

/*#define NR_ELEMENTS_PER_TASKLET (a[0].val/NR_TASKLETS)
#define CACHE_SIZE (SPARSITYA/NR_TASKLETS)
__dma_aligned struct smatrix cache[NR_TASKLETS][CACHE_SIZE];
#define CACHE (SPARSITYB/4)
__dma_aligned struct smatrix cacheB[CACHE];*/


int main(){

	int counter2[NR_TASKLETS]={0};
	if( me() == 0){	
		printf("Start\n");
		perfcounter_config(COUNT_CYCLES, true);
	}

	//synchronize 
	barrier_wait(&my_barrier);
	
	for(int i=me()*NR_ELEM/NR_DPUS; i<(me()+1)*NR_ELEM/NR_DPUS; i++){
		//mram_read(&a[i], &cache[me()][0],sizeof(struct smatrix) * CACHE_SIZE);
		//for(int l = 0; l<CACHE_SIZE ; l++){
		int counter=0;
		for(int j=0; j< nr_cols[i]; j++){
			//if(input_mat[i][j].val !=0){
			//mram_read(&b[j],&cacheB[0],sizeof(struct smatrix)* CACHE);
			//for(int s=0; s<CACHE; s++){
			//printf("%lu\n",nr_cols[i]);
			for(int k=0; k < vec_size[0]; k++){ 
				if(input_mat[i][j].row == input_vec[k].col){
					temp[me()][counter].val = input_vec[k].val * input_mat[i][j].val; 
					temp[me()][counter].row = input_mat[i][j].row;				
					//sparse_matrix_mul(input_mat[i][j], input_vec[k], temp[counter]);
					counter++;
				}
			}
			
		}	
		int value = 0;
		for(int j=0;j< counter;j++){
			value += temp[me()][j].val;
			//printf("<%d,%d>", temp[me()][j].row, temp[me()][j].val);
		}
		if( value!=0){
			output_vecs[me()][counter2[me()]].val = value;
			output_vecs[me()][counter2[me()]].row = temp[me()][counter].row;
			counter2[me()]++;
		}
	}
	
	barrier_wait(&my_barrier);
	
	int c = 0;
	if(me()== 0 ){
		for(int i=0; i<NR_TASKLETS; i++){
			for(int j=0 ; j<NR_ELEM;j++){
				//if(output_vecs[i][j].val != 0){
				output_vec[c]=	output_vecs[i][j];
				//printf("<%d,%d>", output_vec[c].row, output_vec[c].val);
				c++;
			}
		}
	}	
					
	
	end_time = perfcounter_get();
	printf("time: %lu\n", end_time);
	printf("DPU End\n");
	return 0;
}

