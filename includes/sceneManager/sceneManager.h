//
// Created by adima on 21.07.2024.
//

#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include "../defines.h"

typedef struct _lightSource {
    Vec3 position;
    Vec3 color;
    float strenght;
} LightSource;

typedef struct _camera {
    float fov;
    float aspect;
    float* proj;
    float* transform;
} Camera;

typedef struct _scene {
    Object** objects;
    uint objectCount;

    LightSource** lights;
    uint lightCount;

    Camera** cameras;
    uint cameraCount;
    uint activeCamera;

    float volPos[];

} Scene;


void smInit();
void smDestroy();

void smDrawScene();
void smNewScene();
uint smGetSceneCount();
void smSetActiveScene(uint i);

void smAddObject(Object* obj);
void smAddLight(LightSource* light);
void smAddCamera(Camera* cam);

void smSetActiveCamera(uint i);
float* smGetCamTransform();
#endif //SCENEMANAGER_H
