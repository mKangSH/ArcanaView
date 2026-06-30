#pragma once

class UIComponentBase;
class VisualSequenceGraph;

class LayoutManager
{
	DECLARE_SINGLETON(LayoutManager)

public:
	void Init();
	void Update();
	void Render();
	void Cleanup();

	void AddImageView(const std::wstring& imageFile);

	// ImageView -> 그래프 브리지: 현재 이미지를 VSG 의 Image Source 노드로 승격한다.
	void PromoteImageToGraph(const DirectX::ScratchImage& image);

public:
	std::vector<UIComponentBase*> GetUIComponents() const { return _uiComponents; }

private:
	std::vector<UIComponentBase*> _uiComponents;
	std::unordered_map<std::string, UIComponentBase*> _imageViewComponents;

	// 브리지 대상 (Init 에서 캐시). 단일 VSG 가정. 소유하지 않는 별칭(_uiComponents 가 소유).
	VisualSequenceGraph* _visualSequenceGraph = nullptr;

	void ConstructLayout();
};

