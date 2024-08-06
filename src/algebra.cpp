#include "algebra.h"
float mix(float value1, float value2, float factor)
{
    float result;
    result = value1*factor + value2*(1.0-factor);
    return result;
}
vec3 vector_multiplication(vec3 v1, vec3 v2)
{
    vec3 v3;
    v3[0] = v1[1]*v2[2]-v1[2]*v2[1];
    v3[1] = v1[2]*v2[0]-v1[0]*v2[2];
    v3[2] = v1[0]*v2[1]-v1[1]*v2[0];

    return v3;
}
mat4 Invert2(mat4 mat)
{
    double tmp[12];/* temp array for pairs */
    double src[16];/* array of transpose source matrix */
    double det;/* determinant*/
    double dst[16];
    /* transpose matrix */
    for (int i = 0; i < 4; i++)
    {
        src[i] = mat[i*4];
        src[i + 4] = mat[i*4 + 1];
        src[i + 8] = mat[i*4 + 2];
        src[i + 12] = mat[i*4 + 3];
    }/* calculate pairs for first 8 elements (cofactors) */
    tmp[0] = src[10] * src[15];
    tmp[1] = src[11] * src[14];
    tmp[2] = src[9] * src[15];
    tmp[3] = src[11] * src[13];
    tmp[4] = src[9] * src[14];
    tmp[5] = src[10] * src[13];
    tmp[6] = src[8] * src[15];
    tmp[7] = src[11] * src[12];
    tmp[8] = src[8] * src[14];
    tmp[9] = src[10] * src[12];
    tmp[10] = src[8] * src[13];
    tmp[11] = src[9] * src[12];
    /* calculate first 8 elements (cofactors) */
    dst[0] = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7];
    dst[0] -= tmp[1]*src[5] + tmp[2]*src[6] + tmp[5]*src[7];
    dst[1] = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7];
    dst[1] -= tmp[0]*src[4] + tmp[7]*src[6] + tmp[8]*src[7];
    dst[2] = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7];
    dst[2] -= tmp[3]*src[4] + tmp[6]*src[5] + tmp[11]*src[7];
    dst[3] = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6];
    dst[3] -= tmp[4]*src[4] + tmp[9]*src[5] + tmp[10]*src[6];
    dst[4] = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3];
    dst[4] -= tmp[0]*src[1] + tmp[3]*src[2] + tmp[4]*src[3];
    dst[5] = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3];
    dst[5] -= tmp[1]*src[0] + tmp[6]*src[2] + tmp[9]*src[3];
    dst[6] = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3];
    dst[6] -= tmp[2]*src[0] + tmp[7]*src[1] + tmp[10]*src[3];
    dst[7] = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2];
    dst[7] -= tmp[5]*src[0] + tmp[8]*src[1] + tmp[11]*src[2];/* calculate pairs for second 8 elements (cofactors) */
    tmp[0] = src[2]*src[7];
    tmp[1] = src[3]*src[6];
    tmp[2] = src[1]*src[7];
    tmp[3] = src[3]*src[5];
    tmp[4] = src[1]*src[6];
    tmp[5] = src[2]*src[5];
    tmp[6] = src[0]*src[7];
    tmp[7] = src[3]*src[4];
    tmp[8] = src[0]*src[6];
    tmp[9] = src[2]*src[4];
    tmp[10] = src[0]*src[5];
    tmp[11] = src[1]*src[4];
    /* calculate second 8 elements (cofactors) */
    dst[8] = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15];
    dst[8] -= tmp[1]*src[13] + tmp[2]*src[14] + tmp[5]*src[15];
    dst[9] = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15];
    dst[9] -= tmp[0]*src[12] + tmp[7]*src[14] + tmp[8]*src[15];
    dst[10] = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15];
    dst[10]-= tmp[3]*src[12] + tmp[6]*src[13] + tmp[11]*src[15];
    dst[11] = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14];
    dst[11]-= tmp[4]*src[12] + tmp[9]*src[13] + tmp[10]*src[14];
    dst[12] = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9];
    dst[12]-= tmp[4]*src[11] + tmp[0]*src[9] + tmp[3]*src[10];
    dst[13] = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10];
    dst[13]-= tmp[6]*src[10] + tmp[9]*src[11] + tmp[1]*src[8];
    dst[14] = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8];
    dst[14]-= tmp[10]*src[11] + tmp[2]*src[8] + tmp[7]*src[9];
    dst[15] = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9];
    dst[15]-= tmp[8]*src[9] + tmp[11]*src[10] + tmp[5]*src[8];

    /* calculate determinant */
    det=src[0]*dst[0]+src[1]*dst[1]+src[2]*dst[2]+src[3]*dst[3];/* calculate matrix inverse */
    det = 1/det;
    mat4 matret;
    for (int j = 0; j < 16; j++)
    {
        dst[j] *= det;
        matret[j] = dst[j];
    }
    return matret;
}
mat4 matrix_multiplication(mat4 m_1, mat4 m_2)
{
    mat4 m_rez;
    for(uint8_t y = 0; y < 4; y++)
    {
        for(uint8_t x = 0; x < 4; x++)
        {
            m_rez[4*y+x] =
            m_1[4*y+0]*m_2[4*0+x] +
            m_1[4*y+1]*m_2[4*1+x] +
            m_1[4*y+2]*m_2[4*2+x] +
            m_1[4*y+3]*m_2[4*3+x];
        }
    }
    return m_rez;
}
/*mat4 matrix_multiplication(mat4 m_1, mat4 m_2)
{
    mat4 m_rez;
    for(uint8_t i = 0; i < 4; i++)
    {
        for(uint8_t j = 0; j < 4; j++)
        {
            m_rez[i][j] = 0.f;
            for (int k = 0; k < 4; ++k)
            {
                m_rez[i][j] += m_1[i][k] * m_2[k][j];
            }
        }
    }
    return m_rez;
}*/
/*mat4 MatrixFromQuaterion(Quaternion quat)
{
    mat4 matrix =
    {
        1.0-2.0*(quat.y*quat.y+quat.z*quat.z), 2.0*(quat.x*quat.y+quat.w*quat.z), 2.0*(quat.x*quat.z-quat.w*quat.y), 0,
        2.0*(quat.x*quat.y-quat.w*quat.z), 1.0-2.0*(quat.x*quat.x+quat.z*quat.z), 2.0*(quat.y*quat.z+quat.w*quat.x), 0,
        2.0*(quat.x*quat.z+quat.w*quat.y), 2.0*(quat.y*quat.z-quat.w*quat.x), 1.0-2.0*(quat.x*quat.x+quat.y*quat.y), 0,
        0, 0, 0, 1,
    };
    return matrix;
}*/
/*Quaternion QuaterionFromMatrix(mat4 matrix)
{
    float T = matrix[0] + matrix[5] + matrix[10] + 1;
    float W, X, Y, Z;
    if(T > 0.0)
    {
        float S = 0.5 / sqrt(T);
        W = 0.25 / S;
        X = ( matrix[6] - matrix[9] ) * S;
        Y = ( matrix[8] - matrix[2] ) * S;
        Z = ( matrix[1] - matrix[4] ) * S;
    }
    else
    {
        float max_data = matrix[0];
        if(matrix[5] > max_data)
            max_data = matrix[5];
        if(matrix[10] > max_data)
            max_data = matrix[10];
        float S = sqrt(matrix[0] + matrix[5] + matrix[10] + 1.0)*2;
        if(max_data == matrix[0])
        {
            W = (matrix[9] - matrix[6]) / S;
            X = 0.5 / S;
            Y = (matrix[4] - matrix[1]) / S;
            Z = (matrix[8] - matrix[2]) / S;
        }
        else if(max_data == matrix[5])
        {
            W = (matrix[8] - matrix[2]) / S;
            X = (matrix[4] - matrix[1]) / S;
            Y = 0.5 / S;
            Z = (matrix[9] - matrix[6]) / S;
        }
        else if(max_data == matrix[10])
        {
            W = (matrix[4] - matrix[1]) / S;
            X = (matrix[8] - matrix[2]) / S;
            Y = (matrix[8] - matrix[2]) / S;
            Z = 0.5 / S;
        }
    }
    return Quaternion(W, X, Y, Z);
}*/
