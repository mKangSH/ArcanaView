// Dear ImGui: standalone example application for DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "pch.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>

#include "./Manager/Graphics.h"
#include "./Manager/ImGuiManager.h"
#include "./Manager/LayoutManager.h"

UINT _resizeWidth = 0;
UINT _resizeHeight = 0;

// Forward declarations of helper functions
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    // Create application window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Arcana View", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    if (GRAPHICS->Init(hwnd) == false)
    {
        GRAPHICS->Cleanup();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }
    
    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);
    ::DragAcceptFiles(hwnd, TRUE);

    GUI->Init(hwnd);
    LAYOUT->Init();
    
    bool done = false;
    bool swapChainOccluded = false;
    
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
            {
                done = true;
            }
        }

        if (done)
        {
            break;
        }
        
        // Handle window being minimized or screen locked
        if (swapChainOccluded && GRAPHICS->GetSwapChain()->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        swapChainOccluded = false;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (_resizeWidth != 0 && _resizeHeight != 0)
        {
            GRAPHICS->Resize(&_resizeWidth, &_resizeHeight);
        }

        // Start the Dear ImGui frame
        GUI->Update();
        LAYOUT->Update();
        
        // Rendering
        GRAPHICS->RenderBegin();
        GUI->Render();
        LAYOUT->Render();
        swapChainOccluded = GRAPHICS->RenderEnd();
    }

    // Cleanup
    GUI->Cleanup();
    GRAPHICS->Cleanup();
    
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
        case WM_SIZE:
        {
            if (wParam == SIZE_MINIMIZED)
            {
                return 0;
            }

            _resizeWidth = (UINT)LOWORD(lParam); // Queue resize
            _resizeHeight = (UINT)HIWORD(lParam);
        }
        return 0;

        case WM_DROPFILES:
        {
            UINT count = DragQueryFile((HDROP)wParam, 0xFFFFFFFF, NULL, 0);

            ImGuiIO& io = ImGui::GetIO();
            ImGuiID guiId = io.MouseHoveredViewport;

            for (UINT i = 0; i < count; i++)
            {
                UINT nLength = DragQueryFile((HDROP)wParam, i, NULL, 0);
                std::wstring buffer;
                buffer.reserve(nLength + 1);

                DragQueryFile((HDROP)wParam, i, buffer.data(), nLength + 1);

                LAYOUT->AddImageView(buffer);
            }
        }
        return 0;

        case WM_SYSCOMMAND:
        {
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            {
                return 0;
            }
        }
        break;
        
        case WM_DESTROY:
        {
            ::PostQuitMessage(0);
        }
        return 0;

        case WM_DPICHANGED:
        {
            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
            {
                //const int dpi = HIWORD(wParam);
                //printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
                const RECT* suggested_rect = (RECT*)lParam;
                ::SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
            }
        }
        break;
    }

    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
