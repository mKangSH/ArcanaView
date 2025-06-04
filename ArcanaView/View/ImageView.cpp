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
		Draw();
	}
}

void ImageView::Render()
{

}

void ImageView::Draw()
{
	ImVec2 contentRegionSize = ImGui::GetWindowSize();

	ImVec2 mousePos = ImGui::GetMousePos();
	if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
		//&& (mousePos.x >= screenPos.x && mousePos.x < (screenPos.x + contentRegionSize.x)) 
		//&& (mousePos.y >= screenPos.y && mousePos.y < (screenPos.y + contentRegionSize.y)))
	{
		_windowFlags |= ImGuiWindowFlags_NoMove;
	}
	else if (ImGui::IsMouseReleasedWithDelay(ImGuiMouseButton_Left, 0.5))
	{
		_windowFlags &= ~ImGuiWindowFlags_NoMove;
	}

	ImGui::Begin(_title.c_str(), &_isVisible, _windowFlags);
	
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

	UpdateDrawList();

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
	if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && 
		(mousePos.x >= p0.x && mousePos.x < p1.x) &&
		(mousePos.y >= p0.y && mousePos.y < p1.y))
	{
		if (_isLineTool == false)
		{
			_isLineTool = true;
			_lastMousePos = ImGui::GetMousePos();
		}
		else 
		{
			ImVec2 position = ImGui::GetMousePos();
			drawList->AddLine(_lastMousePos, position, ImU32(0xFFFFFFFF));
		}
	}
	else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		_isLineTool = false;
	}

	_mousePos = ImGui::GetMousePos();

	_mousePos.x -= p0.x;
	_mousePos.y -= p0.y;
}