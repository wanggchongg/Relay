#include "matrix.h"

static uint32 search_col_1(uint8 **mat, uint32 start, uint32 row);
static void row_exchange(uint32 row, uint32 col, uint8 **mat,uint32 matsize);
static void row_sub(uint32 row1, uint32 row2, uint8 **mat, uint32 matsize, uint8 temp1, uint8 temp2);
static void row_or(uint32 row1, uint32 row2, uint8 **mat, uint32 matsize);
static void matrix_free(uint8 **mat, uint32 size);
static void my_xor(uint8 *x, uint8 *y, uint32 size);
static void my_exchange(uint8 *x, uint8 *y, uint32 size);

int matrix_init(uint32 rownum, uint32 columnum, Matrix_t *mymat){
	uint32 i;
	mymat->colum = columnum;
	mymat->row = rownum;
	mymat->rowpoint = (uint8 **)malloc(rownum*sizeof(uint8*));
	if(mymat->rowpoint == NULL){
		printf("rowpoint malloc error!\n");
		return 0;
	}
	for(i = 0; i<rownum; i++){
		mymat->rowpoint[i] = (uint8 *)malloc(columnum*sizeof(uint8));
		if(mymat->rowpoint[i] == NULL){
			printf("mymat->rowpoint[%d] init error\n",i);
			return 0;
		}
		memset(mymat->rowpoint[i],0,columnum);
	}
	return 1;
}

int matrix_reset(uint32 rownum , uint32 columnum, Matrix_t *mymat){
	uint32 i;
	mymat->colum = columnum;
	mymat->row = rownum;
	for(i = 0; i<rownum; i++){
		memset(mymat->rowpoint[i],0,columnum);
	}
	return 1;
}

//求矩阵的逆
int matrix_inverse(Matrix_t *A, Matrix_t *A_1){
//	printf("%d ",A->row);
	uint32 i,j,search_1;
	uint8 **matpoint, **inv_matpoint;
	matpoint = A->rowpoint;//(uint8**)malloc(A->row*sizeof(uint8*));
//	printf("matrix_inverse\n");
/*	for(i=0; i<A->row; i++){
//		printf("%d ",i);
		matpoint[i] = (uint8*)malloc(A->colum*sizeof(uint8));
		memcpy(matpoint[i],A->rowpoint[i],A->colum);
	}*/
	for(i=0; i<A_1->row; i++){
		A_1->rowpoint[i][i] = 1;
	}
	inv_matpoint = A_1->rowpoint;
	/*/////////////////////
	for(uint32 k = 0;k<A->row;k++){
		for(uint32 m = 0; m<A->colum; m++){
			printf("%2d ",matpoint[k][m]);
		}
		printf("\n");
	}
	printf("\n");
	//////////////////*/
	for(j=0; j<A->row; j++){


		search_1 = search_col_1(matpoint, j, A->row);
		if(search_1 == -1){
			printf("A not manyi\n");
			return 0;
		}
		if(search_1 != j){
			row_exchange(j,search_1, matpoint, A->row);
			row_exchange(j,search_1, inv_matpoint, A_1->row);
		}		/*///////////////////
		for(uint32 k = 0;k<A->row;k++){
		for(uint32 m = 0; m<A->colum; m++){
		printf("%0d ",matpoint[k][m]);
		}
		printf("\n");

		}
		printf("\n");
		///////////////////*/

		for(i=0; i<A->row; i++){
			if(matpoint[i][j] != 0){
				if(i!= j){
					row_or(i, j, inv_matpoint, A->colum);
					row_or(i, j, matpoint, A->colum);

				}

				/*///////////////////////////////
				for(uint32 k = 0;k<A->row;k++){
				for(uint32 m = 0; m<A->colum; m++){
				printf("%2d ",matpoint[k][m]);
				}
				printf("    ");
				for(uint32 m = 0; m<A->colum; m++){
				printf("%2d ",inv_matpoint[k][m]);
				}
				printf("\n");
				}
				printf("\n");
				/////////////////////////////////*/
			}
		}
	}
	//matrix_free(matpoint,A->row);
	return 1;
}

int matrix_equations(Matrix_t *A, uint8 *D, uint32 size){
    uint32 i,j,search_1;
    uint8 **matpoint;
    matpoint = (uint8 **)malloc(A->row * sizeof(uint8 *));
    for(i=0; i<A->row; i++){
        matpoint[i] = (uint8 *)malloc(A->colum*sizeof(uint8));
        memcpy(matpoint[i], A->rowpoint[i], A->colum);
    }
    //matpoint = A->rowpoint;
    for(j=0; j<A->colum; j++){
        /*/////////////
        for(uint32 k = 0;k<A->row;k++){
        for(uint32 m = 0; m<A->colum; m++){
        printf("%2d ",matpoint[k][m]);
        }
        printf("\n");
        }
        printf("\n");
        ///////////////*/
        search_1 = search_col_1(matpoint, j, A->row);

        if(search_1!=j){
            if(search_1 == -1){
                //if(j ==
                return 0;
            }
            row_exchange(j,search_1, matpoint, A->colum);
            my_exchange(D+j*size, D+search_1*size, size);
        }

        for(i=0;i<A->row; i++){
            if(matpoint[i][j] != 0){
                if(i!= j){
                    //row_sub(i,j,inv_matpoint,A->colum,matpoint[j][j],matpoint[i][j]);
                    //row_sub(i,j,matpoint,A->colum,matpoint[j][j],matpoint[i][j]);
                    //row_or(i,j,inv_matpoint,A->colum);
                    row_or(i,j,matpoint,A->colum);
                    my_xor(D+i*size, D+j*size, size);
                }

                ///////////////////////////////
                /*for(uint32 k = 0;k<A->row;k++){
                for(uint32 m = 0; m<A->colum; m++){
                printf("%2d ",matpoint[k][m]);
                }
                printf("\n");
                }
                printf("\n");*/
            }
        }
    }
    matrix_free(matpoint,A->row);
    return 1;
}

static uint32 search_col_1(uint8 **mat, uint32 start, uint32 matsize){
	uint32 i,result=-1;
	int min = matsize;
	for(i=start; i<matsize; i++){
		if(mat[i][start] != 0){
			if(mat[i][start] == 1){
				//	printf("1111\n");
				result = i;
				break;
			}
			else
				if(abs((int)mat[i][start]) < min){
					min = abs((int)mat[i][start]);
					//		printf("not 1 111\n");
					result = i;
				}
		}
	}
	return result;
}

//行交换
static void row_exchange(uint32 row1, uint32 row2, uint8 **mat,uint32 matsize){
	uint32 i;
	/*uint8* temp;
	temp = (uint8*)malloc(matsize*sizeof(uint8));
	memcpy(temp,mat[row1],matsize);
	memcpy(mat[row1],mat[row2],matsize);
	memcpy(mat[row2],temp,matsize);
	free(temp);
	temp = NULL;*/
	for(i = 0; i<matsize; i++){
		mat[row1][i] = mat[row1][i]^mat[row2][i];
		mat[row2][i] = mat[row1][i]^mat[row2][i];
		mat[row1][i] = mat[row1][i]^mat[row2][i];
	}
}

//两列相减 row1 - row2
static void row_sub(uint32 row1, uint32 row2, uint8 **mat, uint32 matsize, uint8 temp1, uint8 temp2){
    uint32 i;

    if(temp1 == 1){
        for(i=0; i<matsize; i++){
            mat[row1][i] = mat[row1][i] - temp2*mat[row2][i];
        }
    }
    else{
        if(temp1 != 0){
            for(i=0; i<matsize; i++){
                mat[row1][i] = temp1*mat[row1][i] - temp2*mat[row2][i];
            }
        }
    }
}

//两行异或 row1异或row2，结果存在row1
static void row_or(uint32 row1, uint32 row2, uint8 **mat, uint32 matsize){
	uint32 i;
	for(i=0; i<matsize; i++){
		mat[row1][i] = mat[row1][i]^mat[row2][i];
	}

}

//释放资源
static void matrix_free(uint8 **mat, uint32 size){
	uint32 i;
	for(i=0; i<size; i++){
		free(mat[i]);
	}
	free(mat);
	mat = NULL;
}

static void my_xor(uint8 *x, uint8 *y, uint32 size){  // 计算X^Y, 结果保存在X中
    uint32 i;
    for(i = 0; i < size; i++)
        x[i] ^= y[i];
    return;
}

static void my_exchange(uint8 *x, uint8 *y, uint32 size){
    uint8* temp;
    temp = (uint8*)malloc(size);
    memcpy(temp, x, size);
    memcpy(x, y, size);
    memcpy(y, temp, size);
    free(temp);
    temp = NULL;
}
