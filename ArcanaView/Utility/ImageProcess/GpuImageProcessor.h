#pragma once
// NodeOp 정의를 위해 VSG 데이터 모델 헤더를 포함한다. (pch가 먼저 포함되어야 함 - 프로젝트 관례)
#include "../../View/VisualSequenceGraph/VSGType.h"

// 그래프를 흐르는 이미지의 GPU 표현.
// 포맷은 DXGI_FORMAT_R32G32B32A32_FLOAT 로 통일한다 (float 중심 픽셀 규약 + 정밀도 보존).
struct GpuImage
{
	ComPtr<ID3D11Texture2D>           Texture;
	ComPtr<ID3D11ShaderResourceView>  SRV;   // 읽기(t0) + ImGui::Image 표시용
	ComPtr<ID3D11UnorderedAccessView> UAV;   // compute 쓰기(u0)

	UINT Width = 0;
	UINT Height = 0;
};

// Compute Shader 기반 이미지 처리기.
// HLSL을 내장 문자열로 들고 런타임 D3DCompile 로 컴파일/캐시한다.
class GpuImageProcessor
{
public:
	// ScratchImage(원본 포맷)를 R32G32B32A32_FLOAT GPU 텍스처로 업로드한다. (Promote 경로)
	GpuImage* UploadFromScratchImage(const DirectX::ScratchImage& src);

	// UAV+SRV 를 가진 빈 출력 텍스처를 만든다. (연산 결과 대상)
	GpuImage* CreateTarget(UINT width, UINT height);

	// op 에 해당하는 Compute Shader 로 in -> out 처리. 지원하지 않는 op 면 false.
	bool Apply(NodeOp op, const GpuImage& in, GpuImage& out);

private:
	ID3D11ComputeShader* GetOrCompileShader(NodeOp op);
	ComPtr<ID3D11ComputeShader> CompileComputeShader(const char* source);

private:
	ComPtr<ID3D11ComputeShader> _grayscaleCS;
	ComPtr<ID3D11ComputeShader> _invertCS;
};
