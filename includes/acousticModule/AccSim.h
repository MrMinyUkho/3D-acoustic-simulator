//
// Created by adima on 07.07.2024.
//

#ifndef ACCSIM_H
#define ACCSIM_H

typedef struct cell {
    float x, y, z;
} asCellPos;

typedef struct cellParam {
    float velocity;
    float amplitude;
} asCellParam;

typedef struct cellVolume {
    int size_x;
    int size_y;
    int size_z;
} asCellsVolume;

void asInitCellVolume( float w, float h, float l, float density, int drawStep);
unsigned long long getOffsetS(int x, int y, int z);
void asDrawVolume(const float* model, const float* view, const float* projection);
ull getOffsetS(int x, int y, int z);
void asDestroyVolume();


#endif //ACCSIM_H
