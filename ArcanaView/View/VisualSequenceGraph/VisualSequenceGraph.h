#pragma once
#include "../UIComponentBase.h"
#include "VSGType.h"
#include "../../Utility/ImageProcess/GpuImageProcessor.h"
#include <imgui_internal.h>

class VisualSequenceGraph : public UIComponentBase
{
	using Super = UIComponentBase;

public:
	VisualSequenceGraph();
	virtual ~VisualSequenceGraph();

public:
	void Update() override;
	void Render() override;

public:
	// 외부(LayoutManager)에서 호출: 그래프에 Image Source 노드를 추가하고 GPU 업로드한다.
	void AddImageSourceNode(const DirectX::ScratchImage& image);

private:
	void Draw();
	void Cleanup();

private:
	// 평가(실행) 엔진
	void RunGraph();
	bool EvaluateNode(Node* node, std::unordered_map<uint64, int>& state); // 0=visiting, 1=done
	Node* FindNodeOfPin(ed::PinId pinId);
	Pin* FindSourcePinForInput(const Pin& inputPin);

	// 노드가 소유한(출력 핀) GpuImage 를 해제하고, 이를 빌려쓰던 입력 핀 참조를 무효화한다.
	void ReleaseNodeImages(Node* node);

private:
	void ShowToolbar();

private:
	void DrawNodes();
	void DrawBlueprintNodes();
	void DrawTreeNodes();
	void DrawHoudiniNodes();
	void DrawCommentNodes();

	void DrawLinks();
	void QueryCreateNode();
	void QueryDeleteNode();

	void DrawPopup();

private:
	ed::EditorContext* _editorContext = nullptr;

private:
	std::vector<Node*> _nodes;
	std::vector<Link*> _links;

private:
	Pin* _newLinkPin = nullptr;
	bool _createNewNode = false;

	float _pinIconSize = 24.0f;

private:
	GpuImageProcessor _processor;

private:
	inline ImRect ImRect_Expanded(const ImRect& rect, float x, float y);
};

inline ImRect VisualSequenceGraph::ImRect_Expanded(const ImRect& rect, float x, float y)
{
	auto result = rect;
	result.Min.x -= x;
	result.Min.y -= y;
	result.Max.x += x;
	result.Max.y += y;
	return result;
}
