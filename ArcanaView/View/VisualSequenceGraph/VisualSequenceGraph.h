#pragma once
#include "../UIComponentBase.h"
#include "VSGType.h"

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
	void DrawBlueprintNode();
	void DrawTreeNode();

	void DrawLinks();
	void QueryCreateNode();
	void QueryDeleteNode();

	void DrawPopup();

private:
	Pin* FindPin(ed::PinId id);

private:
	bool IsPinLinked(ed::PinId id);
	bool CanCreateLink(Pin* input, Pin* output);

private:
	ed::EditorContext* _editorContext = nullptr;

private:
	std::vector<std::shared_ptr<Node>> _nodes;
	std::vector<std::shared_ptr<Link>> _links;

private:
	Pin* _newLinkPin = nullptr;
	bool _createNewNode = false;

	float _pinIconSize = 24.0f;
};

