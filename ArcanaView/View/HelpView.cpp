#include "pch.h"
#include "HelpView.h"

HelpView::HelpView() : Super("Help View")
{
	_windowFlags = ImGuiWindowFlags_None;
}

HelpView::~HelpView()
{

}

void HelpView::Update()
{
	if (_isVisible)
	{
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));

		ImGui::Begin(_title.c_str(), &_isVisible, _windowFlags);

		Draw();

		ImGui::End();

		ImGui::PopStyleColor();
	}
}

void HelpView::Render()
{

}

void HelpView::Draw()
{
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImVec2 textPos = ImGui::GetCursorScreenPos();

	drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 1.4, textPos, IM_COL32(0, 255, 255, 255), "Please drag and drop the image file onto the current viewport.");
}

void HelpView::Cleanup()
{

}