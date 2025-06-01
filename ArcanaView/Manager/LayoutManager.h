#pragma once

class UIComponentBase;

class LayoutManager
{
	DECLARE_SINGLETON(LayoutManager)

public:
	void Init();
	void Update();
	void Render();
	void Cleanup();

	void AddImageView(const std::wstring& imageFile);

public:
	std::vector<std::shared_ptr<UIComponentBase>> GetUIComponents() const { return _uiComponents; }

private:
	std::vector<std::shared_ptr<UIComponentBase>> _uiComponents;
	std::unordered_map<std::string, std::shared_ptr<UIComponentBase>> _imageViewComponents;

	void ConstructLayout();
};

