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
	void DrawNodes();
	void DrawLinks();
	void DrawPopup();
	Pin* FindPin(ed::PinId id);

private:
	ImColor GetIconColor(PinType type);
	void DrawPinIcon(const Pin& pin, bool connected, int alpha);
	bool IsPinLinked(ed::PinId id);

private:
	ed::EditorContext* _editorContext = nullptr;

private:
	std::vector<std::shared_ptr<Node>> _nodes;
	std::vector<std::shared_ptr<Link>> _links;
};

