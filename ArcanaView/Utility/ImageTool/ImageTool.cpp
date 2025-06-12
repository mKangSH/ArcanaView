#include "pch.h"
#include "ImageTool.h"

void ImageTool::Update(ImDrawList* drawList)
{
	switch(_type)
	{
		case ImageToolType::Line:
		{
			DrawLine(drawList);
		}
		break;

		case ImageToolType::Rectangle:
		{
			DrawRectangle(drawList);
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

void ImageTool::DrawLine(ImDrawList* drawList)
{
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 size = ImGui::GetContentRegionAvail();
	if (size.x <= 0 || size.y <= 0)
	{
		return;
	}

	uint8 colorR = static_cast<uint8>(*(_color + 0) * 255);
	uint8 colorG = static_cast<uint8>(*(_color + 1) * 255);
	uint8 colorB = static_cast<uint8>(*(_color + 2) * 255);
	uint8 colorA = static_cast<uint8>(*(_color + 3) * 255);
	ImU32 color = IM_COL32(colorR, colorG, colorB, colorA);
	
	if (ImGui::IsWindowHovered())
	{
		ImVec2 mousePos = ImGui::GetMousePos();
		if (pos.x > mousePos.x || pos.y > mousePos.y || pos.x + size.x < mousePos.x || pos.y + size.y < mousePos.y)
		{
			ImVec2 lineStart = ImVec2(_startPosNormalized.x * size.x, _startPosNormalized.y * size.y);
			lineStart.x += pos.x;
			lineStart.y += pos.y;

			ImVec2 lineEnd = ImVec2(_endPosNormalized.x * size.x, _endPosNormalized.y * size.y);
			lineEnd.x += pos.x;
			lineEnd.y += pos.y;

			drawList->AddLine(lineStart, lineEnd, color, 1.0f);
			return;
		}

		ImVec2 mouseRelativePos = ImVec2(mousePos.x - pos.x, mousePos.y - pos.y);
		ImVec2 mouseRelativePosNormalized = ImVec2(mouseRelativePos.x / size.x, mouseRelativePos.y / size.y);
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			_startPosNormalized = mouseRelativePosNormalized;
			_endPosNormalized = _startPosNormalized;
		}
		else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			_endPosNormalized = mouseRelativePosNormalized;
		}
		else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			_endPosNormalized = mouseRelativePosNormalized;
		}
	}

	ImVec2 lineStart = ImVec2(_startPosNormalized.x * size.x, _startPosNormalized.y * size.y);
	lineStart.x += pos.x;
	lineStart.y += pos.y;

	ImVec2 lineEnd = ImVec2(_endPosNormalized.x * size.x, _endPosNormalized.y * size.y);
	lineEnd.x += pos.x;
	lineEnd.y += pos.y;

	drawList->AddLine(lineStart, lineEnd, color, 1.0f);
}

void ImageTool::DrawRectangle(ImDrawList* drawList)
{
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 size = ImGui::GetContentRegionAvail();
	if (size.x <= 0 || size.y <= 0)
	{
		return;
	}

	uint8 colorR = static_cast<uint8>(*(_color + 0) * 255);
	uint8 colorG = static_cast<uint8>(*(_color + 1) * 255);
	uint8 colorB = static_cast<uint8>(*(_color + 2) * 255);
	uint8 colorA = static_cast<uint8>(*(_color + 3) * 255);
	ImU32 color = IM_COL32(colorR, colorG, colorB, colorA);

	if (ImGui::IsWindowHovered())
	{
		ImVec2 mousePos = ImGui::GetMousePos();
		if (pos.x > mousePos.x || pos.y > mousePos.y || pos.x + size.x < mousePos.x || pos.y + size.y < mousePos.y)
		{
			ImVec2 rectStart = ImVec2(_startPosNormalized.x * size.x, _startPosNormalized.y * size.y);
			rectStart.x += pos.x;
			rectStart.y += pos.y;

			ImVec2 rectEnd = ImVec2(_endPosNormalized.x * size.x, _endPosNormalized.y * size.y);
			rectEnd.x += pos.x;
			rectEnd.y += pos.y;

			drawList->AddRect(rectStart, rectEnd, color, 1.0f);
			return;
		}

		ImVec2 mouseRelativePos = ImVec2(mousePos.x - pos.x, mousePos.y - pos.y);
		ImVec2 mouseRelativePosNormalized = ImVec2(mouseRelativePos.x / size.x, mouseRelativePos.y / size.y);
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			_startPosNormalized = mouseRelativePosNormalized;
			_endPosNormalized = _startPosNormalized;
		}
		else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			_endPosNormalized = mouseRelativePosNormalized;
		}
		else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			_endPosNormalized = mouseRelativePosNormalized;
		}
	}

	ImVec2 rectStart = ImVec2(_startPosNormalized.x * size.x, _startPosNormalized.y * size.y);
	rectStart.x += pos.x;
	rectStart.y += pos.y;

	ImVec2 rectEnd = ImVec2(_endPosNormalized.x * size.x, _endPosNormalized.y * size.y);
	rectEnd.x += pos.x;
	rectEnd.y += pos.y;

	drawList->AddRect(rectStart, rectEnd, color, 1.0f);
}
