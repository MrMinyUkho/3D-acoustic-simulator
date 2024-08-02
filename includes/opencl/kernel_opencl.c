//
// Created by adima on 25.07.2024.
//

#include "../defines.h"

#define TO_STR(x) #x
#define QUOTE(...) #__VA_ARGS__

int _err = 0;

#define  CHECK_ERROR(__err)                                               \
    if (__err != CL_SUCCESS) {                                            \
        fprintf(stderr, "\n%d:OpenCL error: %d\7\n", _err++, __err);      \
        Sleep(30000);                                                     \
        exit(__err);                                                      \
    }

int32_t  arg_drawStep;
int32_t  arg_size_x;
int32_t  arg_size_y;
int32_t  arg_size_z;
ull arg_size;
ull outSize;

const char* kernelMainSource = QUOTE(
    struct CellToCompute {
        float vel, amp, weight;
    };

    __kernel void compute_kernel(
        __global struct CellToCompute* data,
        __global float* dataOut,
        const int drawStep,
        const int size_x,
        const int size_y,
        const int size_z
    ) {
        int x = get_global_id(0);
        int y = get_global_id(1);
        int z = get_global_id(2);


        if (x < size_x && y < size_y && z < size_z) {

            long idx_xm = (x - 1) + z * size_x + y * (size_x * size_z);
            long idx_xp = (x + 1) + z * size_x + y * (size_x * size_z);

            long idx_ym = x + z * size_x + (y - 1) * (size_x * size_z);
            long idx_yp = x + z * size_x + (y + 1) * (size_x * size_z);

            long idx_zm = x + (z - 1) * size_x + y * (size_x * size_z);
            long idx_zp = x + (z + 1) * size_x + y * (size_x * size_z);

            long idx    = x + z * size_x + y * (size_x * size_z);

            //printf("%f %f\n", data[idx].amp, data[idx].vel);

            float force = 0.0f;
            if (x > 0)          force += data[idx_xm].amp;
            if (x < size_x - 1) force += data[idx_xp].amp;
            if (y > 0)          force += data[idx_ym].amp;
            if (y < size_y - 1) force += data[idx_yp].amp;
            if (z > 0)          force += data[idx_zm].amp;
            if (z < size_z - 1) force += data[idx_zp].amp;

            data[idx].vel += (force / 6.0f - data[idx].amp) * data[idx].weight;

            if( x%drawStep + y%drawStep + z%drawStep == 0) {
                int out_x = x / drawStep;
                int out_y = y / drawStep;
                int out_z = z / drawStep;
                long out_idx = out_x + out_z * (size_x / drawStep) + out_y * ((size_x / drawStep) * (size_z / drawStep));
                dataOut[out_idx] = data[idx].amp;
            }
        }
    }
);

const char* kernelAmplSource = QUOTE(
    struct CellToCompute {
        float vel, amp, weight;
    };

    __kernel void compute_kernel(
        __global struct CellToCompute* data,
        const long size
    ) {
        long idx = get_global_id(0);

        if (idx < size) {
            data[idx].amp += data[idx].vel;
        }
    }
);

const char* kernelInitSource = QUOTE(
    struct CellToCompute {
        float vel, amp, weight;
    };

    __kernel void compute_kernel(
        __global struct CellToCompute* data,
        const int size_x,
        const int size_y,
        const int size_z
    ) {
        long x = get_global_id(0);
        long y = get_global_id(1);
        long z = get_global_id(2);

        if (x < size_x && y < size_y && z < size_z) {
            long idx = x + size_x * (y + size_y * z);

            data[idx].amp = 0;
            data[idx].vel = 0;
            data[idx].weight = 1.0f;
        }
    }
);

const char* kernelWghtSource = QUOTE(
    struct CellToCompute {
        float vel, amp, weight;
    };

    typedef struct _vec3 {
        float x, y, z;
    } Vector3;

    int rayIntersectsTriangle(Vector3 rayOrigin, Vector3 rayDirection, Vector3 v0, Vector3 v1, Vector3 v2) {
        const float EPSILON = 1e-12;
        Vector3 edge1, edge2, h, s, q;
        float a, f, u, v;

        // Вычисление векторов сторон треугольника
        edge1.x = v1.x - v0.x;
        edge1.y = v1.y - v0.y;
        edge1.z = v1.z - v0.z;

        edge2.x = v2.x - v0.x;
        edge2.y = v2.y - v0.y;
        edge2.z = v2.z - v0.z;

        // Вычисление вектора нормали
        h.x = rayDirection.y * edge2.z - rayDirection.z * edge2.y;
        h.y = rayDirection.z * edge2.x - rayDirection.x * edge2.z;
        h.z = rayDirection.x * edge2.y - rayDirection.y * edge2.x;

        a = edge1.x * h.x + edge1.y * h.y + edge1.z * h.z;

        if (a > -EPSILON && a < EPSILON)
            return 0;

        f = 1.0 / a;
        s.x = rayOrigin.x - v0.x;
        s.y = rayOrigin.y - v0.y;
        s.z = rayOrigin.z - v0.z;

        u = f * (s.x * h.x + s.y * h.y + s.z * h.z);
        if (u < 0.0 || u > 1.0)
            return 0;

        q.x = s.y * edge1.z - s.z * edge1.y;
        q.y = s.z * edge1.x - s.x * edge1.z;
        q.z = s.x * edge1.y - s.y * edge1.x;

        v = f * (rayDirection.x * q.x + rayDirection.y * q.y + rayDirection.z * q.z);
        if (v < 0.0 || u + v > 1.0)
            return 0;

        float t = f * (edge2.x * q.x + edge2.y * q.y + edge2.z * q.z);
        if (t > EPSILON)
            return 1;
        else
            return 0;
    }

    __kernel void compute_kernel(
        __global struct CellToCompute* data,
        __global float* points,

        __global float* vertCoord,
        __global int*   Idx,
        __global float* mat,

        const int numFaces,

        const int size_x,
        const int size_y,
        const int size_z
    ) {
        long x = get_global_id(0);
        long y = get_global_id(1);
        long z = get_global_id(2);

        long idx = x + z * size_x + y * (size_x * size_z);

        if (x < size_x && y < size_y && z < size_z) {


            Vector3 rayDirection = {1.0, 0.0, 0.0}; // Луч вдоль оси X
            int intersections = 0;

            int currGroup = 0;

            for(int i = 0; i < numFaces; ++i) {

                Vector3 v0 = {vertCoord[9 * i + 0], vertCoord[9 * i + 1], vertCoord[9 * i + 2]};
                Vector3 v1 = {vertCoord[9 * i + 3], vertCoord[9 * i + 4], vertCoord[9 * i + 5]};
                Vector3 v2 = {vertCoord[9 * i + 6], vertCoord[9 * i + 7], vertCoord[9 * i + 8]};

                Vector3 point = {
                    points[idx * 3 + 0],
                    points[idx * 3 + 1],
                    points[idx * 3 + 2]
                };

                if (rayIntersectsTriangle(point, rayDirection, v0, v1, v2)) {
                    ++intersections;
                }

                if (Idx[i]+1 != currGroup) {
                    if((intersections % 2) == 1) {
                        data[idx].weight = mat[currGroup * 3 + 0]
                                         + mat[currGroup * 3 + 1] / 10.0f
                                         + mat[currGroup * 3 + 2] / 100.0f;
                        break;
                    }

                    intersections = 0;
                    currGroup = Idx[i];
                }
            }
        }
        //printf("x:%f y:%f z:%f w:%f\n", points[idx*3+0], points[idx*3+0], points[idx*3+0], data[idx].weight);
        data[idx].vel = 0.0f;
        data[idx].amp = 0.0f;
    }
);

cl_kernel kernelMain;
cl_kernel kernelWght;
cl_kernel kernelAmpl;
cl_kernel kernelInit;

cl_command_queue queue;
cl_int err;
cl_context context;
cl_program program;
cl_device_id device;

cl_mem data;

cl_mem vertexPos;
cl_mem faceIdx;
cl_mem materials;

cl_mem outData;
cl_mem pointsPos;

int readyToRead = 0;

void printDeviceInfo(cl_device_id device);

cl_kernel compileKernel(const char* kernelSource) {
    printf("\nCompile kernel... ");
    // Load kernel source code

    size_t kernelSourceSize = strlen(kernelSource);

    // Create program from kernel source
    program = clCreateProgramWithSource(context, 1, &kernelSource, &kernelSourceSize, &err);
    CHECK_ERROR(err);

    // Build program
    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        size_t logSize;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
        char* log = malloc(logSize);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, log, NULL);
        printf("Build log:\n%s\n", log);
        free(log);
        if(err != CL_SUCCESS) Sleep(INFINITE);
        CHECK_ERROR(err);
    }

    printf("OK\n");

    printf("Creating kernel... ");
    // Create kernel
    cl_kernel kernel = clCreateKernel(program, "compute_kernel", &err);
    CHECK_ERROR(err);
    printf("OK\n");

    return kernel;
}

void initClProgram(ull size_x, ull size_y, ull size_z, int drawStep, GLuint VBO1, GLuint pointsVBO, float* verticies, float* matrls, int16_t* matIdx, uint16_t faceNum, uint16_t matCount) {
    printf("\nInit CL program: %lld %lld %lld\n", size_x, size_y, size_z);
    cl_platform_id platform;
    cl_uint numPlatforms;

    // Get the number of platforms
    err = clGetPlatformIDs(0, NULL, &numPlatforms);
    _err = 0;
    CHECK_ERROR(err);

    printf("Number of platforms: %u\n", numPlatforms);

    if (numPlatforms == 0) {
        printf("No OpenCL platforms found.\n");
        exit(-1);
    }

    // Get platform and device information
    err = clGetPlatformIDs(1, &platform, NULL);
    CHECK_ERROR(err);
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    CHECK_ERROR(err);

    size_t extensionSize;
    err = clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, 0, NULL, &extensionSize);
    CHECK_ERROR(err);

    char *extensions = (char*)malloc(extensionSize);
    err = clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, extensionSize, extensions, NULL);
    CHECK_ERROR(err);

    printf("\n%s\n", extensions);

    if (strstr(extensions, "cl_khr_gl_sharing") == 0) {
        printf("OpenCL-OpenGL interop not supported.\n");
        exit(EXIT_FAILURE);
    }
    printf("OpenCL-OpenGL interop supported.\n");
    free(extensions);

    // Create OpenCL context
    printf("\nInit OpenCL context with GL share buffer support... ");

    cl_context_properties props[] = {
        CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
        CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
        0
    };

    context = clCreateContext(props, 1, &device, NULL, NULL, &err);
    CHECK_ERROR(err);
    printf("OK\n");

    // Create command queue
    printf("Init OpenCL queue... ");
    queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);
    CHECK_ERROR(err);
    printf("OK\n");

    ull size = size_x * size_y * size_z;
    outSize = (size_x / drawStep) * (size_y / drawStep) * (size_z / drawStep);

    printf("Allocate memory for volume... ");
    data      = clCreateBuffer(context, CL_MEM_READ_WRITE, size*sizeof(CellToCompute),       NULL, &err);
    CHECK_ERROR(err);
    vertexPos = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, faceNum*9*sizeof(float), verticies, &err);
    CHECK_ERROR(err);
    faceIdx   = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, faceNum*sizeof(int16_t),    matIdx, &err);
    CHECK_ERROR(err);
    materials = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, matCount*3*sizeof(float),   matrls, &err);
    CHECK_ERROR(err);
    printf("OK\n");


    printf("Bind buffers from VBOs... ");
    outData   = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, VBO1,      &err);
    CHECK_ERROR(err);
    pointsPos = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, pointsVBO, &err);
    CHECK_ERROR(err);
    printf("OK\n");

    arg_drawStep = drawStep;
    arg_size_x = size_x;
    arg_size_y = size_y;
    arg_size_z = size_z;
    arg_size = size;

    kernelWght = compileKernel(kernelWghtSource);
    CHECK_ERROR(err);
    printf("Set arguments for gen weight kernel... ");
    err = clSetKernelArg(kernelWght, 0, sizeof(cl_mem),  &data);
    CHECK_ERROR(err);
    err = clSetKernelArg(kernelWght, 1, sizeof(cl_mem),  &pointsPos);
    CHECK_ERROR(err);
    err = clSetKernelArg(kernelWght, 2, sizeof(cl_mem),  &vertexPos);
    CHECK_ERROR(err);
    err = clSetKernelArg(kernelWght, 3, sizeof(cl_mem),  &faceIdx);
    CHECK_ERROR(err);
    err = clSetKernelArg(kernelWght, 4, sizeof(cl_mem),  &materials);
    CHECK_ERROR(err);
    err = clSetKernelArg(kernelWght, 5, sizeof(int32_t),  &faceNum);
    CHECK_ERROR(err);
    err = clSetKernelArg(kernelWght, 6, sizeof(int32_t),  &arg_size_x);
    CHECK_ERROR(err);
    err = clSetKernelArg(kernelWght, 7, sizeof(int32_t),  &arg_size_y);
    CHECK_ERROR(err);
    err = clSetKernelArg(kernelWght, 8, sizeof(int32_t),  &arg_size_z);
    CHECK_ERROR(err);


    kernelMain = compileKernel(kernelMainSource);
    CHECK_ERROR(err);
    printf("Set arguments for Main kernel... ");
    err = clSetKernelArg(kernelMain, 0, sizeof(cl_mem),  &data);
    CHECK_ERROR(err);
    err = clSetKernelArg(kernelMain, 1, sizeof(cl_mem),  &outData);
    CHECK_ERROR(err);
    err = clSetKernelArg(kernelMain, 2, sizeof(int32_t), &arg_drawStep);
    CHECK_ERROR(err);
    err = clSetKernelArg(kernelMain, 3, sizeof(int32_t), &arg_size_x);
    CHECK_ERROR(err);
    err = clSetKernelArg(kernelMain, 4, sizeof(int32_t), &arg_size_y);
    CHECK_ERROR(err);
    err = clSetKernelArg(kernelMain, 5, sizeof(int32_t), &arg_size_z);
    CHECK_ERROR(err);
    printf("OK\n");

    kernelInit = compileKernel(kernelInitSource);
    CHECK_ERROR(err);
    printf("Set arguments for Init kernel... ");
    err = clSetKernelArg(kernelInit, 0, sizeof(cl_mem), &data);
    CHECK_ERROR(err);
    err = clSetKernelArg(kernelInit, 1, sizeof(int32_t), &arg_size_x);
    CHECK_ERROR(err);
    err = clSetKernelArg(kernelInit, 2, sizeof(int32_t), &arg_size_y);
    CHECK_ERROR(err);
    err = clSetKernelArg(kernelInit, 3, sizeof(int32_t), &arg_size_z);
    CHECK_ERROR(err);
    printf("OK\n");

    kernelAmpl = compileKernel(kernelAmplSource);
    CHECK_ERROR(err);
    printf("Set arguments for Apply velocity kernel... ");
    err = clSetKernelArg(kernelAmpl, 0, sizeof(cl_mem), &data);
    CHECK_ERROR(err);
    err = clSetKernelArg(kernelAmpl, 1, sizeof(ull),   &arg_size);
    CHECK_ERROR(err);
    printf("OK\n");

    // Init memory
    clFinish(queue);
    printf("\nFill memory zero... ");
    size_t globalWorkSize[3] = {size_x, size_y, size_z};
    size_t localWorkSize[3] = {4, 4, 4};
    err = clEnqueueNDRangeKernel(queue, kernelInit, 3, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
    CHECK_ERROR(err);
    clFinish(queue);
    printf("OK\n");
    printf("\nGenerate volume weight... ");
    err = clEnqueueNDRangeKernel(queue, kernelWght, 3, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
    CHECK_ERROR(err);
    printf("OK\n");
}

void tickCompute() {

    clFinish(queue);
    glFlush();
    glFinish();

    // err = clEnqueueAcquireGLObjects(queue, 1, &outData, 0, NULL, NULL);
    // CHECK_ERROR(err);

    size_t globalWorkSize3[3] = {arg_size_x, arg_size_y, arg_size_z};
    size_t localWorkSize3[3] = {4, 4, 4};

    err = clEnqueueNDRangeKernel(queue, kernelMain, 3, NULL, globalWorkSize3, localWorkSize3, 0, NULL, NULL);
    CHECK_ERROR(err);

    //clFinish(queue);

    // err = clEnqueueReleaseGLObjects(queue, 1, &outData1, 0, NULL, NULL);
    // CHECK_ERROR(err);

    size_t globalWorkSize = arg_size_x * arg_size_y * arg_size_z;
    size_t localWorkSize = 64;
    err = clEnqueueNDRangeKernel(queue, kernelAmpl, 1, NULL, &globalWorkSize, &localWorkSize, 0, NULL, NULL);
    CHECK_ERROR(err);

    //*curBuf = (*curBuf + 1) % 2;
}

void setAmpData(float x, float y, float z, float amp) {

    int pos_x = arg_size_x * x;
    int pos_y = arg_size_y * y;
    int pos_z = arg_size_z * z;

    ull index = getOffsetS(pos_x, pos_y, pos_z);
    CellToCompute cell = {0, amp, 1.0f};
    err = clEnqueueWriteBuffer(queue, data, CL_TRUE, sizeof(CellToCompute) * index, sizeof(CellToCompute), &cell, 0, NULL, NULL);  CHECK_ERROR(err);

}

void getClFreeMem(ull* total_mem, ull* free_mem) {
    CHECK_ERROR(clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(ull), total_mem, NULL));
    CHECK_ERROR(clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(ull), free_mem, NULL));
}

void destroyClProgram() {

    clReleaseKernel(kernelMain);
    clReleaseKernel(kernelInit);
    clReleaseKernel(kernelAmpl);

    clReleaseMemObject(data);
    clReleaseMemObject(outData);
    clReleaseProgram(program);

    clReleaseCommandQueue(queue);
    clReleaseContext(context);
}

void printDeviceInfo(cl_device_id device) {

    char buffer[1024];
    cl_uint buf_uint;
    cl_ulong buf_ulong;
    cl_int err;

    // Имя устройства
    err = clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(buffer), buffer, NULL);
    if (err == CL_SUCCESS) printf("Device Name: %s\n", buffer);

    // Vendor
    err = clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(buffer), buffer, NULL);
    if (err == CL_SUCCESS) printf("Vendor: %s\n", buffer);

    // Версия OpenCL
    err = clGetDeviceInfo(device, CL_DEVICE_VERSION, sizeof(buffer), buffer, NULL);
    if (err == CL_SUCCESS) printf("OpenCL Version: %s\n", buffer);

    // Максимальное число вычислительных юнитов
    err = clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(buf_uint), &buf_uint, NULL);
    if (err == CL_SUCCESS) printf("Max Compute Units: %u\n", buf_uint);

    // Максимальная рабочая группа
    err = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(buf_ulong), &buf_ulong, NULL);
    if (err == CL_SUCCESS) printf("Max Work Group Size: %llu\n", buf_ulong);

    // Глобальная память
    err = clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(buf_ulong), &buf_ulong, NULL);
    if (err == CL_SUCCESS) printf("Global Memory Size: %llu MB\n", buf_ulong / (1024 * 1024));

    // Локальная память
    err = clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(buf_ulong), &buf_ulong, NULL);
    if (err == CL_SUCCESS) printf("Local Memory Size: %llu KB\n", buf_ulong / 1024);

    // Размер кэша
    err = clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(buf_ulong), &buf_ulong, NULL);
    if (err == CL_SUCCESS) printf("Global Memory Cache Size: %llu KB\n", buf_ulong / 1024);

    // Частота процессора
    err = clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(buf_uint), &buf_uint, NULL);
    if (err == CL_SUCCESS) printf("Max Clock Frequency: %u MHz\n", buf_uint);

    // Версия драйвера
    err = clGetDeviceInfo(device, CL_DRIVER_VERSION, sizeof(buffer), buffer, NULL);
    if (err == CL_SUCCESS) printf("Driver Version: %s\n", buffer);
}

void printClDevives() {

    printf("\n################### Devices Info ###################\n");

    cl_int err;
    cl_uint numPlatforms;
    cl_platform_id* platforms;

    // Получаем количество платформ
    err = clGetPlatformIDs(0, NULL, &numPlatforms);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Unable to get platforms\n");
        exit(EXIT_FAILURE);
    }

    platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * numPlatforms);

    // Получаем платформы
    err = clGetPlatformIDs(numPlatforms, platforms, NULL);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Unable to get platform IDs\n");
        free(platforms);
        exit(EXIT_FAILURE);
    }

    // Перебираем платформы
    for (cl_uint i = 0; i < numPlatforms; i++) {
        cl_uint numDevices;
        cl_device_id* devices;

        // Получаем количество устройств
        err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices);
        if (err != CL_SUCCESS) {
            fprintf(stderr, "Unable to get device count\n");
            continue;
        }

        devices = (cl_device_id*)malloc(sizeof(cl_device_id) * numDevices);

        // Получаем устройства
        err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, numDevices, devices, NULL);
        if (err != CL_SUCCESS) {
            fprintf(stderr, "Unable to get device IDs\n");
            free(devices);
            continue;
        }

        // Перебираем устройства и выводим информацию
        for (cl_uint j = 0; j < numDevices; j++) {
            printf("Device %u:\n", j + 1);
            printDeviceInfo(devices[j]);
            printf("\n");
        }

        free(devices);
    }

    free(platforms);
    printf("\n####################################################\n");
}
