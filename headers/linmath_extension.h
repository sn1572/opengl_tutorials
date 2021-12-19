#include "linmath.h"
#include <math.h>


void mat4x4_print(mat4x4 const M){
    for (int i=0; i<4; i++){
        for (int j=0; j<4; j++){
            printf("%4.2f ", M[i][j]);
        }
        printf("\n");
    }
}


LINMATH_H_FUNC float mat4x4_sub_determinant(mat4x4 const M)
{
    float out = 0.f;
    out += M[0][0]*(M[1][1]*M[2][2]-M[1][2]*M[2][1]);
    out -= M[0][1]*(M[1][0]*M[2][2]-M[2][0]*M[1][2]);
    out += M[0][2]*(M[1][0]*M[2][1]-M[2][0]*M[1][1]);
    return out;
}


LINMATH_H_FUNC void mat4x4_normal_matrix(mat4x4 out, mat4x4 const in)
{
    int i=0, j=0;
    #ifdef LINALG_DEBUG
    if (fabs(mat4x4_sub_determinant(in)) < 0.00001f){
        mat4x4_dup(out, in);
        return;
    }
    #endif
    mat4x4 tmp1 = {{0.f, 0.f, 0.f, 0.f},
                   {0.f, 0.f, 0.f, 0.f},
                   {0.f, 0.f, 0.f, 0.f},
                   {0.f, 0.f, 0.f, 1.f}};
    mat4x4 tmp2;

    for (; i<3; i++){
        for (j=0; j<3; j++){
            tmp1[i][j] = in[i][j];
        }
    }
    mat4x4_invert(tmp2, tmp1);
    mat4x4_transpose(out, tmp2);
}
