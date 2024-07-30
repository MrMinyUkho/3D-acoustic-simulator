//
// Created by adima on 20.07.2024.
//

#ifndef DEFINES_H
#define DEFINES_H

//#define _CRT_SECURE_NO_WARNINGS

#define CL_VERSION_2_0                            1
#define CL_TARGET_OPENCL_VERSION                  200

#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001

#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_COLOR_BITS_ARB                        0x2014
#define WGL_DEPTH_BITS_ARB                        0x2022
#define WGL_STENCIL_BITS_ARB                      0x2023

#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_TYPE_RGBA_ARB                         0x202B

#define PI                  3.14159265
#define RAD2DEG(x)          ((x)/PI*180)
#define DEG2RAD(x)          ((x)*PI/180)

#define uint uint32_t
#define ull uint64_t
#define ll int64_t

#include <time.h>
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <corecrt_math.h>

#include "glad/glad.h"
#include "glad/glad_wgl.h"
#include "myLinearMath/matrix.h"
#include "myLinearMath/vector.h"
#include "opencl/kernel_opencl.h"
#include "shaderUtils/shaderLT.h"
#include "acousticModule/AccSim.h"
#include "modelsUtils/modelsUtils.h"
#include "sceneManager/sceneManager.h"

#endif //DEFINES_H
