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
	Node beginNode = { 1, 100, 101 };        // Begin(��� �ɸ�)
	Node funcNode = { 2, 201, 202 };    // Function(�Է�/��� ��)

	Link link = { 1, 101, 201 }; // Begin�� ��� �� �� Function�� �Է� ��

	ed::EditorContext* _editorContext = nullptr;

private:
	void ExecuteFromBegin(const Node& beginNode, const std::vector<Node>& nodes, const std::vector<Link>& links);
	void CallMyFunction();
};

