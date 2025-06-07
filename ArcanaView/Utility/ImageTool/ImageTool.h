#pragma once

enum class ImageToolType
{
	None,
	Line,
	Rectangle,
	Brush,
	Eraser,
	Fill,
	Select,
	Crop,
	Resize
};

class ImageTool
{
public:
	ImageTool(ImageToolType type = ImageToolType::None) : _type(type) {}

	void SetType(ImageToolType type) { _type = type; }
	ImageToolType& GetType() { return _type; }
	
public:
	bool IsNone() const { return _type == ImageToolType::None; }
	bool IsLine() const { return _type == ImageToolType::Line; }
	bool IsRectangle() const { return _type == ImageToolType::Rectangle; }
	bool IsBrush() const { return _type == ImageToolType::Brush; }
	bool IsEraser() const { return _type == ImageToolType::Eraser; }
	bool IsFill() const { return _type == ImageToolType::Fill; }
	bool IsSelect() const { return _type == ImageToolType::Select; }
	bool IsCrop() const { return _type == ImageToolType::Crop; }
	bool IsResize() const { return _type == ImageToolType::Resize; }

public:
	void Update(ImDrawList* drawList);

private:
	ImageToolType _type;
	ImVec2 _startPos = ImVec2(0.0f, 0.0f);
	ImVec2 _endPos = ImVec2(0.0f, 0.0f);
};

