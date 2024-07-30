//
// Created by adima on 20.07.2024.
//

#include "../defines.h"

Vec3 Vec3Minus(Vec3 a, Vec3 b) {
    Vec3 c = { a.x - b.x, a.y - b.y, a.z - b.z};
    return c;
}

Vec3 Vec3Plus(Vec3 a, Vec3 b) {
    Vec3 c = { a.x + b.x, a.y + b.y, a.z + b.z};
    return c;
}

void rotateVector(Vec3 *vec, float _x, float _y, float _z){

    _x = DEG2RAD(_x);
    _y = DEG2RAD(_y);
    _z = DEG2RAD(_z);


    float sina = sinf(_x);
    float cosa = cosf(_x);

    float sinb = sinf(_y);
    float cosb = cosf(_y);

    float sing = sinf(_z);
    float cosg = cosf(_z);

    float x = vec->x;
    float y = vec->y;
    float z = vec->z;

    vec->x = x * ( cosb*cosg) + y * (sina*sinb*cosg - cosa*sing) + z * (cosa*sinb*cosg + sina*sing);
    vec->y = x * ( cosa*sing) + y * (sina*sinb*sing + cosa*cosg) + z * (cosa*sinb*sing - sina*cosg);
    vec->z = x * (-sinb)      + y * (sina*cosb)                  + z * (cosa*cosb);
}
