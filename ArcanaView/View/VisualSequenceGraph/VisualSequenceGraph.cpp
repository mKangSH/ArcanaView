#include "pch.h"
#include "VisualSequenceGraph.h"

VisualSequenceGraph::VisualSequenceGraph() : Super("Visual Scene Graph")
{
	ed::Config config;
	config.SettingsFile = "VisualSequenceGraph.json";
	config.UserPointer = this;

	_editorContext = ed::CreateEditor(&config);
}

VisualSequenceGraph::~VisualSequenceGraph()
{
	ed::DestroyEditor(_editorContext);
}

void VisualSequenceGraph::Update()
{
	if (_isVisible)
	{
		ImGui::Begin(_title.c_str(), &_isVisible);

		if (ImGui::Button("Begin 실행"))
		{
			ExecuteFromBegin(beginNode, { funcNode }, { link });
		}

		Draw();

		ImGui::End();
	}
}

void VisualSequenceGraph::Render()
{
	
}

void VisualSequenceGraph::Draw()
{
	ed::SetCurrentEditor(_editorContext);

	ed::Begin(_title.c_str(), ImVec2(0.0, 0.0f));

	ed::BeginNode(beginNode.ID);
	{
		ImGui::Text("Begin");
		ed::BeginPin(beginNode.ExecutionInputPinID, ed::PinKind::Input);
		ImGui::Text("Exec In");
		ed::EndPin(); ImGui::SameLine();

		ed::BeginPin(beginNode.ExecutionOutputPinID, ed::PinKind::Output);
		ImGui::Text("Exec Out");
		ed::EndPin();
	}
	ed::EndNode();

	ed::BeginNode(funcNode.ID);
	{
		ImGui::Text("MyFunction"); 
		ed::BeginPin(funcNode.ExecutionInputPinID, ed::PinKind::Input);
		ImGui::Text("Exec In");
		ed::EndPin(); ImGui::SameLine();

		ed::BeginPin(funcNode.ExecutionOutputPinID, ed::PinKind::Output);
		ImGui::Text("Exec Out");
		ed::EndPin();
	}
	ed::EndNode();

	ed::Link(link.ID, link.InputID, link.OutputID);

	ed::End();

	ed::SetCurrentEditor(nullptr);
}

void VisualSequenceGraph::Cleanup()
{

}

void VisualSequenceGraph::ExecuteFromBegin(const Node& beginNode, const std::vector<Node>& nodes, const std::vector<Link>& links) 
{
	// Begin 노드의 출력 핀에서 연결된 링크 찾기
	auto it = std::find_if(links.begin(), links.end(),[&](const Link& l) { return l.InputID == beginNode.ExecutionOutputPinID; });
	if (it == links.end())
	{
		return;
	}

	ed::PinId nextPinId = it->OutputID;
	// 해당 핀을 가진 노드 찾기
	auto funcIt = std::find_if(nodes.begin(), nodes.end(), [&](const Node& n) { return n.ExecutionInputPinID == nextPinId; });
	if (funcIt == nodes.end())
	{
		return;
	}

	// 함수 노드 실행
	if (funcIt->ID == ax::NodeEditor::NodeId(2))
	{
		CallMyFunction();
	}
}

#include <chrono>
void VisualSequenceGraph::CallMyFunction() 
{
	// 원하는 동작
	link.InputID = 202;
	link.OutputID = 100;
}