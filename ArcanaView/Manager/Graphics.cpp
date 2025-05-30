#include "pch.h"
#include "Graphics.h"
#include "Utility/ImGui/imgui.h"

bool Graphics::Init(HWND hwnd)
{
    // Initialize Direct3D
    bool result = CreateDeviceD3D(hwnd);

    return result;
}

void Graphics::RenderBegin()
{
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    const float clear_color_with_alpha[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
    _deviceContext->OMSetRenderTargets(1, _mainRenderTargetView.GetAddressOf(), nullptr);
    _deviceContext->ClearRenderTargetView(_mainRenderTargetView.Get(), clear_color_with_alpha);
}

bool Graphics::RenderEnd()
{
    // Present
    HRESULT hr = _swapChain->Present(1, 0);   // Present with vsync
    //HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync

    return (hr == DXGI_STATUS_OCCLUDED);
}

void Graphics::Resize(UINT* width, UINT* height)
{
    CleanupRenderTarget();
    _swapChain->ResizeBuffers(0, *width, *height, DXGI_FORMAT_UNKNOWN, 0);

    *width = 0;
    *height = 0;

    CreateRenderTarget();
}

void Graphics::Cleanup()
{
    CleanupDeviceD3D();
}

bool Graphics::CreateDeviceD3D(HWND hWnd)
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
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &_swapChain, &_device, &featureLevel, &_deviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
    {
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &_swapChain, &_device, &featureLevel, &_deviceContext);
    }
        
    if (res != S_OK)
    {
        return false;
    }
        
    CreateRenderTarget();
    return true;
}

void Graphics::CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (_swapChain)
    {
        _swapChain = nullptr;
    }

    if (_deviceContext)
    {
        _deviceContext = nullptr;
    }

    if (_device)
    {
        _device = nullptr;
    }
}

void Graphics::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    _swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    _device->CreateRenderTargetView(pBackBuffer, nullptr, &_mainRenderTargetView);
    pBackBuffer->Release();
}

void Graphics::CleanupRenderTarget()
{
    if (_mainRenderTargetView)
    {
        _mainRenderTargetView = nullptr;
    }
}
