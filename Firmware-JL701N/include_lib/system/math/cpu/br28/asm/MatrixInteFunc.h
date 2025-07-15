#ifndef MatrixInteFunc_H
#define MatrixInteFunc_H

#include "stdio.h"

// DW_Width define
#define DW_8 1	//001
#define DW_16 2 //010
#define DW_32 4 //100

// Real define
#define MATR_ADD 17		 //010001
#define MATR_SUB 25		 //011001
#define MATR_SCALEADD 19 //010011
#define MATR_SCALESUB 27 //011011
#define MATR_SCALEMUL 33 //100001
#define MATR_SCALEMLA 35 //100011
#define MATR_SCALEMLS 43 //101011
#define MATRVECT_MUL 37	 //100101

// Complex define
#define MATC_ADD 16		  //010000
#define MATC_SUB 24		  //011000
#define MATC_SCALEADD 18  //010010
#define MATC_SCALESUB 26  //011010
#define MATC_CSCALEMUL 30 //011110
#define MATC_SCALEMUL 32  //100000
#define MATC_SCALEMLA 34  //100010
#define MATC_SCALEMLS 42  //101010
#define MATCVECT_MUL 36	  //100100

//struct define for parameter configuration
typedef struct {
    unsigned short MatRow;
    unsigned short MatCol;
    unsigned short MatQ;
    unsigned int RS;

} MC;

typedef struct {
    unsigned int MatAdr;
    unsigned short RowStep;
    unsigned short ColStep;
    unsigned short Conj;
    unsigned short Dw;
} MXYZ;

//Hardware parameter configuration structure
typedef struct {
    unsigned int matrix_con;
    unsigned int matrix_xadr;
    unsigned int matrix_yadr;
    unsigned int matrix_zadr;

    unsigned int matrix_rs;

    unsigned int matrix_xconfig;
    unsigned int matrix_yconfig;
    unsigned int matrix_zconfig;
} matrix_config;



/*
typedef struct
{
	volatile unsigned int CON;
	volatile unsigned int CADR;
	volatile unsigned int ACC0H;
	volatile unsigned int ACC0L;
	volatile unsigned int ACC1H;
	volatile unsigned int ACC1L;
} JL_FFT_TypeDef;

#define JL_FFT_BASE 0x102000
#define JL_FFT ((JL_FFT_TypeDef *)JL_FFT_BASE)
*/

//*********** Z = Y + X ***********//
void MatrixADD_Real(MXYZ *mx, MXYZ *my, MXYZ *mz, MC *mc);

//*********** Z = Y - X ***********//
void MatrixSUB_Real(MXYZ *mx, MXYZ *my, MXYZ *mz, MC *mc);

//*********** Z = Y + X*RS/(2^Q) ***********//
void MatrixScaleADD_Real(MXYZ *mx, MXYZ *my, MXYZ *mz, MC *mc);

//*********** Z = Y - X*RS/(2^Q) ***********//
void MatrixScaleSUB_Real(MXYZ *mx, MXYZ *my, MXYZ *mz, MC *mc);

//*********** Z = Y .* X/(2^Q) ***********//
void MatrixScaleMUL_Real(MXYZ *mx, MXYZ *my, MXYZ *mz, MC *mc);

//*********** Z = Z + Y .* X/(2^Q) ***********//
void MatrixScaleMLA_Real(MXYZ *mx, MXYZ *my, MXYZ *mz, MC *mc);

//*********** Z = Z - Y .* X/(2^Q) ***********//
void MatrixScaleMLS_Real(MXYZ *mx, MXYZ *my, MXYZ *mz, MC *mc);

//*********** Z_r = Y_v .* X_v ***********//
void MatVecMUL_Real(MXYZ *mx, MXYZ *my, int *rst, MC *mc, int rQ);

//*********** Z = Y * X ***********//
void MatrixMUL_Real(MXYZ *mx, MXYZ *my, MXYZ *mz, MC *mc, int rQ);

//*********** Z = Y + X (Complex) ***********//
void MatrixADD_Complex(MXYZ *mx, MXYZ *my, MXYZ *mz, MC *mc);

//*********** Z = Y - X (Complex) ***********//
void MatrixSUB_Complex(MXYZ *mx, MXYZ *my, MXYZ *mz, MC *mc);

//*********** Z = Y + X*RS/(2^Q) (Complex) ***********//
void MatrixScaleADD_Complex(MXYZ *mx, MXYZ *my, MXYZ *mz, MC *mc);

//*********** Z = Y - X*RS/(2^Q) (Complex) ***********//
void MatrixScaleSUB_Complex(MXYZ *mx, MXYZ *my, MXYZ *mz, MC *mc);

//*********** Z_c = Y_c .* X_r/(2^Q) (Complex) ***********//
void MatrixComplexScaleMUL_Complex(MXYZ *mx, MXYZ *my, MXYZ *mz, MC *mc);

//*********** Z = Y .* X/(2^Q) (Complex) ***********//
void MatrixScaleMUL_Complex(MXYZ *mx, MXYZ *my, MXYZ *mz, MC *mc);

//*********** Z = Z + Y .* X/(2^Q) (Complex) ***********//
void MatrixScaleMLA_Complex(MXYZ *mx, MXYZ *my, MXYZ *mz, MC *mc);

//*********** Z = Z - Y .* X/(2^Q) (Complex) ***********//
void MatrixScaleMLS_Complex(MXYZ *mx, MXYZ *my, MXYZ *mz, MC *mc);

//*********** Z_r = Y_v .* X_v (Complex) ***********//
void MatVecMUL_Complex(MXYZ *mx, MXYZ *my, int *rst, MC *mc, int rQ);

//*********** Z = Y * X (Complex) ***********//
void MatrixMUL_Complex(MXYZ *mx, MXYZ *my, MXYZ *mz, MC *mc, int rQ);

#endif
