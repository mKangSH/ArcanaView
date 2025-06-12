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

		ImGui::End();
	}
}

void ImageView::Render()
{

}

void ImageView::UpdateWindowInfo()
{
	ImVec2 screenPos = ImGui::GetCursorScreenPos();
	
	ImVec2 mousePos = ImGui::GetMousePos();

	ImGui::SetNextWindowPos(_windowPos, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(_windowSize, ImGuiCond_FirstUseEver);

	float textlineWithSpacing = ImGui::GetTextLineHeightWithSpacing();
	
	bool isMouseInContent = mousePos.x >= _windowPos.x &&
							mousePos.x < _windowPos.x + _windowContentSize.x &&
							mousePos.y >= _windowPos.y &&
							mousePos.y < _windowPos.y + _windowContentSize.y + textlineWithSpacing;

	if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && isMouseInContent)
	{
		_windowFlags |= ImGuiWindowFlags_NoMove;
	}
	else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		_windowFlags &= ~ImGuiWindowFlags_NoMove;
	}
}

void ImageView::Draw()
{
	_windowPos = ImGui::GetCursorScreenPos();

	DrawImageValue();

	_windowContentSize = ImGui::GetContentRegionAvail();

	UpdateDrawList();

	_windowSize = ImGui::GetWindowSize();
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

void ImageView::DrawImageValue()
{
	ImGui::RadioButton("None", (int*)&_imageTool.GetType(), 0); ImGui::SameLine();
	ImGui::RadioButton("Line", (int*)&_imageTool.GetType(), 1); ImGui::SameLine();
	ImGui::RadioButton("Rectangle", (int*)&_imageTool.GetType(), 2); ImGui::SameLine();

	ImGui::ColorEdit4("BrushToolColor", _imageTool.GetColor(), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

	ImVec2 mousePos = ImGui::GetMousePos();
	ImVec2 windowTopLeft = ImGui::GetCursorScreenPos();
	
	float textlineWithSpacing = ImGui::GetTextLineHeightWithSpacing();

	float mousePosX = mousePos.x - windowTopLeft.x;
	float mousePosY = mousePos.y - windowTopLeft.y - textlineWithSpacing;

	PixelValue pixelValue = { 0, 0, 0, 0 };

	ImVec2 windowContentRegionAvail = ImGui::GetContentRegionAvail();

	if (ImGui::IsWindowHovered() && 
		mousePosX >= 0 && mousePosX < windowContentRegionAvail.x && 
		mousePosY >= 0 && mousePosY < windowContentRegionAvail.y)
	{
		const uint8* pixels = _image.GetPixels();
		
		float xRatio = _metadata.width / windowContentRegionAvail.x;
		float yRatio = _metadata.height / (windowContentRegionAvail.y - textlineWithSpacing);

		uint32 pixelIndex = static_cast<uint32>(mousePosY * yRatio) * _metadata.width + static_cast<uint32>(mousePosX * xRatio);

		if (pixelIndex < _metadata.width * _metadata.height)
		{
			pixelValue = GetPixelValue(pixels, pixelIndex * 4, _metadata.format);
		}
	}
	else
	{
		mousePosX = 0;
		mousePosY = 0;
	}
	
	ImGui::Text("X: %f Y: %f R: %u G: %u B: %u A: %u", mousePosX, mousePosY, pixelValue.R, pixelValue.G, pixelValue.B, pixelValue.A);
}

void ImageView::UpdateDrawList()
{
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImVec2 p0 = ImGui::GetCursorScreenPos();
	ImVec2 p1 = ImGui::GetContentRegionAvail();
	p1.x += p0.x;
	p1.y += p0.y;

	// Zoom the image using uv coordinates with mouse position as the center of zoom when the mouse wheel is scrolled
	if (ImGui::IsWindowHovered())
	{
		ImVec2 mousePos = ImGui::GetMousePos();
		ImVec2 mouseUV = ImVec2((mousePos.x - p0.x) / (p1.x - p0.x), (mousePos.y - p0.y) / (p1.y - p0.y));

		const float zoomSensitivity = 0.1f;
		float zoomFactor = 1.0f;

		if (ImGui::GetIO().MouseWheel > 0.0f)
		{
			// Zoom in (UV 범위를 줄여서 확대)
			zoomFactor = 1.0f - zoomSensitivity;
		}		
		else if (ImGui::GetIO().MouseWheel < 0.0f)
		{
			// Zoom out (UV 범위를 늘려서 축소)
			zoomFactor = 1.0f + zoomSensitivity;
		}

		if (zoomFactor != 1.0f)
		{
			ImVec2 oldUVSize = ImVec2(_uv1.x - _uv0.x, _uv1.y - _uv0.y);
			ImVec2 newUVSize = ImVec2(oldUVSize.x * zoomFactor, oldUVSize.y * zoomFactor);

			ImVec2 texMouse = ImVec2(_uv0.x + mouseUV.x * oldUVSize.x, _uv0.y + mouseUV.y * oldUVSize.y);
			
			_uv0.x = texMouse.x - mouseUV.x * newUVSize.x;
			_uv0.y = texMouse.y - mouseUV.y * newUVSize.y;

			_uv1.x = _uv0.x + newUVSize.x;
			_uv1.y = _uv0.y + newUVSize.y;
		}
	}

	// Clamp UV coordinates to [0, 1] range
	_uv0.x = std::clamp(_uv0.x, 0.0f, 1.0f);
	_uv0.y = std::clamp(_uv0.y, 0.0f, 1.0f);

	_uv1.x = std::clamp(_uv1.x, 0.0f, 1.0f);
	_uv1.y = std::clamp(_uv1.y, 0.0f, 1.0f);

	drawList->AddImage(reinterpret_cast<ImTextureID>(_shaderResourceView.Get()), p0, p1, _uv0, _uv1);

	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
	{
		ImGui::OpenPopup("Image Processing");
	}

	if (ImGui::BeginPopup("Image Processing"))
	{
		if (ImGui::MenuItem("Promote variable"))
		{
		}
		ImGui::EndPopup();
	}

	_imageTool.Update(drawList);
}

PixelValue GetPixelValue(const uint8* pixels, uint32 pixelIndex, const DXGI_FORMAT& format)
{
	switch (format)
	{
		case DXGI_FORMAT_R8G8B8A8_UNORM:
			return { pixels[pixelIndex + 0], pixels[pixelIndex + 1], pixels[pixelIndex + 2], pixels[pixelIndex + 3] };
		case DXGI_FORMAT_B8G8R8A8_UNORM:
			return { pixels[pixelIndex + 2], pixels[pixelIndex + 1], pixels[pixelIndex + 0], pixels[pixelIndex + 3] };
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return { pixels[pixelIndex + 0], pixels[pixelIndex + 1], pixels[pixelIndex + 2], pixels[pixelIndex + 3] };
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			return { pixels[pixelIndex + 2], pixels[pixelIndex + 1], pixels[pixelIndex + 0], pixels[pixelIndex + 3] };
		default:
			return { 0, 0, 0, 0 }; // Unsupported format
	}
}
