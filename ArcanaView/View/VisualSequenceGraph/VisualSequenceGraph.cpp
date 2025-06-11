#include "pch.h"
#include "VisualSequenceGraph.h"
#include "NodeGenerator.h"
#include "../../Utility/ImGuiNodeEditor/BlueprintUtility/builders.h"
#include "../../Utility/ImGuiNodeEditor/BlueprintUtility/drawing.h"
#include "../../Utility/ImGuiNodeEditor/BlueprintUtility/widgets.h"
#include <imgui_internal.h>
#include <imgui_node_editor_internal.h>
#include "VSGUtil.h"

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

        ImGui::ArrowButton("Start", ImGuiDir_Right); ImGui::SameLine();

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

    DrawLinks();

    DrawPopup();

	ed::End();

	ed::SetCurrentEditor(nullptr);
}

void VisualSequenceGraph::Cleanup()
{

}

void VisualSequenceGraph::DrawNodes()
{
    DrawBlueprintNode();
    
    DrawTreeNode();
}

void VisualSequenceGraph::DrawBlueprintNode()
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
                    {
                        continue;
                    }
                    
                    auto alpha = ImGui::GetStyle().Alpha;
                    if (_newLinkPin && !CanCreateLink(_newLinkPin, &output) && &output != _newLinkPin)
                    {
                        alpha = alpha * (48.0f / 255.0f);
                    }
                    
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

                    ax::Drawing::IconType iconType = VSGUtil::GetPinIconType(output);
                    ImColor color = VSGUtil::GetIconColor(output.Type);
                    alpha *= 255;
                    color.Value.w = alpha / 255.0f;
                    ax::Widgets::Icon(ImVec2(_pinIconSize, _pinIconSize), iconType, IsPinLinked(output.ID), color, ImColor(32, 32, 32, static_cast<int>(alpha)));

                    ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
                    ImGui::EndHorizontal();
                    ImGui::PopStyleVar();
                    ed::EndPin();
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
            if (_newLinkPin && !CanCreateLink(_newLinkPin, &input) && &input != _newLinkPin)
            {
                alpha = alpha * (48.0f / 255.0f);
            }
            
            builder.Input(input.ID);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);

            ax::Drawing::IconType iconType = VSGUtil::GetPinIconType(input);
            ImColor color = VSGUtil::GetIconColor(input.Type);
            alpha *= 255;
            color.Value.w = alpha / 255.0f;
            ax::Widgets::Icon(ImVec2(_pinIconSize, _pinIconSize), iconType, IsPinLinked(input.ID), color, ImColor(32, 32, 32, static_cast<int>(alpha)));

            ImGui::Spring(0);
            if (!input.Name.empty())
            {
                ImGui::TextUnformatted(input.Name.c_str());
                ImGui::Spring(0);
            }
            if (input.Type == PinType::Bool)
            {
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
            {
                continue;
            }
            
            auto alpha = ImGui::GetStyle().Alpha;
            if (_newLinkPin && !CanCreateLink(_newLinkPin, &output) && &output != _newLinkPin)
            {
                alpha = alpha * (48.0f / 255.0f);
            }
            
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
            if (output.Name.empty() == false)
            {
                ImGui::Spring(0);
                ImGui::TextUnformatted(output.Name.c_str());
            }
            ImGui::Spring(0);

            ax::Drawing::IconType iconType = VSGUtil::GetPinIconType(output);
            ImColor color = VSGUtil::GetIconColor(output.Type);
            alpha *= 255;
            color.Value.w = alpha / 255.0f;
            ax::Widgets::Icon(ImVec2(_pinIconSize, _pinIconSize), iconType, IsPinLinked(output.ID), color, ImColor(32, 32, 32, static_cast<int>(alpha)));
            
            ImGui::PopStyleVar();
            builder.EndOutput();
        }

        builder.End();
    }
}

void VisualSequenceGraph::DrawTreeNode()
{
    namespace detail = ax::NodeEditor::Detail;

    for (auto& node : _nodes)
    {
        if (node->Type != NodeType::Tree)
            continue;

        const float rounding = 5.0f;
        const float padding = 12.0f;

        const auto pinBackground = ed::GetStyle().Colors[ed::StyleColor_NodeBg];

        ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(128, 128, 128, 200));
        ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(32, 32, 32, 200));
        ed::PushStyleColor(ed::StyleColor_PinRect, ImColor(60, 180, 255, 150));
        ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor(60, 180, 255, 150));

        ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(0, 0, 0, 0));
        ed::PushStyleVar(ed::StyleVar_NodeRounding, rounding);
        ed::PushStyleVar(ed::StyleVar_SourceDirection, ImVec2(0.0f, 1.0f));
        ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
        ed::PushStyleVar(ed::StyleVar_LinkStrength, 0.0f);
        ed::PushStyleVar(ed::StyleVar_PinBorderWidth, 1.0f);
        ed::PushStyleVar(ed::StyleVar_PinRadius, 5.0f);
        ed::BeginNode(node->ID);

        ImGui::BeginVertical(node->ID.AsPointer());
        ImGui::BeginHorizontal("inputs");
        ImGui::Spring(0, padding * 2);

        ImRect inputsRect;
        int inputAlpha = 200;
        if (node->Inputs.empty() == false)
        {
            auto& pin = node->Inputs[0];
            ImGui::Dummy(ImVec2(0, padding));
            ImGui::Spring(1, 0);
            inputsRect = detail::ImGui_GetItemRect();

            ed::PushStyleVar(ed::StyleVar_PinArrowSize, 10.0f);
            ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 10.0f);
#if IMGUI_VERSION_NUM > 18101
            ed::PushStyleVar(ed::StyleVar_PinCorners, ImDrawFlags_RoundCornersBottom);
#else
            ed::PushStyleVar(ed::StyleVar_PinCorners, 12);
#endif
            ed::BeginPin(pin.ID, ed::PinKind::Input);
            ed::PinPivotRect(inputsRect.GetTL(), inputsRect.GetBR());
            ed::PinRect(inputsRect.GetTL(), inputsRect.GetBR());
            ed::EndPin();
            ed::PopStyleVar(3);

            if (_newLinkPin && !CanCreateLink(_newLinkPin, &pin) && &pin != _newLinkPin)
            {
                inputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
            }
        }
        else
        {
            ImGui::Dummy(ImVec2(0, padding));
        }

        ImGui::Spring(0, padding * 2);
        ImGui::EndHorizontal();

        ImGui::BeginHorizontal("content_frame");
        ImGui::Spring(1, padding);

        ImGui::BeginVertical("content", ImVec2(0.0f, 0.0f));
        ImGui::Dummy(ImVec2(160, 0));
        ImGui::Spring(1);
        ImGui::TextUnformatted(node->Name.c_str());
        ImGui::Spring(1);
        ImGui::EndVertical();
        auto contentRect = detail::ImGui_GetItemRect();

        ImGui::Spring(1, padding);
        ImGui::EndHorizontal();

        ImGui::BeginHorizontal("outputs");
        ImGui::Spring(0, padding * 2);

        ImRect outputsRect;
        int outputAlpha = 200;
        if (node->Outputs.empty() == false)
        {
            auto& pin = node->Outputs[0];
            ImGui::Dummy(ImVec2(0, padding));
            ImGui::Spring(1, 0);
            outputsRect = detail::ImGui_GetItemRect();

#if IMGUI_VERSION_NUM > 18101
            ed::PushStyleVar(ed::StyleVar_PinCorners, ImDrawFlags_RoundCornersTop);
#else
            ed::PushStyleVar(ed::StyleVar_PinCorners, 3);
#endif
            ed::BeginPin(pin.ID, ed::PinKind::Output);
            ed::PinPivotRect(outputsRect.GetTL(), outputsRect.GetBR());
            ed::PinRect(outputsRect.GetTL(), outputsRect.GetBR());
            ed::EndPin();
            ed::PopStyleVar();

            if (_newLinkPin && !CanCreateLink(_newLinkPin, &pin) && &pin != _newLinkPin)
            {
                outputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
            }
        }
        else
        {
            ImGui::Dummy(ImVec2(0, padding));
        }
        
        ImGui::Spring(0, padding * 2);
        ImGui::EndHorizontal();

        ImGui::EndVertical();

        ed::EndNode();
        ed::PopStyleVar(7);
        ed::PopStyleColor(4);

        auto drawList = ed::GetNodeBackgroundDrawList(node->ID);

        //const auto fringeScale = ImGui::GetStyle().AntiAliasFringeScale;
        //const auto unitSize    = 1.0f / fringeScale;

        //const auto ImDrawList_AddRect = [](ImDrawList* drawList, const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners, float thickness)
        //{
        //    if ((col >> 24) == 0)
        //        return;
        //    drawList->PathRect(a, b, rounding, rounding_corners);
        //    drawList->PathStroke(col, true, thickness);
        //};

#if IMGUI_VERSION_NUM > 18101
        const auto    topRoundCornersFlags = ImDrawFlags_RoundCornersTop;
        const auto bottomRoundCornersFlags = ImDrawFlags_RoundCornersBottom;
#else
        const auto    topRoundCornersFlags = 1 | 2;
        const auto bottomRoundCornersFlags = 4 | 8;
#endif

        drawList->AddRectFilled(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
            IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, bottomRoundCornersFlags);
        //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
        drawList->AddRect(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
            IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, bottomRoundCornersFlags);
        //ImGui::PopStyleVar();
        drawList->AddRectFilled(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
            IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, topRoundCornersFlags);
        //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
        drawList->AddRect(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
            IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, topRoundCornersFlags);
        //ImGui::PopStyleVar();
        drawList->AddRectFilled(contentRect.GetTL(), contentRect.GetBR(), IM_COL32(24, 64, 128, 200), 0.0f);
        //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
        drawList->AddRect(contentRect.GetTL(), contentRect.GetBR(), IM_COL32(48, 128, 255, 100), 0.0f);
        //ImGui::PopStyleVar();
    }
}

void VisualSequenceGraph::DrawLinks()
{
    for (auto& link : _links)
    {
        ed::Link(link->ID, link->InputPinID, link->OutputPinID, link->Color, 2.0f);
    }

    if (_createNewNode == false)
    {
        if (ed::BeginCreate(ImColor(255, 255, 255), 2.0f))
        {
            QueryCreateNode();
        }
        else
        {
            _newLinkPin = nullptr;
        }
        ed::EndCreate();

        if (ed::BeginDelete())
        {
            QueryDeleteNode();
        }
        ed::EndDelete();
    }
}

void VisualSequenceGraph::QueryCreateNode()
{
    ed::PinId startPinId = 0;
    ed::PinId endPinId = 0;

    // TODO : 중복 링크 방지
    if (ed::QueryNewLink(&startPinId, &endPinId))
    {
        Pin* startPin = FindPin(startPinId);
        Pin* endPin = FindPin(endPinId);

        _newLinkPin = startPin ? startPin : endPin;

        if (startPin && endPin)
        {
            if (startPin->Kind == PinKind::Input)
            {
                std::swap(startPin, endPin);
                std::swap(startPinId, endPinId);
            }

            if (endPin == startPin)
            {
                ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
            }
            else if (endPin->Kind == startPin->Kind)
            {
                ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
            }
            else if (endPin->Node.lock() == startPin->Node.lock())
            {
                ed::RejectNewItem(ImColor(255, 0, 0), 1.0f);
            }
            else if (endPin->Type != startPin->Type)
            {
                ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
            }
            else
            {
                if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
                {
                    _links.emplace_back(std::make_shared<Link>(Link::GetNextId(), startPinId, endPinId));
                    _links.back()->Color = VSGUtil::GetIconColor(startPin->Type);
                }
            }
        }
    }

    ed::PinId pinId = 0;
    if (ed::QueryNewNode(&pinId))
    {
        _newLinkPin = FindPin(pinId);

        if (ed::AcceptNewItem())
        {
            _createNewNode = true;
            Pin* newNodeLinkPin = FindPin(pinId);
            _newLinkPin = nullptr;
            ed::Suspend();
            ImGui::OpenPopup("Create New Node");
            ed::Resume();
        }
    }
}

void VisualSequenceGraph::QueryDeleteNode()
{
    ed::NodeId nodeId = 0;
    while (ed::QueryDeletedNode(&nodeId))
    {
        if (ed::AcceptDeletedItem())
        {
            auto it = std::find_if(_nodes.begin(), _nodes.end(), [nodeId](const std::shared_ptr<Node>& node) { return node->ID == nodeId; });
            if (it != _nodes.end())
            {
                _nodes.erase(it);
            }
        }
    }

    ed::LinkId linkId = 0;
    while (ed::QueryDeletedLink(&linkId))
    {
        if (ed::AcceptDeletedItem())
        {
            auto it = std::find_if(_links.begin(), _links.end(), [linkId](const std::shared_ptr<Link>& link) { return link->ID == linkId; });
            if (it != _links.end())
            {
                _links.erase(it);
            }
        }
    }
}

void VisualSequenceGraph::DrawPopup()
{
    auto openPopupPosition = ImGui::GetMousePos();

    ed::Suspend();
    if (ed::ShowBackgroundContextMenu())
    {
        ImGui::OpenPopup("Create New Node");
    }
    ed::Resume();

    ed::Suspend();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    if (ImGui::BeginPopup("Create New Node"))
    {
        auto newNodePostion = openPopupPosition;
        //ImGui::SetCursorScreenPos(ImGui::GetMousePosOnOpeningCurrentPopup());

        //auto drawList = ImGui::GetWindowDrawList();
        //drawList->AddCircleFilled(ImGui::GetMousePosOnOpeningCurrentPopup(), 10.0f, 0xFFFF00FF);

        // TODO : Instantiate
        std::shared_ptr<Node> node = nullptr;
        if (ImGui::MenuItem("Input Action"))
        {
            node = NodeGenerator::GenerateInputActionNode();
        }
            
        if (ImGui::MenuItem("Output Action"))
        {
            node = NodeGenerator::GenerateOutputActionNode();
        }
            
        if (ImGui::MenuItem("Branch"))
        {
            node = NodeGenerator::GenerateBranchNode();
        }
            
        if (ImGui::MenuItem("Do N"))
        {
            node = NodeGenerator::GenerateDoNNode();
        }
            
        if (ImGui::MenuItem("Set Timer"))
        {
            node = NodeGenerator::GenerateSetTimerNode();
        }
            
        if (ImGui::MenuItem("Less"))
        {
            node = NodeGenerator::GenerateLessNode();
        }
            
        if (ImGui::MenuItem("Weird"))
        {
            node = NodeGenerator::GenerateWeirdNode();
        }
            
        if (ImGui::MenuItem("Trace by Channel"))
        {
            node = NodeGenerator::GenerateTraceByChannelNode();
        }
            
        if (ImGui::MenuItem("Print String"))
        {
            node = NodeGenerator::GeneratePrintStringNode();
        }
            
        ImGui::Separator();

        if (ImGui::MenuItem("Comment"))
        {
            node = NodeGenerator::GenerateComment();
        }
            
        ImGui::Separator();

        if (ImGui::MenuItem("Sequence"))
        {
            node = NodeGenerator::GenerateTreeSequenceNode();
        }
            
        if (ImGui::MenuItem("Move To"))
        {
            node = NodeGenerator::GenerateTreeTaskNode();
        }
            
        if (ImGui::MenuItem("Random Wait"))
        {
            node = NodeGenerator::GenerateTreeTask2Node();
        }
            
        ImGui::Separator();

        if (ImGui::MenuItem("Message"))
        {
            node = NodeGenerator::GenerateMessageNode();
        }
            
        ImGui::Separator();

        if (ImGui::MenuItem("Transform"))
        {
            node = NodeGenerator::GenerateHoudiniTransformNode();
        }
            
        if (ImGui::MenuItem("Group"))
        {
            node = NodeGenerator::GenerateHoudiniGroupNode();
        }

        if (node)
        {
            NodeGenerator::BuildNode(node);

            _createNewNode = false;

            ed::SetNodePosition(node->ID, newNodePostion);

            _nodes.push_back(node);
        }

        ImGui::EndPopup();
    }
    else
    {
        _createNewNode = false;
    }

    ImGui::PopStyleVar();
    ed::Resume();
}

Pin* VisualSequenceGraph::FindPin(ed::PinId id)
{
    if (id == ed::PinId::Invalid)
    {
        return nullptr;
    }

    for (auto& node : _nodes)
    {
        for (auto& pin : node->Inputs)
        {
            if (pin.ID == id)
            {
                return &pin;
            }
        }

        for (auto& pin : node->Outputs)
        {
            if (pin.ID == id)
            {
                return &pin;
            }
        }
    }
}

bool VisualSequenceGraph::IsPinLinked(ed::PinId id)
{
    if (id == ed::PinId::Invalid)
    {
        return false;
    }
    
    for (auto& link : _links)
    {
        if (link->InputPinID == id || link->OutputPinID == id)
        {
            return true;
        }
    }
    
    return false;
}

bool VisualSequenceGraph::CanCreateLink(Pin* input, Pin* output)
{
    if (!input || !output ||
        (input == output) ||
        (input->Kind == output->Kind) ||
        (input->Type != output->Type) ||
        (input->Node.lock() == output->Node.lock()))
    {
        return false;
    }
    
    return true;
}
