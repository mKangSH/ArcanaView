#include "pch.h"
#include "ImGuiDemoView.h"

ImGuiDemoView::ImGuiDemoView() : Super("ImGui Demo Window")
{
}

ImGuiDemoView::~ImGuiDemoView()
{
}

void ImGuiDemoView::Update()
{
	if (_isVisible)
	{
		Draw();
	}
}

void ImGuiDemoView::Render()
{
}

void ImGuiDemoView::Draw()
{
	ImGui::ShowDemoWindow(&_isVisible);
}
