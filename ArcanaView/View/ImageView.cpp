#include "pch.h"
#include "ImageView.h"

// static member
uint64 ImageView::ImageViewId = 1;

ImageView::ImageView(const std::wstring& imageFile) 
	: Super("Image View " + std::to_string(ImageViewId))
{
	ImageViewId++;

	_imageFile = imageFile.c_str();
	CreateImageTexture();
}

ImageView::~ImageView()
{
	ImageView::ImageViewId--;
}

void ImageView::Update()
{
	if (_isVisible)
	{
		UpdateWindowInfo();

		ImGui::Begin(_title.c_str(), &_isVisible, _windowFlags);

		Draw();

		UpdateDrawList();

		ImGui::End();
	}
}

void ImageView::Render()
{

}

void ImageView::UpdateWindowInfo()
{
	// 부모 윈도우의 위치를 이용하여 현재 윈도우 포지션을 세팅한다.
	ImVec2 parentWindowPos = ImGui::GetWindowPos();
	parentWindowPos.x += _windowPos.x;
	parentWindowPos.y += _windowPos.y;
	ImGui::SetNextWindowPos(parentWindowPos, ImGuiCond_FirstUseEver);

	float titleBarHeight = ImGui::GetFontSize() + ImGui::GetFrameHeight() * 2;

	ImVec2 mousePos = ImGui::GetMousePos();
	bool isMouseInContent = mousePos.x >= parentWindowPos.x &&
							mousePos.x < parentWindowPos.x + _windowContentSize.x &&
							mousePos.y >= parentWindowPos.y + titleBarHeight &&
							mousePos.y < parentWindowPos.y + _windowContentSize.y;

	if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && isMouseInContent)
	{
		_windowFlags |= ImGuiWindowFlags_NoMove;
	}
	else if (ImGui::IsMouseReleasedWithDelay(ImGuiMouseButton_Left, 0.5))
	{
		_windowFlags &= ~ImGuiWindowFlags_NoMove;
	}
}

void ImageView::Draw()
{
	_windowContentSize = ImGui::GetContentRegionAvail();

	if (ImGui::IsWindowHovered() && _contentRegionSize.x > 0 && _contentRegionSize.y > 0)
	{
		_mousePos.x = std::clamp(_mousePos.x, 0.0f, _contentRegionSize.x - 1);
		_mousePos.y = std::clamp(_mousePos.y, 0.0f, _contentRegionSize.y - 1);

		uint8* pixels = _image.GetPixels();

		float xRatio = _metadata.width / _contentRegionSize.x;
		float yRatio = _metadata.height / _contentRegionSize.y;

		_mousePos.x = std::clamp(ceil(_mousePos.x * xRatio), 0.0f, (float)_metadata.width - 1);
		_mousePos.y = std::clamp(ceil(_mousePos.y * yRatio), 0.0f, (float)_metadata.height - 1);

		int pixelIndex = ((_mousePos.y * _metadata.width) + _mousePos.x) * 4;

		// pixes => B G R A
		if (_metadata.format == DXGI_FORMAT_R8G8B8A8_UNORM)
		{
			ImGui::Text("X: %f Y: %f Value: %u %u %u %u", _mousePos.x, _mousePos.y, pixels[pixelIndex], pixels[pixelIndex + 1], pixels[pixelIndex + 2], pixels[pixelIndex + 3]);
		}
		else if (_metadata.format == DXGI_FORMAT_B8G8R8A8_UNORM)
		{
			ImGui::Text("X: %f Y: %f Value: %u %u %u %u", _mousePos.x, _mousePos.y, pixels[pixelIndex + 2], pixels[pixelIndex + 1], pixels[pixelIndex], pixels[pixelIndex + 3]);
		}
	}
	else
	{
		_mousePos.x = 0;
		_mousePos.y = 0;

		ImGui::Text("X: %f Y: %f Value: %u %u %u %u", _mousePos.x, _mousePos.y, 0, 0, 0, 0);
	}
}

void ImageView::Cleanup()
{
}

void ImageView::CreateImageTexture()
{
	HRESULT hr = DirectX::LoadFromWICFile(_imageFile.c_str(), DirectX::WIC_FLAGS_NONE, &_metadata, _image);
	if (hr != S_OK) 
	{
		// TODO : Error Logging

		return;
	}

	hr = DirectX::CreateShaderResourceView(DEVICE.Get(), _image.GetImages(), _image.GetImageCount(), _metadata, _shaderResourceView.GetAddressOf());
	if (hr != S_OK) 
	{
		// TODO : Error Logging
		return;
	}
}

void ImageView::UpdateDrawList()
{
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImVec2 p0 = ImGui::GetCursorScreenPos();
	_contentRegionSize = ImGui::GetContentRegionAvail();

	ImVec2 p1 = _contentRegionSize;
	p1.x += p0.x;
	p1.y += p0.y;

	drawList->AddImage(reinterpret_cast<ImTextureID>(_shaderResourceView.Get()), p0, p1);

	ImVec2 mousePos = ImGui::GetMousePos();
	_mousePos = ImGui::GetMousePos();

	_mousePos.x -= p0.x;
	_mousePos.y -= p0.y;
}