#pragma once

class Graphics
{
	DECLARE_SINGLETON(Graphics)

public:
	bool Init(HWND hwnd);
	void RenderBegin();
	bool RenderEnd();
	void Resize(UINT* width, UINT* height);
	void Cleanup();

	ComPtr<ID3D11Device> GetDevice() { return _device; }
	ComPtr<ID3D11DeviceContext> GetDeviceContext() { return _deviceContext; }
	ComPtr<IDXGISwapChain> GetSwapChain() { return _swapChain; }
	ComPtr<ID3D11RenderTargetView> GetRenderTargetView() { return _mainRenderTargetView; }

private:
	// Forward declarations of helper functions
	bool CreateDeviceD3D(HWND hWnd);
	void CleanupDeviceD3D();

	void CreateRenderTarget();
	void CleanupRenderTarget();

private:
	// DirectX11 Device & SwapChain
	ComPtr<ID3D11Device> _device = nullptr; // »ý¼º
	ComPtr<ID3D11DeviceContext> _deviceContext = nullptr; // Rendering
	ComPtr<IDXGISwapChain> _swapChain = nullptr;
	ComPtr<ID3D11RenderTargetView> _mainRenderTargetView = nullptr;
};

