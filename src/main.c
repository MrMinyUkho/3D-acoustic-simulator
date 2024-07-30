#include "../includes/defines.h"

typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc, HGLRC hShareContext,
                                                     const int *attribList);

typedef BOOL WINAPI wglChoosePixelFormatARB_type(HDC hdc, const int *piAttribIList,
        const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);

typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC) (int interval);

wglCreateContextAttribsARB_type *wglCreateContextAttribsARB;
wglChoosePixelFormatARB_type *wglChoosePixelFormatARB;

float volumePos[16];
Object obj;
Object map;

float rotX;
float rotY;

clock_t timer_curr;
clock_t timer_prev;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
HGLRC EnableOpenGL(HDC* hDC);
void DisableOpenGL(HWND, HDC, HGLRC);
void InputProcess(HWND hwnd);

void Init() {

    printClDevives();

    smInit();

    loadObject(".\\untitled.obj", &obj);
    loadObject(".\\map.obj", &map);
    smAddObject(&obj);
    smAddObject(&map);

    scaleMatrix(map.transform, -1.0f, 1.0f, 1.0f);
    translateMatrix(map.transform, 0.0f, -100.0f, 0.0f);
    //translateMatrix(map.transform, 0.0f, -3.0f, 0.0f);

    applyIdentityMatrix(volumePos);
    scaleMatrix(volumePos, 2.0f, 2.0f, 2.0f);

    Camera* cam = malloc(sizeof(Camera));

    cam->aspect = (float)800/600;
    cam->fov = 90;

    cam->proj = malloc(sizeof(float)*16);
    cam->transform = malloc(sizeof(float)*16);

    applyIdentityMatrix(cam->transform);
    smAddCamera(cam);

    asInitCellVolume(10, 10, 10, 20, 2);

    timer_prev = clock();
}

void update(HWND hwnd) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.1f,0.1f,0.1f,1.0f);

    InputProcess(hwnd);

    rotateMatrix(obj.transform, 0.3f, 0.0f, 1.0f, 0.0f);
    translateMatrix(obj.transform, 0.05f, 0.0f, 0.0f);

    smDrawScene();


    //drawObject(&obj, cameraView, cameraProjection);
}

void InputProcess(HWND hwnd) {
    if(GetForegroundWindow() != hwnd) return;

    Vec3 dir = {0.0f, 0.0f, 0.0f};

    if(GetAsyncKeyState('W') < 0) dir.z += 0.1f;
    if(GetAsyncKeyState('S') < 0) dir.z -= 0.1f;
    if(GetAsyncKeyState('A') < 0) dir.x -= 0.1f;
    if(GetAsyncKeyState('D') < 0) dir.x += 0.1f;
    if(GetAsyncKeyState('R') < 0) dir.y += 0.1f;
    if(GetAsyncKeyState('F') < 0) dir.y -= 0.1f;

    POINT cur;
    static POINT base = {250, 250};
    GetCursorPos(&cur);

    rotX -= -(cur.x - base.x)*0.05f;
    rotY -= -(cur.y - base.y)*0.05f;

    float* camTransform = smGetCamTransform();

    static float eye[3] = {0.0f, 0.0f, 0.0f};

    applyIdentityMatrix(camTransform);

    Vec3 lookat = {0.0f, 0.0f, 1.0f};
    float up[3] = {0.0f, 1.0f, 0.0f};
    float lookatF[3];

    rotateVector(&lookat, rotY, 0.0f, 0.0f);
    rotateVector(&lookat, 0.0f, rotX, 0.0f);
    rotateVector(&dir,    rotY, 0.0f, 0.0f);
    rotateVector(&dir,    0.0f, rotX, 0.0f);

    eye[0] += dir.x;
    eye[1] += dir.y;
    eye[2] += dir.z;

    VEC2FLOAT(lookat, lookatF);
    lookatF[0] += eye[0];
    lookatF[1] += eye[1];
    lookatF[2] += eye[2];
    //printf("%f %f %f\n", lookatF[0], lookatF[1], lookatF[2]);

    getViewMatrix(camTransform, eye, lookatF, up);

    SetCursorPos(base.x, base.y);
}

static void fatal_error(const char *msg) {
    MessageBoxA(NULL, msg, "Error", MB_OK | MB_ICONEXCLAMATION);
    exit(-1);
}

void EnableVSYNC() {
    if (wglSwapIntervalEXT) {
        wglSwapIntervalEXT(1);  // Включение VSYNC
    } else {
        fprintf(stderr, "wglSwapIntervalEXT is not supported\n");
    }
}

static void init_opengl_extensions(void) {
    // Before we can load extensions, we need a dummy OpenGL context, created using a dummy window.
    // We use a dummy window because you can only set the pixel format for a window once. For the
    // real window, we want to use wglChoosePixelFormatARB (so we can potentially specify options
    // that aren't available in PIXELFORMATDESCRIPTOR), but we can't load and use that before we
    // have a context.
    WNDCLASSA window_class = {
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc = DefWindowProcA,
        .hInstance = GetModuleHandle(0),
        .lpszClassName = "Dummy_WGL_djuasiodwa",
    };

    if (!RegisterClassA(&window_class)) {
        fatal_error("Failed to register dummy OpenGL window.");
    }

    const HWND dummy_window = CreateWindowExA(
        0,
        window_class.lpszClassName,
        "Dummy OpenGL Window",
        0,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        window_class.hInstance,
        0);

    if (!(int)dummy_window) fatal_error("Failed to create dummy OpenGL window.");

    const HDC dummy_dc = GetDC(dummy_window);

    PIXELFORMATDESCRIPTOR pfd = {
        .nSize = sizeof(pfd),
        .nVersion = 1,
        .iPixelType = PFD_TYPE_RGBA,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .cColorBits = 32,
        .cAlphaBits = 8,
        .iLayerType = PFD_MAIN_PLANE,
        .cDepthBits = 24,
        .cStencilBits = 8,
    };

    int pixel_format = ChoosePixelFormat(dummy_dc, &pfd);
    if (!pixel_format) fatal_error("Failed to find a suitable pixel format.");
    if (!SetPixelFormat(dummy_dc, pixel_format, &pfd)) fatal_error("Failed to set the pixel format.");

    const HGLRC dummy_context = wglCreateContext(dummy_dc);
    if (!(int)dummy_context) fatal_error("Failed to create a dummy OpenGL rendering context.");

    if (!wglMakeCurrent(dummy_dc, dummy_context)) fatal_error("Failed to activate dummy OpenGL rendering context.");

    wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type*)wglGetProcAddress(
        "wglCreateContextAttribsARB");
    wglChoosePixelFormatARB = (wglChoosePixelFormatARB_type*)wglGetProcAddress(
        "wglChoosePixelFormatARB");

    wglMakeCurrent(dummy_dc, 0);
    wglDeleteContext(dummy_context);
    ReleaseDC(dummy_window, dummy_dc);
    DestroyWindow(dummy_window);
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    freopen("CONIN$", "r", stdin);

    printf("Hello World\n");
    WNDCLASSEX wcex;
    HDC hDC;
    MSG msg;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "3D acoustic simulator";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex)) {
        const DWORD error = GetLastError();
        char errorMessage[256];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, errorMessage, 256, NULL);
        MessageBox(NULL, errorMessage, "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    const HWND hwnd = CreateWindowEx(0,
                               "3D acoustic simulator",
                               "3D acoustic simulator",
                               WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               800,
                               600,
                               NULL,
                               NULL,
                               hInstance,
                               NULL
    );

    if (!(int)hwnd) {
        const DWORD error = GetLastError();
        char errorMessage[256];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, errorMessage, 256, NULL);
        MessageBox(NULL, errorMessage, "Error", MB_OK | MB_ICONERROR);
        return 2;
    }

    hDC = GetDC(hwnd);

    ShowWindow(hwnd, nCmdShow);

    HGLRC hRC = EnableOpenGL(&hDC);

    Init();

    while (TRUE) {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            switch (msg.message) {
                case WM_QUIT:
                    asDestroyVolume();
                    smDestroy();
                    DisableOpenGL(hwnd, hDC, hRC);
                    DestroyWindow(hwnd);
                    destroyClProgram();
                    return msg.wParam;

                default:
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                break;
            }
        }

        timer_curr = clock();

        update(hwnd);
        SwapBuffers(hDC);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN: {
            switch (wParam) {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                default:
                break;
            }
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

HGLRC EnableOpenGL(const HDC* hDC)
{
    init_opengl_extensions();

    // Now we can choose a pixel format the modern way, using wglChoosePixelFormatARB.
    const int pixel_format_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
        WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,         32,
        WGL_DEPTH_BITS_ARB,         24,
        WGL_STENCIL_BITS_ARB,       8,
        WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
        WGL_SAMPLES_ARB, 2,
        0
    };

    int pixel_format;
    UINT num_formats;
    wglChoosePixelFormatARB(*hDC, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);
    if (!num_formats) fatal_error("Failed to set the OpenGL 4.6 pixel format.");


    PIXELFORMATDESCRIPTOR pfd;
    DescribePixelFormat(*hDC, pixel_format, sizeof(pfd), &pfd);
    if (!SetPixelFormat(*hDC, pixel_format, &pfd)) fatal_error("Failed to set the OpenGL 4.6 pixel format.");

    /* create and enable the render context (RC) */

    const int attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };


    const HGLRC hRC = wglCreateContextAttribsARB(*hDC, 0, attribs);
    if(!(int)hRC) fatal_error("Failed to create OpenGL 4.6 context.");
    if(!wglMakeCurrent(*hDC, hRC)) fatal_error("Failed to activate OpenGL 4.6 rendering context.");

    if (!gladLoadGL()) {
        printf("Failed to initialize GLAD\n");
        Sleep(1000);
        exit(-3);
    }
    if (gladLoadWGL(*hDC)) {
        wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
        EnableVSYNC();
    } else {
        printf("Failed to load WGL extensions\n");
        Sleep(1000);
        exit(-4);
    }

    glEnable(GL_MULTISAMPLE);

    return hRC;
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC) {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}