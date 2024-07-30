//
// Created by adima on 20.07.2024.
//

#ifndef VECTOR_H
#define VECTOR_H

#define VEC2FLOAT(v, f) f[0] = v.x; f[1] = v.y; f[2] = v.z
#define PRINT_VEC3(v) printf("%f %f %f\n", v.x, v.y, v.z);


typedef struct _vec3 {
    float x, y, z;
} Vec3;

Vec3 Vec3Minus(Vec3 a, Vec3 b);
Vec3 Vec3Plus(Vec3 a, Vec3 b);

void rotateVector(Vec3 *vec, float _x, float _y, float _z);

#endif //VECTOR_H
