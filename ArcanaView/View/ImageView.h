#pragma once
#include "UIComponentBase.h"

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
	void Draw();
	void Cleanup();
	void CreateImageTexture();

	void UpdateDrawList();

private:
	std::wstring _imageFile = L""; // Path to the image file

	DirectX::ScratchImage _image;
	DirectX::TexMetadata _metadata = {};
	ComPtr<ID3D11ShaderResourceView> _shaderResourceView = nullptr;

private:
	ImVec2 _mousePos = { 0.0f, 0.0f };
	ImVec2 _contentRegionSize = { 0.0f, 0.0f };

private:
	ImVec2 _lastMousePos = { 0.0f, 0.0f };
	bool _isLineTool = false;
	ImGuiWindowFlags _windowFlags = ImGuiWindowFlags_None;;

public:
	static uint64 ImageViewId;
};