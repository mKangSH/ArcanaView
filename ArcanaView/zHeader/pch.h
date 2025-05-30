#pragma once

#define WIN32_LEAN_AND_MEAN

#include "Define.h"

// STL
#include <memory>
#include <iostream>
#include <array>
#include <vector>
#include <list>
#include <queue>
#include <map>
#include <unordered_map>
#include <string>
#include <algorithm>

// WIN
#include <windows.h>
#include <assert.h>
#include <optional>
#include <shellapi.h>

// DirectX
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

// DirectXTex
#include <DirectXTex/DirectXTex.h>
#include <DirectXTex/DirectXTex.inl>
using namespace DirectX;
using namespace Microsoft::WRL;

// ImGui
#include "Utility/ImGui/imgui.h"
#include "Utility/ImPlot/implot.h"
#include "Utility/ImPlot3D/implot3d.h"

// Libs
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#ifdef _DEBUG
	#pragma comment(lib, "DirectXTex/DirectXTex_debug.lib")
#else
	#pragma comment(lib, "DirectXTex/DirectXTex.lib")
#endif

// Managers
#include "Manager/Graphics.h"
