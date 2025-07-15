#include "asm/MatrixInteFunc.h"

char src_a_u8[16]__attribute__((aligned(4))) = {0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7};
char src_b_u8[16]__attribute__((aligned(4))) = {4, 5, 6, 7, 10, 11, 12, 13, 4, 5, 6, 7, 10, 11, 12, 13};
char  dst_u8[16]__attribute__((aligned(4)));
short int src_a_16[16]__attribute__((aligned(4))) = {0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7};
short int src_b_16[16]__attribute__((aligned(4))) = {4, 5, 6, 7, 10, 11, 12, 13, 4, 5, 6, 7, 10, 11, 12, 13};
short int dst_16[16]__attribute__((aligned(4)));
int src_a_32[16]__attribute__((aligned(4))) = {0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7};
int src_b_32[16]__attribute__((aligned(4))) = {4, 5, 6, 7, 10, 11, 12, 13, 4, 5, 6, 7, 10, 11, 12, 13};
int dst_32[16]__attribute__((aligned(4)));

MC MC_t;
MXYZ MXYZ_a, MXYZ_b, MXYZ_c;
void test_MatrixInteFunc_add_sub()
{
    printf(__func__);
    printf("ssssssnnn\n");
    MC_t.MatRow = 2 - 1;
    MC_t.MatCol = 4 - 1;
    MC_t.MatQ = 0;
    MC_t.RS = 0;

    MXYZ_a.MatAdr = (unsigned int)&src_a_16;
    MXYZ_a.Dw = DW_16;
    MXYZ_a.RowStep = MXYZ_a.Dw ;
    MXYZ_a.ColStep = MXYZ_a.Dw * (MC_t.MatCol + 1);
    MXYZ_a.Conj = 0;

    MXYZ_b.Dw = DW_16;
    MXYZ_b.MatAdr = (unsigned int)&src_b_16;
    MXYZ_b.RowStep = MXYZ_b.Dw ;
    MXYZ_b.ColStep = MXYZ_b.Dw * (MC_t.MatCol + 1);
    MXYZ_b.Conj = 0;

    MXYZ_c.Dw = DW_16;
    MXYZ_c.MatAdr = (unsigned int)&dst_16;
    MXYZ_c.RowStep = MXYZ_c.Dw ;
    MXYZ_c.ColStep = MXYZ_c.Dw * (MC_t.MatCol + 1);
    MXYZ_c.Conj = 0;

    MatrixADD_Real(&MXYZ_a, &MXYZ_b, &MXYZ_c, &MC_t);

    for (int k = 0; k < 8; k++) {
        printf("dst_u8=%d\n", dst_16[k]);
    }

    //sub
    //MXYZ_a.MatAdr = &src_a_16;
    MXYZ_a.Dw = DW_16;
    MXYZ_a.RowStep = MXYZ_a.Dw ;
    MXYZ_a.ColStep = MXYZ_a.Dw * (MC_t.MatCol + 1);
    MXYZ_a.Conj = 0;

    MXYZ_b.Dw = DW_16;
    MXYZ_b.MatAdr = (unsigned int)&src_b_16;
    MXYZ_b.RowStep = MXYZ_b.Dw ;
    MXYZ_b.ColStep = MXYZ_b.Dw * (MC_t.MatCol + 1);
    MXYZ_b.Conj = 0;

    MXYZ_c.Dw = DW_16;
    MXYZ_c.MatAdr = (unsigned int)&dst_16;
    MXYZ_c.RowStep = MXYZ_c.Dw ;
    MXYZ_c.ColStep = MXYZ_c.Dw * (MC_t.MatCol + 1);
    MXYZ_c.Conj = 0;

    MatrixSUB_Real(&MXYZ_a, &MXYZ_b, &MXYZ_c, &MC_t);
    for (int k = 0; k < 8; k++) {
        printf("dst_16=%d\n", dst_16[k]);
    }


}
void test_MatrixInteFunc_scale_add_sub()
{
    printf(__func__);
    printf("ssssssnnn\n");
    MC_t.MatRow = 2 - 1;
    MC_t.MatCol = 4 - 1;
    MC_t.MatQ = 1;
    MC_t.RS = 4;

    MXYZ_a.MatAdr = (unsigned int)&src_a_u8;
    MXYZ_a.RowStep = 1;
    MXYZ_a.ColStep = 4;
    MXYZ_a.Conj = 0;
    MXYZ_a.Dw = DW_8;

    MXYZ_b.MatAdr = (unsigned int)&src_b_u8;
    MXYZ_b.RowStep = 1;
    MXYZ_b.ColStep = 4;
    MXYZ_b.Conj = 0;
    MXYZ_b.Dw = DW_8;

    MXYZ_c.MatAdr = (unsigned int)&dst_u8;
    MXYZ_c.RowStep = 1;
    MXYZ_c.ColStep = 4;
    MXYZ_c.Conj = 0;
    MXYZ_c.Dw = DW_8;

    MatrixScaleADD_Real(&MXYZ_a, &MXYZ_b, &MXYZ_c, &MC_t);

    for (int k = 0; k < 8; k++) {
        printf("dst_u8=%d\n", dst_u8[k]);
    }

    //sub
    MXYZ_a.MatAdr = (unsigned int)&src_a_16;
    MXYZ_b.MatAdr = (unsigned int)&src_b_16;
    MXYZ_c.MatAdr = (unsigned int)&dst_16;
    MXYZ_a.Dw = DW_16;
    MXYZ_b.Dw = DW_16;
    MXYZ_c.Dw = DW_16;
    MXYZ_a.RowStep = 1 * 2;
    MXYZ_a.ColStep = 4 * 2;
    MXYZ_b.RowStep = 1 * 2;
    MXYZ_b.ColStep = 4 * 2;
    MXYZ_c.RowStep = 1 * 2;
    MXYZ_c.ColStep = 4 * 2;
    MatrixScaleSUB_Real(&MXYZ_a, &MXYZ_b, &MXYZ_c, &MC_t);
    for (int k = 0; k < 8; k++) {
        printf("dst_16=%d\n", dst_16[k]);
    }


}
void test_MatrixInteFunc_scale_mul_mla_mls()
{
    printf(__func__);
    printf("ssssssnnn\n");
    MC_t.MatRow = 2 - 1;
    MC_t.MatCol = 4 - 1;
    MC_t.MatQ = 1;
    MC_t.RS = 0;

    MXYZ_a.MatAdr = (unsigned int)&src_a_u8;
    MXYZ_a.RowStep = 1;
    MXYZ_a.ColStep = 4;
    MXYZ_a.Conj = 0;
    MXYZ_a.Dw = DW_8;

    MXYZ_b.MatAdr = (unsigned int)&src_b_u8;
    MXYZ_b.RowStep = 1;
    MXYZ_b.ColStep = 4;
    MXYZ_b.Conj = 0;
    MXYZ_b.Dw = DW_8;

    MXYZ_c.MatAdr = (unsigned int)&dst_u8;
    MXYZ_c.RowStep = 1;
    MXYZ_c.ColStep = 4;
    MXYZ_c.Conj = 0;
    MXYZ_c.Dw = DW_8;

    MatrixScaleMUL_Real(&MXYZ_a, &MXYZ_b, &MXYZ_c, &MC_t);
    //
    for (int k = 0; k < 8; k++) {
        printf("dst_u8=%d\n", dst_u8[k]);
    }

    for (int k = 0; k < 8; k++) {
        dst_32[k] = k;
    }
    MXYZ_a.MatAdr = (unsigned int)&src_a_32;
    MXYZ_b.MatAdr = (unsigned int)&src_b_32;
    MXYZ_c.MatAdr = (unsigned int)&dst_32;
    MXYZ_a.Dw = DW_32;
    MXYZ_b.Dw = DW_32;
    MXYZ_c.Dw = DW_32;
    MXYZ_a.RowStep = 1 * 4;
    MXYZ_a.ColStep = 4 * 4;
    MXYZ_b.RowStep = 1 * 4;
    MXYZ_b.ColStep = 4 * 4;
    MXYZ_c.RowStep = 1 * 4;
    MXYZ_c.ColStep = 4 * 4;

    MatrixScaleMLA_Real(&MXYZ_a, &MXYZ_b, &MXYZ_c, &MC_t);
    for (int k = 0; k < 8; k++) {
        printf("dst_16=%d\n", dst_32[k]);
    }


    //
    for (int k = 0; k < 8; k++) {
        dst_32[k] = -1;
    }

    MXYZ_a.MatAdr = (unsigned int)&src_a_32;
    MXYZ_b.MatAdr = (unsigned int)&src_b_32;
    MXYZ_c.MatAdr = (unsigned int)&dst_32;
    MXYZ_a.Dw = DW_32;
    MXYZ_b.Dw = DW_32;
    MXYZ_c.Dw = DW_32;
    MatrixScaleMLS_Real(&MXYZ_a, &MXYZ_b, &MXYZ_c, &MC_t);
    for (int k = 0; k < 8; k++) {
        printf("dst_16=%d\n", dst_32[k]);
    }

}
void test_Matvec_mul()
{
    printf(__func__);
    printf("ssssssnnn\n");
    MC_t.MatRow = 2 - 1;
    MC_t.MatCol = 4 - 1;
    MC_t.MatQ = 0;
    MC_t.RS = 0;

    MXYZ_a.MatAdr = (unsigned int)&src_a_32;
    MXYZ_a.RowStep = 1 * 4;
    MXYZ_a.ColStep = 4 * 4;
    MXYZ_a.Conj = 0;
    MXYZ_a.Dw = DW_32;

    MXYZ_b.MatAdr = (unsigned int)&src_b_32;
    MXYZ_b.RowStep = 1 * 4;
    MXYZ_b.ColStep = 4 * 4;
    MXYZ_b.Conj = 0;
    MXYZ_b.Dw = DW_32;

    /*MXYZ_c.MatAdr = &dst_32;
    MXYZ_c.RowStep = 1;
    MXYZ_c.ColStep = 4;
    MXYZ_c.Conj = 0;
    MXYZ_c.Dw = DW_32;*/
    int sum;
    MatVecMUL_Real(&MXYZ_a, &MXYZ_b, &sum, &MC_t, 1);
    //
    /*for(int k = 0;k<8;k++)
    {
    	printf("dst_u8=%d\n",dst_32[k]);
    }*/
    printf("sum=%d\n", sum);
}


//complex
void test_MatrixInteFunc_add_sub_complex()
{
    printf(__func__);
    printf("ssssssnnn\n");
    MC_t.MatRow = 2 - 1;
    MC_t.MatCol = 2 - 1;
    MC_t.MatQ = 0;
    MC_t.RS = 0;

    MXYZ_a.MatAdr = (unsigned int)&src_a_u8;
    MXYZ_a.RowStep = 1 * 2;
    MXYZ_a.ColStep = 4;
    MXYZ_a.Conj = 0;
    MXYZ_a.Dw = DW_8;

    MXYZ_b.MatAdr = (unsigned int)&src_b_u8;
    MXYZ_b.RowStep = 1 * 2;
    MXYZ_b.ColStep = 4;
    MXYZ_b.Conj = 0;
    MXYZ_b.Dw = DW_8;

    MXYZ_c.MatAdr = (unsigned int)&dst_u8;
    MXYZ_c.RowStep = 1 * 2;
    MXYZ_c.ColStep = 4;
    MXYZ_c.Conj = 0;
    MXYZ_c.Dw = DW_8;

    MatrixADD_Complex(&MXYZ_a, &MXYZ_b, &MXYZ_c, &MC_t);

    for (int k = 0; k < 8; k++) {
        printf("dst_u8=%d\n", dst_u8[k]);
    }

    //sub
    MXYZ_a.MatAdr = (unsigned int)&src_a_16;
    MXYZ_b.MatAdr = (unsigned int)&src_b_16;
    MXYZ_c.MatAdr = (unsigned int)&dst_16;
    MXYZ_a.Dw = DW_16;
    MXYZ_b.Dw = DW_16;
    MXYZ_c.Dw = DW_16;
    MXYZ_a.RowStep = 1 * 2 * 2;
    MXYZ_a.ColStep = 4 * 2;
    MXYZ_b.RowStep = 1 * 2 * 2;
    MXYZ_b.ColStep = 4 * 2;
    MXYZ_c.RowStep = 1 * 2 * 2;
    MXYZ_c.ColStep = 4 * 2;
    MatrixSUB_Complex(&MXYZ_a, &MXYZ_b, &MXYZ_c, &MC_t);
    for (int k = 0; k < 8; k++) {
        printf("dst_16=%d\n", dst_16[k]);
    }


}
void test_MatrixInteFunc_scale_add_sub_complex()
{
    printf(__func__);
    printf("ssssssnnn\n");
    MC_t.MatRow = 2 - 1;
    MC_t.MatCol = 2 - 1;
    MC_t.MatQ = 1;
    MC_t.RS = 4;

    MXYZ_a.MatAdr = (unsigned int)&src_a_u8;
    MXYZ_a.RowStep = 1 * 2;
    MXYZ_a.ColStep = 4;
    MXYZ_a.Conj = 0;
    MXYZ_a.Dw = DW_8;

    MXYZ_b.MatAdr = (unsigned int)&src_b_u8;
    MXYZ_b.RowStep = 1 * 2;
    MXYZ_b.ColStep = 4;
    MXYZ_b.Conj = 0;
    MXYZ_b.Dw = DW_8;

    MXYZ_c.MatAdr = (unsigned int)&dst_u8;
    MXYZ_c.RowStep = 1 * 2;
    MXYZ_c.ColStep = 4;
    MXYZ_c.Conj = 0;
    MXYZ_c.Dw = DW_8;

    MatrixScaleADD_Complex(&MXYZ_a, &MXYZ_b, &MXYZ_c, &MC_t);

    for (int k = 0; k < 8; k++) {
        printf("dst_u8=%d\n", dst_u8[k]);
    }

    //sub
    MXYZ_a.MatAdr = (unsigned int)&src_a_16;
    MXYZ_b.MatAdr = (unsigned int)&src_b_16;
    MXYZ_c.MatAdr = (unsigned int)&dst_16;
    MXYZ_a.Dw = DW_16;
    MXYZ_b.Dw = DW_16;
    MXYZ_c.Dw = DW_16;
    MXYZ_a.RowStep = 1 * 2 * 2;
    MXYZ_a.ColStep = 4 * 2;
    MXYZ_b.RowStep = 1 * 2 * 2;
    MXYZ_b.ColStep = 4 * 2;
    MXYZ_c.RowStep = 1 * 2 * 2;
    MXYZ_c.ColStep = 4 * 2;

    MatrixScaleSUB_Complex(&MXYZ_a, &MXYZ_b, &MXYZ_c, &MC_t);
    for (int k = 0; k < 8; k++) {
        printf("dst_16=%d\n", dst_16[k]);
    }


}
void test_MatrixInteFunc_scale_mul_mla_mls_complex()
{
    printf(__func__);
    printf("ssssssnnn\n");
    MC_t.MatRow = 2 - 1;
    MC_t.MatCol = 2 - 1;
    MC_t.MatQ = 1;
    MC_t.RS = 0;

    MXYZ_a.MatAdr = (unsigned int)&src_a_u8;
    MXYZ_a.RowStep = 1 * 2;
    MXYZ_a.ColStep = 4;
    MXYZ_a.Conj = 0;
    MXYZ_a.Dw = DW_8;

    MXYZ_b.MatAdr = (unsigned int)&src_b_u8;
    MXYZ_b.RowStep = 1 * 2;
    MXYZ_b.ColStep = 4;
    MXYZ_b.Conj = 0;
    MXYZ_b.Dw = DW_8;

    MXYZ_c.MatAdr = (unsigned int)&dst_u8;
    MXYZ_c.RowStep = 1 * 2;
    MXYZ_c.ColStep = 4;
    MXYZ_c.Conj = 0;
    MXYZ_c.Dw = DW_8;

    MatrixScaleMUL_Complex(&MXYZ_a, &MXYZ_b, &MXYZ_c, &MC_t);
    //
    for (int k = 0; k < 8; k++) {
        printf("dst_u8=%d\n", dst_u8[k]);
    }

    for (int k = 0; k < 8; k++) {
        dst_32[k] = k;
    }
    MXYZ_a.MatAdr = (unsigned int)&src_a_32;
    MXYZ_b.MatAdr = (unsigned int)&src_b_32;
    MXYZ_c.MatAdr = (unsigned int)&dst_32;
    MXYZ_a.Dw = DW_32;
    MXYZ_b.Dw = DW_32;
    MXYZ_c.Dw = DW_32;
    MXYZ_a.RowStep = 1 * 4 * 2;
    MXYZ_a.ColStep = 4 * 4;
    MXYZ_b.RowStep = 1 * 4 * 2;
    MXYZ_b.ColStep = 4 * 4;
    MXYZ_c.RowStep = 1 * 4 * 2;
    MXYZ_c.ColStep = 4 * 4;
    MatrixScaleMLA_Complex(&MXYZ_a, &MXYZ_b, &MXYZ_c, &MC_t);
    for (int k = 0; k < 8; k++) {
        printf("dst_16=%d\n", dst_32[k]);
    }


    //
    for (int k = 0; k < 8; k++) {
        dst_32[k] = -1;
    }

    MXYZ_a.MatAdr = (unsigned int)&src_a_32;
    MXYZ_b.MatAdr = (unsigned int)&src_b_32;
    MXYZ_c.MatAdr = (unsigned int)&dst_32;
    MXYZ_a.Dw = DW_32;
    MXYZ_b.Dw = DW_32;
    MXYZ_c.Dw = DW_32;
    MatrixScaleMLS_Complex(&MXYZ_a, &MXYZ_b, &MXYZ_c, &MC_t);
    for (int k = 0; k < 8; k++) {
        printf("dst_16=%d\n", dst_32[k]);
    }

}
void test_Matvec_mul_complex()
{
    printf(__func__);
    printf("ssssssnnn\n");
    MC_t.MatRow = 2 - 1;
    MC_t.MatCol = 2 - 1;
    MC_t.MatQ = 0;
    MC_t.RS = 0;

    MXYZ_a.MatAdr = (unsigned int)&src_a_32;
    MXYZ_a.RowStep = 1 * 4 * 2;
    MXYZ_a.ColStep = 4 * 4;
    MXYZ_a.Conj = 0;
    MXYZ_a.Dw = DW_32;

    MXYZ_b.MatAdr = (unsigned int)&src_b_32;
    MXYZ_b.RowStep = 1 * 4 * 2;
    MXYZ_b.ColStep = 4 * 4;
    MXYZ_b.Conj = 0;
    MXYZ_b.Dw = DW_32;

    /*MXYZ_c.MatAdr = &dst_32;
    MXYZ_c.RowStep = 1;
    MXYZ_c.ColStep = 4;
    MXYZ_c.Conj = 0;
    MXYZ_c.Dw = DW_32;*/
    int sum[2];
    MatVecMUL_Complex(&MXYZ_a, &MXYZ_b, (int *)&sum, &MC_t, -1);
    //
    /*for(int k = 0;k<8;k++)
    {
    	printf("dst_u8=%d\n",dst_32[k]);
    }*/
    printf("sum1=%d\n", sum[0]);
    printf("sum2=%d\n", sum[1]);
}

void test_MatrixComplexScaleMul_Complex()
{
    printf(__func__);
    printf("ssssssnnn\n");
    MC_t.MatRow = 2 - 1;
    MC_t.MatCol = 2 - 1;
    MC_t.MatQ = 1;
    MC_t.RS = 0;

    MXYZ_a.MatAdr = (unsigned int)&src_a_32;
    MXYZ_a.RowStep = 1 * 4;
    MXYZ_a.ColStep = 4 * 2;
    MXYZ_a.Conj = 0;
    MXYZ_a.Dw = DW_32;

    MXYZ_b.MatAdr = (unsigned int)&src_b_32;
    MXYZ_b.RowStep = 1 * 4 * 2;
    MXYZ_b.ColStep = 4 * 2 * 2;
    MXYZ_b.Conj = 0;
    MXYZ_b.Dw = DW_32;

    MXYZ_c.MatAdr = (unsigned int)&dst_32;
    MXYZ_c.RowStep = 1 * 4 * 2;
    MXYZ_c.ColStep = 4 * 2 * 2;
    MXYZ_c.Conj = 0;
    MXYZ_c.Dw = DW_32;


    MatrixComplexScaleMUL_Complex(&MXYZ_a, &MXYZ_b, &MXYZ_c, &MC_t);
    for (int k = 0; k < 8; k++) {
        printf("dst_u8=%d\n", dst_32[k]);
    }
}
void test_MatrixMUL_Real()
{
    printf(__func__);
    printf("ssssssnnn\n");

    MC_t.MatRow = 2 - 1;
    MC_t.MatCol = 4 - 1;
    MC_t.MatQ = 0;
    MC_t.RS = 0;

    MXYZ_a.MatAdr = (unsigned int)&src_a_32;
    MXYZ_a.RowStep = 4;
    MXYZ_a.ColStep = 4 * 4;
    MXYZ_a.Conj = 0;
    MXYZ_a.Dw = DW_32;

    int src_b_32_T[8]__attribute__((aligned(4))) = {4, 6, 10, 12, 5, 7, 11, 13};

    MXYZ_b.MatAdr = (unsigned int)&src_b_32_T;
    MXYZ_b.RowStep = 4;
    MXYZ_b.ColStep = 4 * 4;
    MXYZ_b.Conj = 0;
    MXYZ_b.Dw = DW_32;

    MXYZ_c.MatAdr = (unsigned int)&dst_32;
    MXYZ_c.RowStep = 4;
    MXYZ_c.ColStep = 4 * 4;
    MXYZ_c.Conj = 0;
    MXYZ_c.Dw = DW_32;

    MatrixMUL_Real(&MXYZ_b, &MXYZ_a, &MXYZ_c, &MC_t, -1);
    //
    for (int k = 0; k < 4; k++) {
        printf("dst_u8=%d\n", dst_32[k]);
    }
}

void test_MatrixMUL_Complex()
{
    printf(__func__);
    printf("ssssssnnn\n");

    MC_t.MatRow = 2 - 1;
    MC_t.MatCol = 2 - 1;
    MC_t.MatQ = 0;
    MC_t.RS = 0;

    MXYZ_a.MatAdr = (unsigned int)&src_a_32;
    MXYZ_a.RowStep = 4;
    MXYZ_a.ColStep = 4 * 4;
    MXYZ_a.Conj = 0;
    MXYZ_a.Dw = DW_32;

    int src_b_32_T1[8]__attribute__((aligned(4))) = {4, 5, 10, 11, 6, 7, 12, 13};

    MXYZ_b.MatAdr = (unsigned int)&src_b_32_T1;
    MXYZ_b.RowStep = 4;
    MXYZ_b.ColStep = 4 * 4;
    MXYZ_b.Conj = 0;
    MXYZ_b.Dw = DW_32;

    MXYZ_c.MatAdr = (unsigned int)&dst_32;
    MXYZ_c.RowStep = 4;
    MXYZ_c.ColStep = 4 * 4;
    MXYZ_c.Conj = 0;
    MXYZ_c.Dw = DW_32;

    MatrixMUL_Complex(&MXYZ_b, &MXYZ_a, &MXYZ_c, &MC_t, -1);
    //
    for (int k = 0; k < 8; k++) {
        printf("dst_u8=%d\n", dst_32[k]);
    }
}

