//
// Created by adima on 19.07.2024.
//

#include "matrix.h"

#include <math.h>
#include <string.h>
#include <stdio.h>

#define DEG2RAD 0.01745329251f

float identityMatrix[16] = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

float emptyMatrix[16] = {
    0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f
};

void applyIdentityMatrix(float* objectMatrix) {
    // Обновите объектную матрицу с единичной матрицей
    memcpy(objectMatrix, identityMatrix, sizeof(identityMatrix));
}

void applyEmptyMatrix(float* objectMatrix) {
    // Обновите объектную матрицу с пустой матрицей
    memcpy(objectMatrix, emptyMatrix, sizeof(emptyMatrix));
}

void getPerspectiveMatrix(float* matrix, const float fov, const float aspect, const float near, const float far) {
    const float tanHalfFovy = tanf(fov * DEG2RAD  / 2.0f);

    matrix[0] = 1.0f / (aspect * tanHalfFovy);
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;

    matrix[4] = 0.0f;
    matrix[5] = 1.0f / tanHalfFovy;
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;

    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = -(far + near) / (far - near);
    matrix[11] = -1.0f;

    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = -2.0f * far * near / (far - near);
    matrix[15] = 0.0f;
}

void getViewMatrix(float* matrix, float* eye, float* center, float* up) {
    float f[3], s[3], u[3];
    for (int i = 0; i < 3; i++) f[i] = center[i] - eye[i];
    float len = sqrtf(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
    for (int i = 0; i < 3; i++) f[i] /= len;

    s[0] = up[1] * f[2] - up[2] * f[1];
    s[1] = up[2] * f[0] - up[0] * f[2];
    s[2] = up[0] * f[1] - up[1] * f[0];
    len = sqrtf(s[0] * s[0] + s[1] * s[1] + s[2] * s[2]);
    for (int i = 0; i < 3; i++) s[i] /= len;

    u[0] = f[1] * s[2] - f[2] * s[1];
    u[1] = f[2] * s[0] - f[0] * s[2];
    u[2] = f[0] * s[1] - f[1] * s[0];

    matrix[0] = s[0];
    matrix[1] = u[0];
    matrix[2] = -f[0];
    matrix[3] = 0.0f;

    matrix[4] = s[1];
    matrix[5] = u[1];
    matrix[6] = -f[1];
    matrix[7] = 0.0f;

    matrix[8] = s[2];
    matrix[9] = u[2];
    matrix[10] = -f[2];
    matrix[11] = 0.0f;

    matrix[12] = -(s[0] * eye[0] + s[1] * eye[1] + s[2] * eye[2]);
    matrix[13] = -(u[0] * eye[0] + u[1] * eye[1] + u[2] * eye[2]);
    matrix[14] = f[0] * eye[0] + f[1] * eye[1] + f[2] * eye[2];
    matrix[15] = 1.0f;
}

void multiplyMatrices(float* result, const float* a, const float* b) {
    float temp[16];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            temp[i * 4 + j] = 0.0f;
            for (int k = 0; k < 4; k++) {
                temp[i * 4 + j] += a[i * 4 + k] * b[k * 4 + j];
            }
        }
    }
    memcpy(result, temp, sizeof(temp));
}

void translateMatrix(float* matrix, float x, float y, float z) {
    float translationMatrix[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
           x,    y,    z, 1.0f
    };
    multiplyMatrices(matrix, matrix, translationMatrix);
}

void translateMatrixGlobal(float *matrix, float x, float y, float z) {
    matrix[12] += x;
    matrix[13] += y;
    matrix[14] += z;
}

void scaleMatrix(float* matrix, float x, float y, float z) {
    float scalingMatrix[16] = {
           x, 0.0f, 0.0f, 0.0f,
        0.0f,    y, 0.0f, 0.0f,
        0.0f, 0.0f,    z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    multiplyMatrices(matrix, matrix, scalingMatrix);
}

void rotateMatrix(float* matrix, float angle, float x, float y, float z) {
    float rad = angle * DEG2RAD;
    float c = cosf(rad);
    float s = sinf(rad);
    float t = 1.0f - c;

    float rotationMatrix[16] = {
        t*x*x + c, t*x*y - s*z, t*x*z + s*y, 0.0f,
        t*x*y + s*z, t*y*y + c, t*y*z - s*x, 0.0f,
        t*x*z - s*y, t*y*z + s*x, t*z*z + c, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    multiplyMatrices(matrix, matrix, rotationMatrix);
}

void printMatrix(float* matrix) {
    for(int i = 0; i < 16; ++i) {
        printf("%f ", matrix[i]);
        if((i+1)%4 == 0) printf("\n");
    }
    printf("\n");
}
