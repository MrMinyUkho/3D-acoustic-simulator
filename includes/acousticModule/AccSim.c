//
// Created by adima on 07.07.2024.
//

#include "../defines.h"

float* atrrArr;
asCellsVolume cV;

GLuint vao;

GLuint ampVBO;

GLuint volumeVBO;
GLuint VolumeShader;

ull vboSize;

// Thread variables

CRITICAL_SECTION critSecAccesToData;
HANDLE pauseEvent;
HANDLE computeThread;
volatile int running = 1;

DWORD WINAPI asCompute();

// ---------------

ull getOffsetS(const int x, const int y, const int z) {
    return (cV.size_x * cV.size_z) * y + cV.size_x * z + x;
}

ull getOffsetI(const int size_x, const int size_y, const int size_z, const int x, const int y, const int z) {
    return (size_x * size_z) * y + size_x * z + x;
}

void asInitCellVolume(Object* VolumeBaseObjects, const int objCount, const float density, const int drawStep) {

    if(drawStep < 1) {
        printf("Ну как бы если хочешь шаг отрисовки меньше 1, то пожалуйста, лично я не против\n");
        Sleep(2000);
    }

    // Create Volume


    float xmin = FLT_MAX, xmax = -FLT_MAX;
    float ymin = FLT_MAX, ymax = -FLT_MAX;
    float zmin = FLT_MAX, zmax = -FLT_MAX;

    for(int i = 0; i < objCount; ++i) {
        for(int j = 0; j < (VolumeBaseObjects + i)->vertCount; ++j) {

            Vec3 vert = (VolumeBaseObjects + i)->vertices[j];

            xmin = xmin > vert.x ? vert.x : xmin;
            xmax = xmax < vert.x ? vert.x : xmax;

            ymin = ymin > vert.y ? vert.y : ymin;
            ymax = ymax < vert.y ? vert.y : ymax;

            zmin = zmin > vert.z ? vert.z : zmin;
            zmax = zmax < vert.z ? vert.z : zmax;
        }
    }

    float w = xmax - xmin;
    float h = ymax - ymin;
    float l = zmax - zmin;

    printf("\n\ncV lin size %f %f %f\n", w, h, l);

    cV.size_x = density * w;
    cV.size_y = density * h;
    cV.size_z = density * l;

    printf("\ncV size %d %d %d\n", cV.size_x, cV.size_y, cV.size_z);

    const ull vboSizeX = cV.size_x / drawStep;
    const ull vboSizeY = cV.size_y / drawStep;
    const ull vboSizeZ = cV.size_z / drawStep;

    printf("\nVBO size %lld %lld %lld\n\n", vboSizeX, vboSizeY, vboSizeZ);

    vboSize = vboSizeX * vboSizeY * vboSizeZ;

    Vec3* vboPoint = malloc(sizeof(Vec3)*vboSize);
    atrrArr = malloc(sizeof(float)*vboSize);

    for(int x = 0; x < vboSizeX; ++x) {
        for(int y = 0; y < vboSizeY; ++y) {
            for(int z = 0; z < vboSizeZ; ++z) {
                const ull draw_offset = getOffsetI(vboSizeX, vboSizeY, vboSizeZ, x, y, z);

                vboPoint[draw_offset].x = w / vboSizeX * x + xmin;
                vboPoint[draw_offset].y = h / vboSizeY * y + ymin;
                vboPoint[draw_offset].z = l / vboSizeZ * z + zmin;
            }
        }
    }

    // GPU section

    VolumeShader = ShaderCreateProgram("shaders/CellVolume.vert", "shaders/CellVolume.frag");

    glGenBuffers(1, &volumeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, volumeVBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(Vec3)*vboSize, vboPoint, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &ampVBO);
    glBindBuffer(GL_ARRAY_BUFFER, ampVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vboSize, atrrArr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Create and bind VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Bind VBO with vertex coordinates
    glBindBuffer(GL_ARRAY_BUFFER, volumeVBO);
    const GLuint posAttrib = glGetAttribLocation(VolumeShader, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Create and bind VBO for amplitudes
    glBindBuffer(GL_ARRAY_BUFFER, ampVBO);
    const GLuint ampAttrib = glGetAttribLocation(VolumeShader, "amplitude");
    glEnableVertexAttribArray(ampAttrib);
    glVertexAttribPointer(ampAttrib, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);


    // Init OpenCL

    uint16_t faceCount = 0;
    for(int i = 0; i < objCount; ++i) {
        faceCount += (VolumeBaseObjects+i)->faceCount;
    }

    float* verts     = malloc(faceCount*9*sizeof(float));
    float* materials = malloc(objCount*3*sizeof(float));
    int16_t* matIdx  = malloc(faceCount*sizeof(int16_t));

    int _i = 0;
    for(int i = 0; i < objCount; ++i) {

        materials[i*3 + 0] = (VolumeBaseObjects+i)->color->diffuse.x;
        materials[i*3 + 1] = (VolumeBaseObjects+i)->color->diffuse.y;
        materials[i*3 + 2] = (VolumeBaseObjects+i)->color->diffuse.z;

        for(int j = 0; j < (VolumeBaseObjects+i)->faceCount; ++j) {
            Vec3* vert   = (VolumeBaseObjects+i)->vertices;
            GLuint* face = (VolumeBaseObjects+i)->faces->vertices;

            verts[_i * 9 + 0] = vert[face[0]].x;
            verts[_i * 9 + 1] = vert[face[0]].y;
            verts[_i * 9 + 2] = vert[face[0]].z;

            verts[_i * 9 + 3] = vert[face[1]].x;
            verts[_i * 9 + 4] = vert[face[1]].y;
            verts[_i * 9 + 5] = vert[face[1]].z;

            verts[_i * 9 + 6] = vert[face[2]].x;
            verts[_i * 9 + 7] = vert[face[2]].y;
            verts[_i * 9 + 8] = vert[face[2]].z;

            matIdx[_i] = i;
            _i++;
        }
    }

    initClProgram(cV.size_x, cV.size_y, cV.size_z, drawStep, ampVBO, volumeVBO, verts, materials, matIdx, faceCount, objCount);

    // Init thread

    InitializeCriticalSection(&critSecAccesToData);
    pauseEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

    ResetEvent(pauseEvent);
    computeThread = CreateThread(NULL, 0, asCompute, NULL, 0, NULL);
    SetEvent(pauseEvent);

    free(vboPoint);
}

void asDrawVolume(const float* model, const float* view, const float* projection) {

    glUseProgram(VolumeShader);

    const GLuint projectionLoc = glGetUniformLocation(VolumeShader, "projectionMatrix");
    const GLuint viewLoc = glGetUniformLocation(VolumeShader, "viewMatrix");
    const GLuint modelLoc = glGetUniformLocation(VolumeShader, "modelMatrix");

    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_BLEND);

    // Bind VAO
    glBindVertexArray(vao);

    // Draw
    glPointSize(3.0);
    glDrawArrays(GL_POINTS, 0, vboSize);

    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glUseProgram(0);
}


ull clamp(const ull val, const ull min, const ull max) {
    if(val > max) return max;
    if(val < min) return min;
    return val;
}

DWORD WINAPI asCompute() {
    float t = 0;

    while(running) {

        WaitForSingleObject(pauseEvent, INFINITE);

        setAmpData(0.5f,0.5f,0.5f,sinf(t)*250);
        tickCompute();
        t+=0.2f;
        Sleep(100);
    }

    return 0;
}

void asDestroyVolume() {
    running = 0;
    WaitForSingleObject(computeThread, INFINITE);
}

