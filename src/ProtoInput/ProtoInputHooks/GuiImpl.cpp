//https://github.com/ocornut/imgui/issues/2022

#include "Gui.h"

#include <Windows.h>
#include <stdio.h>
#include "imgui.h"
#include <GL/glew.h>  
#include <gl/GL.h>
#include <tchar.h>
#include <GL/wglew.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_opengl3.h>
#include "RawInput.h"

HGLRC   g_GLRenderContext;
HDC     g_HDCDeviceContext;
HWND    protoHwnd;
int     g_display_w = 800;
int     g_display_h = 600;

void CreateGlContext();
bool SetSwapInterval(int interval);
bool WGLExtensionSupported(const char* extension_name);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

unsigned long Proto::GuiThreadID = 0;

int Proto::ShowGuiImpl()
{
    auto hInstance = GetModuleHandle(NULL);

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    const wchar_t* className = L"PROTOINPUTUI";
    wc.lpszClassName = className;
    wc.style = CS_OWNDC;
    if (!RegisterClass(&wc))
        return 1;
    protoHwnd = CreateWindowW(wc.lpszClassName, L"Proto Input", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 1200, 800, 0, 0, hInstance, 0);

    // Show the window
    ShowWindow(protoHwnd, SW_SHOWDEFAULT);
    UpdateWindow(protoHwnd);

    //Prepare OpenGlContext
    CreateGlContext();
    SetSwapInterval(1); //0 - No Interval, 1 - Sync with VSYNC, n - n times Sync with VSYNC
    glewInit();

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    //Init Win32
    ImGui_ImplWin32_Init(protoHwnd);

    //Init OpenGL Imgui Implementation
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);

    //Set Window bg color
    ImVec4 clear_color = ImVec4(0.1f, 0.0f, 0.0f, 1.0f);

    // Setup style
    ImGui::StyleColorsDark();

    wglMakeCurrent(g_HDCDeviceContext, g_GLRenderContext);
    	
    // Main loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        if (PeekMessage(&msg, protoHwnd, 0U, 0U, PM_REMOVE))
        // if (PeekMessage(&msg, 0, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        Proto::RenderImgui();

        ImGui::Render();


    	
        // wglMakeCurrent(g_HDCDeviceContext, g_GLRenderContext);


    	
        glViewport(0, 0, g_display_w, g_display_h);                 //Display Size got from Resize Command
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


    	
        // wglMakeCurrent(g_HDCDeviceContext, g_GLRenderContext);


    	
        SwapBuffers(g_HDCDeviceContext);

    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    wglDeleteContext(g_GLRenderContext);
    ImGui::DestroyContext();
    ImGui_ImplWin32_Shutdown();

    DestroyWindow(protoHwnd);
    UnregisterClassW(className, wc.hInstance);

    return 0;
}

//extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            g_display_w = (UINT)LOWORD(lParam);
            g_display_h = (UINT)HIWORD(lParam);
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void CreateGlContext()
{

    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
        PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
        32,                   // Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                   // Number of bits for the depthbuffer
        8,                    // Number of bits for the stencilbuffer
        0,                    // Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    g_HDCDeviceContext = GetDC(protoHwnd);

    int pixelFormal = ChoosePixelFormat(g_HDCDeviceContext, &pfd);
    SetPixelFormat(g_HDCDeviceContext, pixelFormal, &pfd);
    g_GLRenderContext = wglCreateContext(g_HDCDeviceContext);
    wglMakeCurrent(g_HDCDeviceContext, g_GLRenderContext);
}

bool SetSwapInterval(int interval)
{
    if (WGLExtensionSupported("WGL_EXT_swap_control"))
    {
        // Extension is supported, init pointers.
        wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

        // this is another function from WGL_EXT_swap_control extension
        wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");

        wglSwapIntervalEXT(interval);
        return true;
    }

    return false;

}

//Got from https://stackoverflow.com/questions/589064/how-to-enable-vertical-sync-in-opengl/589232
bool WGLExtensionSupported(const char* extension_name)
{
    // this is pointer to function which returns pointer to string with list of all wgl extensions
    PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = NULL;

    // determine pointer to wglGetExtensionsStringEXT function
    _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");

    if (strstr(_wglGetExtensionsStringEXT(), extension_name) == NULL)
    {
        // string was not found
        return false;
    }

    // extension is supported
    return true;
}