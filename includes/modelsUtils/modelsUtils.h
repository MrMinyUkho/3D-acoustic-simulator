//
// Created by adima on 19.07.2024.
//

#ifndef MODELSUTILS_H
#define MODELSUTILS_H

#include "../myLinearMath/vector.h"
#include "../glad/glad.h"

typedef struct fc {
    GLuint vertices[3];
    GLuint normals[3];
} Face;

typedef struct _mtl {
    char* matName;
    Vec3 diffuse;
} MtlMaterial;

typedef struct objct {
    char* name;
    GLuint vertCount;
    Vec3* vertices;
    GLuint normCount;
    Vec3* normals;
    GLuint faceCount;
    Face* faces;

    GLuint vboVert;
    GLuint vboNorm;
    GLuint vboFace;
    GLuint vao;
    GLuint ebo;

    MtlMaterial* color;

    float transform[16];

} Object;

void loadObject(const char* filename, Object* obj);
void loadMtl(const char* filename, MtlMaterial** materials, int* material_count);

void drawObject(const Object *obj, const float *view, const float *proj);

void deleteObject(Object* obj);

#endif //MODELSUTILS_H
