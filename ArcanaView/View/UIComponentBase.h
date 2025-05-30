#pragma once

class UIComponentBase
{
public:
	UIComponentBase(std::string title);
	virtual ~UIComponentBase();

public:
	virtual void Update() {}
	virtual void Render() {}

protected:
	virtual void Draw() abstract;

public:
	std::string GetTitle() { return _title; }
	bool* IsVisible() { return &_isVisible; }

protected:
	std::string _title = "";
	bool _isVisible = true;
};

