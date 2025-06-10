#include "pch.h"
#include "VisualSequenceGraph.h"
#include "NodeGenerator.h"
#include "../../Utility/ImGuiNodeEditor/BlueprintUtility/builders.h"
#include "../../Utility/ImGuiNodeEditor/BlueprintUtility/drawing.h"
#include "../../Utility/ImGuiNodeEditor/BlueprintUtility/widgets.h"

VisualSequenceGraph::VisualSequenceGraph() : Super("Visual Scene Graph")
{
	using namespace std;

	ed::Config config;
	config.SettingsFile = "VisualSequenceGraph.json";
	config.UserPointer = this;

	_editorContext = ed::CreateEditor(&config);
	ed::SetCurrentEditor(_editorContext);

	shared_ptr<Node> node = NodeGenerator::GenerateInputActionNode();
	_nodes.push_back(node); ed::SetNodePosition(node->ID, ImVec2(-252, 220));

	shared_ptr<Node> node2 = NodeGenerator::GenerateDoNNode();
	_nodes.push_back(node2); ed::SetNodePosition(node2->ID, ImVec2(-300, 220));

	ed::NavigateToContent();
}

VisualSequenceGraph::~VisualSequenceGraph()
{
	ed::DestroyEditor(_editorContext);
	_editorContext = nullptr;
}

void VisualSequenceGraph::Update()
{
	if (_isVisible)
	{
		ImGui::Begin(_title.c_str(), &_isVisible);

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

	DrawNodes();

	ed::End();

	ed::SetCurrentEditor(nullptr);
}

void VisualSequenceGraph::Cleanup()
{

}

void VisualSequenceGraph::DrawNodes()
{
    namespace util = ax::NodeEditor::Utilities;

    util::BlueprintNodeBuilder builder;

    for (auto& node : _nodes)
    {
        if (node->Type != NodeType::Blueprint && node->Type != NodeType::Simple)
        {
            continue;
        }
        
        const auto isSimple = (node->Type == NodeType::Simple);

        bool hasOutputDelegates = false;
        for (auto& output : node->Outputs)
        {
            if (output.Type == PinType::Delegate)
            {
                hasOutputDelegates = true;
            }
        }

        builder.Begin(node->ID);

        if (isSimple == false)
        {
            builder.Header(node->Color);
            ImGui::Spring(0);
            ImGui::TextUnformatted(node->Name.c_str());
            ImGui::Spring(1);
            ImGui::Dummy(ImVec2(0, 28));

            if (hasOutputDelegates)
            {
                ImGui::BeginVertical("delegates", ImVec2(0, 28));
                ImGui::Spring(1, 0);
                for (auto& output : node->Outputs)
                {
                    if (output.Type != PinType::Delegate)
                        continue;

                    auto alpha = ImGui::GetStyle().Alpha;

                    ed::BeginPin(output.ID, ed::PinKind::Output);
                    ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
                    ed::PinPivotSize(ImVec2(0, 0));
                    ImGui::BeginHorizontal(output.ID.AsPointer());
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
                    if (!output.Name.empty())
                    {
                        ImGui::TextUnformatted(output.Name.c_str());
                        ImGui::Spring(0);
                    }
                    DrawPinIcon(output, IsPinLinked(output.ID), (int)(alpha * 255));
                    ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
                    ImGui::EndHorizontal();
                    ImGui::PopStyleVar();
                    ed::EndPin();

                    //DrawItemRect(ImColor(255, 0, 0));
                }
                ImGui::Spring(1, 0);
                ImGui::EndVertical();
                ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
            }
            else
            {
                ImGui::Spring(0);
            }
            
            builder.EndHeader();
        }

        for (auto& input : node->Inputs)
        {
            auto alpha = ImGui::GetStyle().Alpha;

            builder.Input(input.ID);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
            DrawPinIcon(input, IsPinLinked(input.ID), (int)(alpha * 255));
            ImGui::Spring(0);
            if (!input.Name.empty())
            {
                ImGui::TextUnformatted(input.Name.c_str());
                ImGui::Spring(0);
            }
            if (input.Type == PinType::Bool)
            {
                ImGui::Button("Hello");
                ImGui::Spring(0);
            }
            ImGui::PopStyleVar();
            builder.EndInput();
        }

        if (isSimple)
        {
            builder.Middle();

            ImGui::Spring(1, 0);
            ImGui::TextUnformatted(node->Name.c_str());
            ImGui::Spring(1, 0);
        }

        for (auto& output : node->Outputs)
        {
            if (isSimple == false && output.Type == PinType::Delegate)
                continue;

            auto alpha = ImGui::GetStyle().Alpha;

            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
            builder.Output(output.ID);
            if (output.Type == PinType::String)
            {
                static char buffer[128] = "Edit Me\nMultiline!";
                static bool wasActive = false;

                ImGui::PushItemWidth(100.0f);
                ImGui::InputText("##edit", buffer, 127);
                ImGui::PopItemWidth();
                if (ImGui::IsItemActive() && !wasActive)
                {
                    ed::EnableShortcuts(false);
                    wasActive = true;
                }
                else if (!ImGui::IsItemActive() && wasActive)
                {
                    ed::EnableShortcuts(true);
                    wasActive = false;
                }
                ImGui::Spring(0);
            }
            if (!output.Name.empty())
            {
                ImGui::Spring(0);
                ImGui::TextUnformatted(output.Name.c_str());
            }
            ImGui::Spring(0);
            DrawPinIcon(output, IsPinLinked(output.ID), (int)(alpha * 255));
            ImGui::PopStyleVar();
            builder.EndOutput();
        }

        builder.End();
    }

    for (auto& link : _links)
    {
        ed::Link(link->ID, link->InputPinID, link->OutputPinID, link->Color, 2.0f);
    }
    
    if (ed::BeginCreate(ImColor(255, 255, 255), 2.0f))
    {
    }

    ed::EndCreate();
}

ImColor VisualSequenceGraph::GetIconColor(PinType type)
{
    switch (type)
    {
        case PinType::Flow:     return ImColor(255, 255, 255);
        case PinType::Bool:     return ImColor(220, 48, 48);
        case PinType::Int:      return ImColor(68, 201, 156);
        case PinType::Float:    return ImColor(147, 226, 74);
        case PinType::String:   return ImColor(124, 21, 153);
        case PinType::Object:   return ImColor(51, 150, 215);
        case PinType::Function: return ImColor(218, 0, 183);
        case PinType::Delegate: return ImColor(255, 48, 48);
        default:                return ImColor(255, 255, 255);
    }
}

void VisualSequenceGraph::DrawPinIcon(const Pin& pin, bool connected, int alpha)
{
    using namespace ax::Drawing;
    IconType iconType;
    ImColor  color = GetIconColor(pin.Type);
    color.Value.w = alpha / 255.0f;
    switch (pin.Type)
    {
        case PinType::Flow:     iconType = IconType::Flow;   break;
        case PinType::Bool:     iconType = IconType::Circle; break;
        case PinType::Int:      iconType = IconType::Circle; break;
        case PinType::Float:    iconType = IconType::Circle; break;
        case PinType::String:   iconType = IconType::Circle; break;
        case PinType::Object:   iconType = IconType::Circle; break;
        case PinType::Function: iconType = IconType::Circle; break;
        case PinType::Delegate: iconType = IconType::Square; break;
        default:
            return;
    }

    ax::Widgets::Icon(ImVec2(24.0f, 24.0f), iconType, connected, color, ImColor(32, 32, 32, alpha));
}

bool VisualSequenceGraph::IsPinLinked(ed::PinId id)
{
    if (!id)
        return false;

    for (auto& link : _links)
    {
        if (link->InputPinID == id || link->OutputPinID == id)
        {
            return true;
        }
    }
    
    return false;
}
