//
// Created by adima on 19.07.2024.
//

#include "../defines.h"

GLuint basicShader = -1;

void setupObjectBuffers(Object *obj) {
    // Создание VAO
    glGenVertexArrays(1, &obj->vao);
    glBindVertexArray(obj->vao);

    float* vertex = malloc(obj->faceCount * 9 * sizeof(float));
    float* normals = malloc(obj->faceCount * 9 * sizeof(float));
    for (int i = 0; i < obj->faceCount; ++i) {
        vertex[i * 9 + 0] = obj->vertices[obj->faces[i].vertices[0]].x;
        vertex[i * 9 + 1] = obj->vertices[obj->faces[i].vertices[0]].y;
        vertex[i * 9 + 2] = obj->vertices[obj->faces[i].vertices[0]].z;

        vertex[i * 9 + 3] = obj->vertices[obj->faces[i].vertices[1]].x;
        vertex[i * 9 + 4] = obj->vertices[obj->faces[i].vertices[1]].y;
        vertex[i * 9 + 5] = obj->vertices[obj->faces[i].vertices[1]].z;

        vertex[i * 9 + 6] = obj->vertices[obj->faces[i].vertices[2]].x;
        vertex[i * 9 + 7] = obj->vertices[obj->faces[i].vertices[2]].y;
        vertex[i * 9 + 8] = obj->vertices[obj->faces[i].vertices[2]].z;

        normals[i * 9 + 0] = obj->normals[obj->faces[i].normals[0]].x;
        normals[i * 9 + 1] = obj->normals[obj->faces[i].normals[0]].y;
        normals[i * 9 + 2] = obj->normals[obj->faces[i].normals[0]].z;

        normals[i * 9 + 3] = obj->normals[obj->faces[i].normals[1]].x;
        normals[i * 9 + 4] = obj->normals[obj->faces[i].normals[1]].y;
        normals[i * 9 + 5] = obj->normals[obj->faces[i].normals[1]].z;

        normals[i * 9 + 6] = obj->normals[obj->faces[i].normals[2]].x;
        normals[i * 9 + 7] = obj->normals[obj->faces[i].normals[2]].y;
        normals[i * 9 + 8] = obj->normals[obj->faces[i].normals[2]].z;
    }

    // Создание VBO для вершин
    glGenBuffers(1, &obj->vboVert);
    glBindBuffer(GL_ARRAY_BUFFER, obj->vboVert);
    glBufferData(GL_ARRAY_BUFFER, obj->faceCount * 9 * sizeof(float), vertex, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Создание VBO для нормалей
    glGenBuffers(1, &obj->vboNorm);
    glBindBuffer(GL_ARRAY_BUFFER, obj->vboNorm);
    glBufferData(GL_ARRAY_BUFFER, obj->faceCount * 9 * sizeof(float), normals, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); // Сброс связки VAO
}

void loadObject(const char *filename, Object* obj) {
    FILE *file;
    printf("Open object %s file... ", filename);
    errno_t err = fopen_s(&file, filename, "rb");
    if(err != 0) printf("\nError(%d) with open %s model file\n", err, filename);
    else printf("OK\nSucces open %s model file\n", filename);
    printf("Load object data... ");

    obj->name = filename;
    obj->color[0] = 0.2;
    obj->color[1] = 0.9;
    obj->color[2] = 0.4;

    char line[200];
    Vec3 *vertices = malloc(sizeof(Vec3) * 256);
    Vec3 *normals  = malloc(sizeof(Vec3) * 256);
    Face *faces    = malloc(sizeof(Face) * 256);
    unsigned long long vertCount = 0, normCount = 0, faceCount = 0;
    unsigned long long vertAlloc = 256, normAlloc = 256, faceAlloc = 256;

    if (!vertices || !normals || !faces) {
        perror("\nFailed to allocate initial memory");
        free(vertices);
        free(normals);
        free(faces);
        fclose(file);
        return;
    }

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || isspace(line[0])) {
            continue;
        }

        if (strncmp(line, "v ", 2) == 0) {
            if (vertCount >= vertAlloc) {
                vertAlloc *= 2;
                Vec3* temp = realloc(vertices, vertAlloc * sizeof(Vec3));
                if (!temp) {
                    printf("\nFailed to reallocate memory for vertices");
                    free(vertices);
                    free(normals);
                    free(faces);
                    fclose(file);
                    return;
                }
                vertices = temp;
            }
            sscanf_s(line, "v %f %f %f", &vertices[vertCount].x, &vertices[vertCount].y, &vertices[vertCount].z);
            vertCount++;
        } else if (strncmp(line, "vn ", 3) == 0) {
            if (normCount >= normAlloc) {
                normAlloc *= 2;
                Vec3* temp = realloc(normals, normAlloc * sizeof(Vec3));
                if (!temp) {
                    printf("\nFailed to reallocate memory for normals");
                    free(vertices);
                    free(normals);
                    free(faces);
                    fclose(file);
                    return;
                }
                normals = temp;
            }
            sscanf_s(line, "vn %f %f %f", &normals[normCount].x, &normals[normCount].y, &normals[normCount].z);
            normCount++;
        } else if (strncmp(line, "f ", 2) == 0) {
            if (faceCount >= faceAlloc) {
                faceAlloc *= 2;
                Face* temp = realloc(faces, faceAlloc * sizeof(Face));
                if (!temp) {
                    printf("\nFailed to reallocate memory for faces");
                    free(vertices);
                    free(normals);
                    free(faces);
                    fclose(file);
                    return;
                }
                faces = temp;
            }
            Face face;
            int result = sscanf_s(line, "f %u//%u %u//%u %u//%u",
                                  &face.vertices[0], &face.normals[0],
                                  &face.vertices[1], &face.normals[1],
                                  &face.vertices[2], &face.normals[2]);
            if (result == 6) {
                face.vertices[0]--;
                face.normals[0]--;
                face.vertices[1]--;
                face.normals[1]--;
                face.vertices[2]--;
                face.normals[2]--;
            } else {
                result = sscanf_s(line, "f %u %u %u",
                                  &face.vertices[0], &face.vertices[1], &face.vertices[2]);
                if (result == 3) {
                    face.vertices[0]--;
                    face.vertices[1]--;
                    face.vertices[2]--;
                    face.normals[0] = face.normals[1] = face.normals[2] = 0; // Default normals if not provided
                } else {
                    printf("\nError parsing face line: %s", line);
                    continue;
                }
            }
            faces[faceCount] = face;
            faceCount++;
        }
    }

    fclose(file);

    obj->vertices = vertices;
    obj->vertCount = vertCount;
    obj->normals = normals;
    obj->normCount = normCount;
    obj->faces = faces;
    obj->faceCount = faceCount;

    applyIdentityMatrix(obj->transform);
    setupObjectBuffers(obj);
    printf("OK\n");
}

void loadMtl(const char* filename, MtlMaterial** materials, int* material_count) {
    FILE *file;
    printf("Open material %s file... ", filename);
    errno_t err = fopen_s(&file, filename, "rb");
    if(err != 0) printf("\nError(%d) with open %s material file\n", err, filename);
    else printf("OK\nSucces open %s material file\n", filename);
    printf("Load material data... ");

    char line[200];

    *material_count = 0;
    *materials = (MtlMaterial*)malloc(0);

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || isspace(line[0])) {
            continue;
        }

        if (strncmp(line, "newmtl ", 7) == 0) {
            (*material_count)++;
            *materials = (MtlMaterial*)realloc(*materials, sizeof(MtlMaterial)*(*material_count));

            int len = 0;
            for(int i = 7; i < 200; ++i) {
                len++;
                if(line[i] == '\n') break;
            }
            char* name = malloc(len);
            name[len-1] = '\0';
            for(int i = 7; i < 7+len-1; ++i) name[i-7] = line[i];

            (*materials)[*material_count-1].matName = name;
        } else if (strncmp(line, "Kd ", 3) == 0) {
            sscanf_s(line, "Kd %f %f %f",
                &(*materials)[*material_count-1].diffuse.x,
                &(*materials)[*material_count-1].diffuse.y,
                &(*materials)[*material_count-1].diffuse.z
            );
        }
    }
}

void loadObjects(const char* filename, Object** obj, int* object_count) {
    FILE *file;
    printf("Open wavefront %s file... ", filename);
    errno_t err = fopen_s(&file, filename, "rb");
    if(err != 0) printf("\nError(%d) with open %s wavefront file\n", err, filename);
    else printf("OK\nSucces open %s wavefront file\n", filename);
    printf("Load wavefront data... ");

    char line[200];
    *object_count = 0;
    *obj = (Object*)malloc(0);

    MtlMaterial* materials = malloc(0);
    int materialCount = 0;

    int alloc_face = 0, alloc_vert = 0, alloc_norm = 0;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || isspace(line[0])) {
            continue;
        }

        if (strncmp(line, "o ", 2) == 0) {
            if(*object_count != 0) setupObjectBuffers(obj[*object_count-1]);
            (*object_count)++;
            *obj = (Object*)realloc(*obj, sizeof(Object)*(*object_count));

            int len = 0;
            for(int i = 2; i < 200; ++i) {
                len++;
                if(line[i] == '\n') break;
            }
            char* name = malloc(len);
            name[len-1] = '\0';
            for(int i = 2; i < 2+len-1; ++i) name[i-7] = line[i];

            (*obj)[*object_count-1].name = name;

            (*obj)[*object_count-1].faceCount = 1;
            (*obj)[*object_count-1].vertCount = 1;
            (*obj)[*object_count-1].normCount = 1;

            (*obj)[*object_count-1].faces    = (Face*)malloc(sizeof(Face));
            (*obj)[*object_count-1].vertices = (Vec3*)malloc(sizeof(Vec3));
            (*obj)[*object_count-1].normals  = (Vec3*)malloc(sizeof(Vec3));

            alloc_face = 1;
            alloc_vert = 1;
            alloc_norm = 1;

        } else if (strncmp(line, "v ", 2) == 0) {
            sscanf_s(line, "v %f %f %f",
                &(*obj)[*object_count-1].vertices[(*obj)[*object_count-1].vertCount-1].x,
                &(*obj)[*object_count-1].vertices[(*obj)[*object_count-1].vertCount-1].y,
                &(*obj)[*object_count-1].vertices[(*obj)[*object_count-1].vertCount-1].z
            );
            if(alloc_vert == (*obj)[*object_count-1].vertCount) {
                alloc_vert*=2;
                (*obj)[*object_count-1].vertices =
                    (Vec3*)realloc((*obj)[*object_count-1].vertices, sizeof(Vec3)*alloc_vert);
            }
            (*obj)[*object_count-1].vertCount++;
        } else if (strncmp(line, "vn ", 3) == 0) {
            sscanf_s(line, "vn %f %f %f",
                &(*obj)[*object_count-1].normals[(*obj)[*object_count-1].normCount-1].x,
                &(*obj)[*object_count-1].normals[(*obj)[*object_count-1].normCount-1].y,
                &(*obj)[*object_count-1].normals[(*obj)[*object_count-1].normCount-1].z
            );

            if(alloc_norm == (*obj)[*object_count-1].normCount) {
                alloc_norm*=2;
                (*obj)[*object_count-1].normals =
                    (Vec3*)realloc((*obj)[*object_count-1].normals, sizeof(Vec3)*alloc_norm);
            }

            (*obj)[*object_count-1].normCount++;

        } else if (strncmp(line, "f ", 3) == 0) {
            Face* face = &(*obj)[*object_count-1].faces[(*obj)[*object_count-1].faceCount-1];
            sscanf_s(line, "f %u//%u %u//%u %u//%u",
                                  &face->vertices[0], &face->normals[0],
                                  &face->vertices[1], &face->normals[1],
                                  &face->vertices[2], &face->normals[2]);
            face->vertices[0]--;
            face->normals[0]--;
            face->vertices[1]--;
            face->normals[1]--;
            face->vertices[2]--;
            face->normals[2]--;

            if(alloc_face == (*obj)[*object_count-1].faceCount) {
                alloc_face*=2;
                (*obj)[*object_count-1].faces =
                    (Face*)realloc((*obj)[*object_count-1].faces, sizeof(Vec3)*alloc_face);
            }

            (*obj)[*object_count-1].faceCount++;
        } else if(strncmp(line, "usemtl ", 7) == 0) {

            int len = 0;
            for(int i = 7; i < 200; ++i) {
                len++;
                if(line[i] == '\n') break;
            }
            char* name = malloc(len);
            name[len-1] = '\0';
            for(int i = 2; i < 2+len-1; ++i) name[i-7] = line[i];


        }
    }
}

void drawObject(const Object *obj, const float *view, const float *proj) {
    if(basicShader == -1) basicShader = ShaderCreateProgram(".\\shaders\\Basic.vert", ".\\shaders\\Basic.frag");
    // Активировать VAO
    glBindVertexArray(obj->vao);

    // Установка режима смешивания и буфера глубины
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Установка шейдеров и их параметров
    // Активировать шейдерную программу
    glUseProgram(basicShader);

    // Установка юниформов
    GLint transformLoc = glGetUniformLocation(basicShader, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, obj->transform);

    GLint viewLoc = glGetUniformLocation(basicShader, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view);

    GLint projLoc = glGetUniformLocation(basicShader, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, proj);

    GLint colorLoc = glGetUniformLocation(basicShader, "color");
    glUniform3fv(colorLoc, 1, obj->color);

    // Отрисовка
    glDrawArrays(GL_TRIANGLES, 0, obj->faceCount * 3);

    // Отключение текущих буферов и шейдеров
    glUseProgram(0);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
}

void deleteObject(Object *obj) {
    free(obj->faces);
    free(obj->normals);
    free(obj->vertices);
}
