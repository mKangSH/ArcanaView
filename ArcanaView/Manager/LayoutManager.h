#pragma once

class UIComponentBase;

class LayoutManager
{
public:
	void Init();
	void Update();
	void Render();
	void Cleanup();

public:
	std::vector<std::shared_ptr<UIComponentBase>> GetUIComponents() const { return _uiComponents; }

private:
	std::vector<std::shared_ptr<UIComponentBase>> _uiComponents;

	void ConstructLayout();
};

