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

void asInitCellVolume(const float w, const float h, const float l, const float density, const int drawStep) {

    if(drawStep < 1) {
        printf("Ну как бы если хочешь шаг отрисовки меньше 1, то пожалуйста, лично я не против\n");
        Sleep(2000);
    }

    // Create Volume

    cV.size_x = density * w;
    cV.size_y = density * h;
    cV.size_z = density * l;

    const ull vboSizeX = cV.size_x / drawStep;
    const ull vboSizeY = cV.size_y / drawStep;
    const ull vboSizeZ = cV.size_z / drawStep;

    printf("VBO size %lld %lld %lld\n", vboSizeX, vboSizeY, vboSizeZ);

    vboSize = vboSizeX * vboSizeY * vboSizeZ;

    Vec3* vboPoint = malloc(sizeof(Vec3)*vboSize);
    atrrArr = malloc(sizeof(float)*vboSize);

    for(int x = 0; x < vboSizeX; ++x) {
        for(int y = 0; y < vboSizeY; ++y) {
            for(int z = 0; z < vboSizeZ; ++z) {
                const ull draw_offset = getOffsetI(vboSizeX, vboSizeY, vboSizeZ, x, y, z);

                vboPoint[draw_offset].x = w / vboSizeX * x - w/2;
                vboPoint[draw_offset].y = h / vboSizeY * y - h/2;
                vboPoint[draw_offset].z = l / vboSizeZ * z - l/2;
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

    free(vboPoint);

    // Init OpenCL

    initClProgram(cV.size_x, cV.size_y, cV.size_z, drawStep, ampVBO);

    // Init thread

    InitializeCriticalSection(&critSecAccesToData);
    pauseEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

    ResetEvent(pauseEvent);
    computeThread = CreateThread(NULL, 0, asCompute, NULL, 0, NULL);
    SetEvent(pauseEvent);

    // Init OpenCL

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

        setAmpData(0.5f,0.5f,0.5f,sinf(t)*500);
        tickCompute();
        t+=0.1f;
        //Sleep(100);
    }

    return 0;
}

void asDestroyVolume() {
    running = 0;
    WaitForSingleObject(computeThread, INFINITE);
}

