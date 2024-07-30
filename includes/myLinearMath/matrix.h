//
// Created by adima on 19.07.2024.
//

#ifndef MATRIX_H
#define MATRIX_H

void applyIdentityMatrix(float* objectMatrix);
void applyEmptyMatrix(float* objectMatrix);

void getPerspectiveMatrix(float* matrix, float fov, float aspect, float near, float far);
void getViewMatrix(float* matrix, float* eye, float* center, float* up);

void multiplyMatrices(float* result, const float* a, const float* b);

void translateMatrix(float* matrix, float x, float y, float z);
void translateMatrixGlobal(float* matrix, float x, float y, float z);
void scaleMatrix(float* matrix, float x, float y, float z);
void rotateMatrix(float* matrix, float angle, float x, float y, float z);

void printMatrix(float* matrix);

#endif //MATRIX_H
