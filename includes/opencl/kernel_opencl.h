//
// Created by adima on 25.07.2024.
//

#ifndef KERNEL_OPENCL_H
#define KERNEL_OPENCL_H

#include "../defines.h"

typedef struct cellToCompute {
    float vel, amp, weight;
} CellToCompute;

void initClProgram(ull size_x, ull size_y, ull size_z, int drawStep, GLuint VBO1, GLuint pointsVBO, float* verticies, float* matrls, int16_t* matIdx, uint16_t faceNum, uint16_t matCount);
void tickCompute();
void setAmpData(float x, float y, float z, float amp);
void getClFreeMem(ull* total_mem, ull* free_mem);
void destroyClProgram();
void printClDevives();

#endif //KERNEL_OPENCL_H
