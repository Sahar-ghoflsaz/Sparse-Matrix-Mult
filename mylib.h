#include <stdint.h>

#ifndef __MYLIB_H__
#define __MYLIB_H__

#define SPARSITYA 5
#define SPARSITYB 5

#define COL_SIZE 6
#define ROW_SIZE 5
#define SIZE_VECTOR ROW_SIZE
#define NR_DPUS COL_SIZE 
#define NR_ELEM_PER_DPU 1
#include <stdlib.h>
struct smatrix{

	uint32_t row;
	int32_t val;
};

struct svector{
	uint32_t col;
	int32_t val;
};

void sparse_matrix_mul(struct smatrix *B, struct svector *A, struct svector *C){
	
			if(B->row == A->col){
				C->val = A->val * B->val;
				C->col = A->col;
			}

};

/*void convert_smatrix( uint32_t **A, int cola, int rowa, struct smatrix ***B, uint32_t *nr_cols){
	
	int count=0;

	**B = malloc( sizeof(struct smatrix *) * cola);	
	for(int i=0; i<cola ; i++){
		for(int j=0; j<rowa;j++){
			if(A[j][i] !=0 ){ count++;}
		}
		B[i] = malloc(sizeof(struct smatrix)* count);
		count = 0;
	}		

	count=0;
	for(int i=0; i< cola; i++){
		for(int j=0; j< rowa; j++){
			if(A[j][i] != 0 ){
				B[i][count]->val = A[j][i];
				B[i][count]->row = j;
				count++;		
			}
		}
		nr_cols[i]=count;
	}
};

void convert_svector( uint32_t *A,int size, struct svector **B,uint32_t size_vec){

	uint32_t count=0;
	for(int i=0; i<size;i++){
		if(A[i] !=0 ){ count++;}
	}
	size_vec = count;
	*B = malloc(sizeof(struct svector) * count);			
	for(int i=0; i<size;i++){
		if(A[i] !=0 )
			B[i]->val=A[i];
			B[i]->col=i;
	}		
};*/

#endif /* __MYLIB_H__*/
