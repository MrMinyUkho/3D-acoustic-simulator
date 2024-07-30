//
// Created by adima on 21.07.2024.
//

#include "sceneManager.h"

#define SCENE scenes[activeScene]
#define CAMERA scenes[activeScene].cameras[scenes[activeScene].activeCamera]

Scene* scenes;
uint sceneCount;
uint activeScene;

void smInit() {
    scenes = malloc(sizeof(Scene));
    if (!(int)scenes) {
        // Handle malloc failure
        exit(1);
    }
    activeScene = 0;
    sceneCount = 1;
    scenes[0].objectCount = 0;
    scenes[0].objects = NULL;
    scenes[0].cameraCount = 0;
    scenes[0].cameras = NULL;
    scenes[0].lightCount = 0;
    scenes[0].lights = NULL;
    scenes[0].activeCamera = 0;
    applyIdentityMatrix(scenes[0].volPos);
}

void smDestroy() {
    for(int i = 0; i < sceneCount; ++i) {
        for(int j = 0; j < scenes[i].cameraCount; ++j) {
            free(scenes[i].cameras[j]);
            scenes[i].cameras[j] = NULL;
        }
        for(int j = 0; j < scenes[i].lightCount; ++j) {
            free(scenes[i].lights[j]);
            scenes[i].lights[j] = NULL;
        }
        for(int j = 0; j < scenes[i].objectCount; ++j) {
            deleteObject(scenes[i].objects[j]);
        }
    }
}

void smDrawScene() {
    if(SCENE.cameraCount == 0) return;


    for(int i = 0; i < SCENE.objectCount; ++i) {
        drawObject(SCENE.objects[i], CAMERA->transform, CAMERA->proj);
    }

    asDrawVolume(SCENE.volPos, CAMERA->transform, CAMERA->proj);
}

void smAddObject(Object *obj) {
    SCENE.objectCount++;
    SCENE.objects = realloc(SCENE.objects, sizeof(Object) * SCENE.objectCount);
    SCENE.objects[SCENE.objectCount-1] = obj;
}

void smAddCamera(Camera *cam) {

    getPerspectiveMatrix(cam->proj, cam->fov, cam->aspect, 0.3, 100.0f);

    SCENE.cameraCount++;
    SCENE.cameras = realloc(SCENE.cameras, sizeof(Camera) * SCENE.cameraCount);
    SCENE.cameras[SCENE.cameraCount-1] = cam;
}

void smSetActiveCamera(unsigned int i) {
    if(i > SCENE.cameraCount >= i) return;
    SCENE.activeCamera = i;
}

float* smGetCamTransform() {
    return CAMERA->transform;
}



