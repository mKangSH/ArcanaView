#pragma once
#include "UIComponentBase.h"

struct PixelValue
{
	uint8 R, G, B, A;
};

class ImageView : public UIComponentBase
{
	using Super = UIComponentBase;

public:
	ImageView(const std::wstring& imageFile);
	virtual ~ImageView();

public:
	void Update() override;
	void Render() override;

private:
	void UpdateWindowInfo();

	void Draw();
	void Cleanup();

	void CreateImageTexture();

	void DrawImageValue();
	void UpdateDrawList();

private:
	std::wstring _imageFile = L""; // Path to the image file

	DirectX::ScratchImage _image;
	DirectX::TexMetadata _metadata = {};
	ComPtr<ID3D11ShaderResourceView> _shaderResourceView = nullptr;

private:
	ImVec2 _uv0 = ImVec2(0, 0);
	ImVec2 _uv1 = ImVec2(1, 1);

private:
	ImVec2 _windowPos = { 300.0f, 300.0f };
	ImVec2 _windowSize = { 400.0f, 400.0f };
	ImVec2 _windowContentSize = { 100.0f, 100.0f };
	ImGuiWindowFlags _windowFlags = ImGuiWindowFlags_None | ImGuiWindowFlags_NoSavedSettings;

	ImageTool _imageTool;

public:
	static uint64 ImageViewId;
};

PixelValue GetPixelValue(const uint8* pixels, uint32 pixelIndex, const DXGI_FORMAT& format);
