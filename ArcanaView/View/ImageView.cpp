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

// ImGui 기본 셰이더로 바로 표시 가능한 8bit RGBA/BGRA 포맷인지 판별한다.
static bool IsDisplayableFormat(DXGI_FORMAT format)
{
	switch (format)
	{
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			return true;
		default:
			return false;
	}
}

void ImageView::CreateImageTexture()
{
	HRESULT hr = DirectX::LoadFromWICFile(_imageFile.c_str(), DirectX::WIC_FLAGS_NONE, &_metadata, _image);
	if (hr != S_OK)
	{
		wchar_t msg[256];
		swprintf_s(msg, L"[ImageView] LoadFromWICFile failed. hr=0x%08X, file=%s\n", hr, _imageFile.c_str());
		OutputDebugStringW(msg);
		return;
	}

	// WIC가 매핑한 실제 DXGI 포맷을 진단용으로 출력 (예: 32bit float TIFF -> R32_FLOAT / R32G32B32A32_FLOAT)
	{
		wchar_t msg[128];
		swprintf_s(msg, L"[ImageView] Loaded format=%d (%zux%zu)\n", static_cast<int>(_metadata.format), _metadata.width, _metadata.height);
		OutputDebugStringW(msg);
	}

	// 표시(렌더링)용 텍스처는 ImGui 기본 셰이더와 호환되는 8bit RGBA로 변환한다.
	// 원본 _image(float 등)는 픽셀 검사기에서 raw 값을 읽기 위해 그대로 보존한다.
	if (IsDisplayableFormat(_metadata.format))
	{
		hr = DirectX::CreateShaderResourceView(DEVICE.Get(), _image.GetImages(), _image.GetImageCount(), _metadata, _shaderResourceView.GetAddressOf());
		if (hr != S_OK)
		{
			wchar_t msg[128];
			swprintf_s(msg, L"[ImageView] CreateShaderResourceView failed. hr=0x%08X\n", hr);
			OutputDebugStringW(msg);
			return;
		}
	}
	else
	{
		DirectX::ScratchImage converted;
		hr = DirectX::Convert(_image.GetImages(), _image.GetImageCount(), _metadata,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DirectX::TEX_FILTER_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT, converted);
		if (hr != S_OK)
		{
			wchar_t msg[128];
			swprintf_s(msg, L"[ImageView] Convert to R8G8B8A8_UNORM failed. hr=0x%08X, srcFormat=%d\n", hr, static_cast<int>(_metadata.format));
			OutputDebugStringW(msg);
			return;
		}

		hr = DirectX::CreateShaderResourceView(DEVICE.Get(), converted.GetImages(), converted.GetImageCount(), converted.GetMetadata(), _shaderResourceView.GetAddressOf());
		if (hr != S_OK)
		{
			wchar_t msg[128];
			swprintf_s(msg, L"[ImageView] CreateShaderResourceView (converted) failed. hr=0x%08X\n", hr);
			OutputDebugStringW(msg);
			return;
		}
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
		const DirectX::Image* img = _image.GetImage(0, 0, 0);

		if (img != nullptr)
		{
			float xRatio = _metadata.width / windowContentRegionAvail.x;
			float yRatio = _metadata.height / (windowContentRegionAvail.y - textlineWithSpacing);

			size_t col = static_cast<size_t>(mousePosX * xRatio);
			size_t row = static_cast<size_t>(mousePosY * yRatio);

			if (col < _metadata.width && row < _metadata.height)
			{
				// 포맷별 픽셀당 바이트 수와 행 피치(패딩 포함)를 사용해 정확한 바이트 오프셋 계산
				size_t bytesPerPixel = DirectX::BitsPerPixel(_metadata.format) / 8;
				size_t byteOffset = row * img->rowPitch + col * bytesPerPixel;

				pixelValue = GetPixelValue(img->pixels, byteOffset, _metadata.format);
			}
		}
	}
	else
	{
		mousePosX = 0;
		mousePosY = 0;
	}

	ImGui::Text("X: %f Y: %f R: %g G: %g B: %g A: %g", mousePosX, mousePosY, pixelValue.R, pixelValue.G, pixelValue.B, pixelValue.A);
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
			// 현재 이미지를 Visual Sequence Graph 의 Image Source 노드로 승격.
			LAYOUT->PromoteImageToGraph(_image);
		}
		ImGui::EndPopup();
	}

	_imageTool.Update(drawList);
}

PixelValue GetPixelValue(const uint8* pixels, size_t byteOffset, const DXGI_FORMAT& format)
{
	const uint8* p = pixels + byteOffset;

	switch (format)
	{
		// 8bit UNORM: 바이트 값(0~255)을 그대로 표시
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return { static_cast<float>(p[0]), static_cast<float>(p[1]), static_cast<float>(p[2]), static_cast<float>(p[3]) };
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			return { static_cast<float>(p[2]), static_cast<float>(p[1]), static_cast<float>(p[0]), static_cast<float>(p[3]) };

		// 32bit float: raw 값을 그대로 표시
		case DXGI_FORMAT_R32_FLOAT:
		{
			const float* f = reinterpret_cast<const float*>(p);
			return { f[0], 0.0f, 0.0f, 1.0f };
		}
		case DXGI_FORMAT_R32G32B32_FLOAT:
		{
			const float* f = reinterpret_cast<const float*>(p);
			return { f[0], f[1], f[2], 1.0f };
		}
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		{
			const float* f = reinterpret_cast<const float*>(p);
			return { f[0], f[1], f[2], f[3] };
		}

		default:
			return { 0.0f, 0.0f, 0.0f, 0.0f }; // Unsupported format
	}
}
