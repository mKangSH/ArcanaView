#include "pch.h"
#include "ImageView.h"
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
		Draw();
	}
}

void ImageView::Render()
{

}

void ImageView::Draw()
{
	ImGui::Begin(_title.c_str(), &_isVisible);

	if (_contentRegionSize.x <= 0.0f || _contentRegionSize.y <= 0.0f)
	{
		_mousePos.x = 0;
		_mousePos.y = 0;

		ImGui::Text("X: %f Y: %f Value: %u %u %u %u", _mousePos.x, _mousePos.y, 0, 0, 0, 0);
	}
	else
	{
		if (ImGui::IsWindowHovered())
		{
			_mousePos.x = std::clamp(_mousePos.x, 0.0f, _contentRegionSize.x - 1);
			_mousePos.y = std::clamp(_mousePos.y, 0.0f, _contentRegionSize.y - 1);

			uint8* pixels = _image.GetPixels();

			float xRatio = _metadata.width / _contentRegionSize.x;
			float yRatio = _metadata.height / _contentRegionSize.y;

			_mousePos.x = std::clamp(ceil(_mousePos.x * xRatio), 0.0f, (float)_metadata.width - 1);
			_mousePos.y = std::clamp(ceil(_mousePos.y * yRatio), 0.0f, (float)_metadata.height - 1);

			int pixelIndex = ((_mousePos.y * _metadata.width) + _mousePos.x) * 4;

			ImGui::Text("X: %f Y: %f Value: %u %u %u %u", _mousePos.x, _mousePos.y, pixels[pixelIndex], pixels[pixelIndex + 1], pixels[pixelIndex + 2], pixels[pixelIndex + 3]);
		}
		else
		{
			_mousePos.x = 0;
			_mousePos.y = 0;

			ImGui::Text("X: %f Y: %f Value: %u %u %u %u", _mousePos.x, _mousePos.y, 0, 0, 0, 0);
		}
	}

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	{
		ImVec2 p0 = ImGui::GetCursorScreenPos();

		_contentRegionSize = ImGui::GetContentRegionAvail();
		
		ImVec2 p1;
		p1.x = p0.x + _contentRegionSize.x;
		p1.y = p0.y + _contentRegionSize.y;

		drawList->AddImage(reinterpret_cast<ImTextureID>(_shaderResourceView.Get()), p0, p1);

		_mousePos = ImGui::GetMousePos();

		_mousePos.x -= p0.x;
		_mousePos.y -= p0.y;
	}

	ImGui::End();
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