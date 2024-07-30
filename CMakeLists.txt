cmake_minimum_required(VERSION 3.28)

message(STATUS "${CMAKE_GENERATOR}")
message(STATUS "${CMAKE_C_COMPILER}")
message(STATUS "${CMAKE_PREFIX_PATH}")

project(3D_acoustic_simulator C)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_PREFIX_PATH "C:/Users/adima/vcpkg/packages/opencl_x64-windows")

set(CMAKE_C_STANDARD 11)
#set(X64 ON)

find_package(OpenGL REQUIRED)
find_package(OpenCLHeaders CONFIG REQUIRED)
find_package(OpenCLICDLoader CONFIG REQUIRED)

include_directories(${OPENGL_INCLUDE_DIRS})

add_executable(3D_acoustic_simulator
        WIN32

        src/main.c

        includes/defines.h

        includes/acousticModule/AccSim.h
        includes/acousticModule/AccSim.c

        includes/opencl/kernel_opencl.c
        includes/opencl/kernel_opencl.h

        includes/glad/glad.h
        includes/glad/glad.c
        includes/glad/glad_wgl.h
        includes/glad/glad_wgl.c
        includes/glad/khrplatform.h

        includes/modelsUtils/modelsUtils.h
        includes/modelsUtils/modelsUtils.c

        includes/myLinearMath/matrix.h
        includes/myLinearMath/matrix.c

        includes/shaderUtils/shaderLT.c
        includes/shaderUtils/shaderLT.h
        includes/sceneManager/sceneManager.c
        includes/sceneManager/sceneManager.h
        includes/myLinearMath/vector.c
)

target_link_libraries(3D_acoustic_simulator ${OPENGL_LIBRARIES})
target_link_libraries(3D_acoustic_simulator OpenCL::Headers)
target_link_libraries(3D_acoustic_simulator OpenCL::OpenCL)

set_target_properties(3D_acoustic_simulator PROPERTIES
    WIN32_EXECUTABLE YES
    LINK_FLAGS "/SUBSYSTEM:WINDOWS"
)