// Dear ImGui: standalone example application for DirectX 11
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma warning (disable : 4005, 4244) 
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "byte_array.h"

#include <d3d11.h>
#include <tchar.h>
#include <d3dx9tex.h>
#include <d3dx11.h>
#include "custom.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "globals.h"

// Data
static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;

//fonts
ImFont* mainfont;
ImFont* smallfont;
ImFont* iconfont;
ImFont* logofont;

// menu tabs

enum tabs {
    rageaim,
    legitaim,
    antiaim,
    view,
    world,
    players,
    scripts,
    configs,
    settings
};

enum subtabs {
    globals, 
    external,
    internal,
    settings2
};

tabs tabmenu = rageaim;
subtabs subtabmenu = globals;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


IDirect3DTexture9* user = nullptr;
static bool particles = true;

void Particles()
{
    if (!particles)
        return;

    ImVec2 screen_size = { (float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN) };

    static ImVec2 partile_pos[100];
    static ImVec2 partile_target_pos[100];
    static float partile_speed[100];
    static float partile_radius[100];


    for (int i = 1; i < 50; i++)
    {
        if (partile_pos[i].x == 0 || partile_pos[i].y == 0)
        {
            partile_pos[i].x = rand() % (int)screen_size.x + 1;
            partile_pos[i].y = 15.f;
            partile_speed[i] = 1 + rand() % 25;
            partile_radius[i] = rand() % 4;

            partile_target_pos[i].x = rand() % (int)screen_size.x;
            partile_target_pos[i].y = screen_size.y * 2;
        }

        partile_pos[i] = ImLerp(partile_pos[i], partile_target_pos[i], ImGui::GetIO().DeltaTime * (partile_speed[i] / 60));

        if (partile_pos[i].y > screen_size.y)
        {
            partile_pos[i].x = 0;
            partile_pos[i].y = 0;
        }

        ImGui::GetWindowDrawList()->AddCircleFilled(partile_pos[i], partile_radius[i], ImColor(174, 139, 148, 255 / 2));
    }

}


void RenderUI() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 15.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::SetNextWindowSize(ImVec2(650, 450));
    ImGui::Begin("MainWindow", nullptr, ImGuiWindowFlags_NoDecoration ); {

        auto draw = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetWindowPos();
        ImRect win(ImVec4(pos.x, pos.y, pos.x + 650, pos.y + 450));

        // main frame
        draw->AddRectFilled(win.Min, win.Max, ImColor(20, 20, 20), 18.0f, NULL);
        draw->AddRectFilled(pos, pos + ImVec2(150, 450), ImColor(34, 34, 34), 15.0f, ImDrawFlags_RoundCornersLeft);
        Particles();
        draw->AddRect(win.Min, win.Max, ImColor(65, 65, 65), 15.0f, NULL, 1.4f);
        draw->AddLine(pos + ImVec2(150, 0), pos + ImVec2(150, 450), ImColor(65, 65, 65), 1.0f);

        draw->AddRectFilled(win.Min + ImVec2(160, 10), win.Min + ImVec2(520, 50), ImColor(30, 30, 30), 10.0f);
        draw->AddRect(win.Min + ImVec2(160, 10), win.Min + ImVec2(520, 50), ImColor(65, 65, 65), 10.0f);
        ImGui::SetCursorPos(ImVec2(0, 50));

        draw->AddRectFilled(win.Min + ImVec2(530, 10), win.Min + ImVec2(640, 50), ImColor(30, 30, 30), 10.0f);
        draw->AddRect(win.Min + ImVec2(530, 10), win.Min + ImVec2(640, 50), ImColor(65, 65, 65), 10.0f);

        draw->AddCircle(win.Min + ImVec2(550, 30), 16.0f, ImColor(65, 65, 65), 100);
        draw->AddCircleFilled(win.Min + ImVec2(550, 30), 16.0f, ImColor(15,15,15), 100);

        draw->AddText(win.Min + ImVec2(570, 15), ImColor(155, 155, 155), "flux1337");
        ImGui::PushFont(smallfont);
        draw->AddText(win.Min + ImVec2(570, 30), ImColor(105, 105, 105), "To: Lifetime");
        ImGui::PopFont();

        ImGui::PushFont(logofont);
        draw->AddText(win.Min + ImVec2(20, 10), ImColor(250, 114, 185), "ECLIPSE");
        ImGui::PopFont();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));
        ImGui::BeginChild("tabmenu",ImVec2(150, 400));
        {   
            custom::tabchild("Aimbot", ImVec2(150, 125)); {
                if (custom::tab("Rage Aim", ImVec2(135, 30), NULL, (char*)u8"\ue909", tabmenu == rageaim)) { tabmenu = rageaim; }
                if (custom::tab("Legit Aim", ImVec2(135, 30), NULL, (char*)u8"\ue908", tabmenu == legitaim)) { tabmenu = legitaim; }
                if (custom::tab("Anti Aim", ImVec2(135, 30), NULL, (char*)u8"\ue907", tabmenu == antiaim)) { tabmenu = antiaim; }
            } ImGui::EndChild();
            custom::tabchild("Globals", ImVec2(150, 125)); {
                if (custom::tab("View", ImVec2(135, 30), NULL, (char*)u8"\ue906", tabmenu == view)) { tabmenu = view; }
                if (custom::tab("World", ImVec2(135, 30), NULL, (char*)u8"\ue905", tabmenu == world)) { tabmenu = world; }
                if (custom::tab("Players", ImVec2(135, 30), NULL, (char*)u8"\ue904", tabmenu == players)) { tabmenu = players; }
            } ImGui::EndChild();
            custom::tabchild("Miscellaneous", ImVec2(150, 125)); {
                if (custom::tab("Sctipts", ImVec2(135, 30), NULL, (char*)u8"\ue902", tabmenu == scripts)) { tabmenu = scripts; }
                if (custom::tab("Configs", ImVec2(135, 30), NULL, (char*)u8"\ue901", tabmenu == configs)) { tabmenu = configs; }
                if (custom::tab("Settings", ImVec2(135, 30), NULL, (char*)u8"\ue900", tabmenu == settings)) { tabmenu = settings; }
            } ImGui::EndChild();

        } ImGui::EndChild();
        ImGui::PopStyleVar(2);

        ImGui::SetCursorPos(ImVec2(160, 10));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 5));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10,50));
        ImGui::BeginChild("subtabmenu", ImVec2(420, 40)); {

            if (custom::subtab("GLOBALS", ImVec2(75, 30), NULL, subtabmenu == globals)) { subtabmenu = globals; }
            if (custom::subtab("INTERNAL", ImVec2(75, 30), NULL, subtabmenu == internal)) { subtabmenu = internal; }
            if (custom::subtab("EXTERNAL", ImVec2(75, 30), NULL, subtabmenu == external)) { subtabmenu = external; }
            if (custom::subtab("SETTINGS", ImVec2(75, 30), NULL, subtabmenu == settings2)) { subtabmenu = settings2; }

        } ImGui::EndChild();
        ImGui::PopStyleVar(2);

        ImGui::SetCursorPos(ImVec2(160, 60));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15, 15));
        ImGui::BeginChild("Main", ImVec2(480, 387)); {

            switch (tabmenu) {

            case rageaim: {

                    switch (subtabmenu)
                        case globals: {

                        custom::MenuChild("Rage AIM", ImVec2(220, 300)); {

                            ImGui::Checkbox("Checkbox", &checkbox);
                            ImGui::SliderInt("Slider", &slider, 0, 360);
                            ImGui::Combo("Combo", &item, items, IM_ARRAYSIZE(items)); \
                                ImGui::MultiCombo("MultiCombo", i, items, IM_ARRAYSIZE(items));
                            ImGui::InputText("InputText", text, sizeof(text));
                            ImGui::Keybind("Aim keybind", &key, true);
                            ImGui::ColorEdit4("ColorEdit", color, picker_flags);



                        } ImGui::EndChild();
                        custom::MenuChild("Legit AIM", ImVec2(220, 200)); {



                        } ImGui::EndChild();
                        ImGui::SetCursorPos(ImVec2(240, 5));
                        custom::MenuChild("Anti AIM", ImVec2(220, 200)); {



                        } ImGui::EndChild();
                        ImGui::SetCursorPos(ImVec2(240, 220));
                        custom::MenuChild("Damager", ImVec2(220, 300)); {



                        } ImGui::EndChild();

                        }

                }

            }
            


        } ImGui::EndChild();
        ImGui::PopStyleVar(2);

        ImGui::PopStyleVar(3);
    } ImGui::End();
}


int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("ImGui Example"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Dear ImGui DirectX11 Example"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);


    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);



    ImFontConfig iconfontcfg;
    iconfontcfg.MergeMode = true;
    iconfontcfg.GlyphMaxAdvanceX = 13.0f;
    static const ImWchar icon_ranges[] = { 0xe900, 0xe909, 0 };
    //fonts
    mainfont = io.Fonts->AddFontFromMemoryTTF(lexend, sizeof(lexend), 16.0f);
    smallfont = io.Fonts->AddFontFromMemoryTTF(lexend, sizeof(lexend), 13.0f);
    iconfont = io.Fonts->AddFontFromMemoryTTF(iconfontmem, sizeof(iconfontmem), 17.0f, &iconfontcfg, icon_ranges);
    logofont = io.Fonts->AddFontFromMemoryTTF(lexendsb, sizeof(lexendsb), 31.0f);
    
    /*HRESULT usericonthread = D3DX11CreateTextureFromMemory(g_pd3dDevice, usericonmem, sizeof(usericonmem), nullptr, nullptr, &usericon, &usericonthread);*/
    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        RenderUI();


        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
