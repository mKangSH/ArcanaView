#pragma once
#include "../UIComponentBase.h"
#include "VSGType.h"
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

private:
	void Draw();
	void Cleanup();

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
	std::vector<std::shared_ptr<Node>> _nodes;
	std::vector<std::shared_ptr<Link>> _links;

private:
	Pin* _newLinkPin = nullptr;
	bool _createNewNode = false;

	float _pinIconSize = 24.0f;

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
