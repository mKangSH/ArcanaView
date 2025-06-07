#include "pch.h"
#include "ImageTool.h"

void ImageTool::Update(ImDrawList* drawList)
{
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 size = ImGui::GetContentRegionAvail();
	if (size.x <= 0 || size.y <= 0)
	{
		return;
	}

	switch(_type)
	{
		case ImageToolType::Line:
		{
			ImVec2 mousePos = ImGui::GetMousePos();
			if (pos.x > mousePos.x || pos.y > mousePos.y || pos.x + size.x < mousePos.x || pos.y + size.y < mousePos.y)
			{
				drawList->AddLine(_startPos, _endPos, IM_COL32(255, 255, 0, 255), 1.0f);
				return;
			}

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				_startPos = mousePos;
				_endPos = _startPos;
			}
			else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				_endPos = mousePos;
			}
			else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				_endPos = mousePos;
			}

			drawList->AddLine(_startPos, _endPos, IM_COL32(255, 255, 0, 255), 1.0f);
		}
		break;

		case ImageToolType::Rectangle:
		{
			ImVec2 mousePos = ImGui::GetMousePos();
			if (pos.x > mousePos.x || pos.y > mousePos.y || pos.x + size.x < mousePos.x || pos.y + size.y < mousePos.y)
			{
				drawList->AddRect(_startPos, _endPos, IM_COL32(255, 255, 0, 255), 1.0f);
				return;
			}

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				_startPos = mousePos;
				_endPos = _startPos;
			}
			else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				_endPos = mousePos;
			}
			else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				_endPos = mousePos;
			}

			drawList->AddRect(_startPos, _endPos, IM_COL32(255, 255, 0, 255), 1.0f);
		}
		break;

		case ImageToolType::Eraser:
		{
		}
		break;

		case ImageToolType::Brush:
		{
		}
		break;
	}
}
