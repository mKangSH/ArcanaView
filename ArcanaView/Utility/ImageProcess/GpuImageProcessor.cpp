#include "pch.h"
#include "GpuImageProcessor.h"

// ---- 내장 Compute Shader (HLSL) ---------------------------------------------
// 입력은 Texture2D<float4> SRV(t0)로 읽고, 출력은 RWTexture2D<float4> UAV(u0)로 쓰기만 한다.
// (R32G32B32A32_FLOAT 는 typed-UAV store 를 지원하므로 안전. typed-UAV load 는 사용하지 않음)

static const char* g_grayscaleHLSL = R"hlsl(
Texture2D<float4>   InputTex  : register(t0);
RWTexture2D<float4> OutputTex : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    float4 c = InputTex[id.xy];
    float l = dot(c.rgb, float3(0.2126, 0.7152, 0.0722));
    OutputTex[id.xy] = float4(l, l, l, c.a);
}
)hlsl";

static const char* g_invertHLSL = R"hlsl(
Texture2D<float4>   InputTex  : register(t0);
RWTexture2D<float4> OutputTex : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    float4 c = InputTex[id.xy];
    OutputTex[id.xy] = float4(1.0 - c.rgb, c.a);
}
)hlsl";

// ---- Shader 컴파일 ----------------------------------------------------------

ComPtr<ID3D11ComputeShader> GpuImageProcessor::CompileComputeShader(const char* source)
{
	ComPtr<ID3DBlob> shaderBlob;
	ComPtr<ID3DBlob> errorBlob;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = D3DCompile(source, strlen(source), nullptr, nullptr, nullptr,
		"main", "cs_5_0", flags, 0, shaderBlob.GetAddressOf(), errorBlob.GetAddressOf());
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA("[GpuImageProcessor] D3DCompile failed:\n");
			OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
		}
		return nullptr;
	}

	ComPtr<ID3D11ComputeShader> cs;
	hr = DEVICE->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, cs.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringW(L"[GpuImageProcessor] CreateComputeShader failed.\n");
		return nullptr;
	}

	return cs;
}

ID3D11ComputeShader* GpuImageProcessor::GetOrCompileShader(NodeOp op)
{
	switch (op)
	{
	case NodeOp::Grayscale:
		if (!_grayscaleCS) _grayscaleCS = CompileComputeShader(g_grayscaleHLSL);
		return _grayscaleCS.Get();
	case NodeOp::Invert:
		if (!_invertCS) _invertCS = CompileComputeShader(g_invertHLSL);
		return _invertCS.Get();
	default:
		return nullptr;
	}
}

// ---- 텍스처 생성 / 업로드 ----------------------------------------------------

GpuImage* GpuImageProcessor::UploadFromScratchImage(const DirectX::ScratchImage& src)
{
	const DirectX::TexMetadata& srcMeta = src.GetMetadata();

	// float 파이프라인 일관성을 위해 R32G32B32A32_FLOAT 로 변환한다.
	DirectX::ScratchImage converted;
	const DirectX::ScratchImage* floatImage = &src;

	if (srcMeta.format != DXGI_FORMAT_R32G32B32A32_FLOAT)
	{
		HRESULT hr = DirectX::Convert(src.GetImages(), src.GetImageCount(), srcMeta,
			DXGI_FORMAT_R32G32B32A32_FLOAT, DirectX::TEX_FILTER_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT, converted);
		if (FAILED(hr))
		{
			OutputDebugStringW(L"[GpuImageProcessor] Convert to R32G32B32A32_FLOAT failed.\n");
			return nullptr;
		}
		floatImage = &converted;
	}

	const DirectX::Image* img = floatImage->GetImage(0, 0, 0);
	if (img == nullptr)
	{
		return nullptr;
	}

	GpuImage* gpu = new GpuImage();
	gpu->Width = static_cast<UINT>(img->width);
	gpu->Height = static_cast<UINT>(img->height);

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = gpu->Width;
	desc.Height = gpu->Height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = img->pixels;
	initData.SysMemPitch = static_cast<UINT>(img->rowPitch);

	HRESULT hr = DEVICE->CreateTexture2D(&desc, &initData, gpu->Texture.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringW(L"[GpuImageProcessor] CreateTexture2D (source) failed.\n");
		delete gpu;
		return nullptr;
	}

	hr = DEVICE->CreateShaderResourceView(gpu->Texture.Get(), nullptr, gpu->SRV.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringW(L"[GpuImageProcessor] CreateShaderResourceView (source) failed.\n");
		delete gpu;
		return nullptr;
	}

	return gpu;
}

GpuImage* GpuImageProcessor::CreateTarget(UINT width, UINT height)
{
	if (width == 0 || height == 0)
	{
		return nullptr;
	}

	GpuImage* gpu = new GpuImage();
	gpu->Width = width;
	gpu->Height = height;

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

	HRESULT hr = DEVICE->CreateTexture2D(&desc, nullptr, gpu->Texture.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringW(L"[GpuImageProcessor] CreateTexture2D (target) failed.\n");
		delete gpu;
		return nullptr;
	}

	hr = DEVICE->CreateShaderResourceView(gpu->Texture.Get(), nullptr, gpu->SRV.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringW(L"[GpuImageProcessor] CreateShaderResourceView (target) failed.\n");
		delete gpu;
		return nullptr;
	}

	hr = DEVICE->CreateUnorderedAccessView(gpu->Texture.Get(), nullptr, gpu->UAV.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringW(L"[GpuImageProcessor] CreateUnorderedAccessView (target) failed.\n");
		delete gpu;
		return nullptr;
	}

	return gpu;
}

// ---- 처리 실행 --------------------------------------------------------------

bool GpuImageProcessor::Apply(NodeOp op, const GpuImage& in, GpuImage& out)
{
	ID3D11ComputeShader* cs = GetOrCompileShader(op);
	if (cs == nullptr || in.SRV == nullptr || out.UAV == nullptr)
	{
		return false;
	}

	ID3D11DeviceContext* ctx = DEVICECONTEXT.Get();

	ID3D11ShaderResourceView* srv = in.SRV.Get();
	ID3D11UnorderedAccessView* uav = out.UAV.Get();

	ctx->CSSetShader(cs, nullptr, 0);
	ctx->CSSetShaderResources(0, 1, &srv);
	ctx->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);

	UINT groupsX = (out.Width + 7) / 8;
	UINT groupsY = (out.Height + 7) / 8;
	ctx->Dispatch(groupsX, groupsY, 1);

	// 후속 패스/ImGui 렌더와의 리소스 해저드 방지를 위해 바인딩 해제.
	ID3D11ShaderResourceView* nullSRV = nullptr;
	ID3D11UnorderedAccessView* nullUAV = nullptr;
	ctx->CSSetShaderResources(0, 1, &nullSRV);
	ctx->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
	ctx->CSSetShader(nullptr, nullptr, 0);

	return true;
}
