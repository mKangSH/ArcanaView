#include "pch.h"
#include "VisualSequenceGraph.h"
#include "NodeGenerator.h"
#include "../../Utility/ImGuiNodeEditor/BlueprintUtility/builders.h"
#include "../../Utility/ImGuiNodeEditor/BlueprintUtility/drawing.h"
#include "../../Utility/ImGuiNodeEditor/BlueprintUtility/widgets.h"
#include <imgui_node_editor_internal.h>
#include "VSGUtil.h"

VisualSequenceGraph::VisualSequenceGraph() : Super("Visual Scene Graph")
{
	using namespace std;

	ed::Config config;
	config.SettingsFile = "VisualSequenceGraph.json";
	config.UserPointer = this;

    config.LoadNodeSettings = [](ed::NodeId nodeId, char* data, void* userPointer) -> size_t
        {
            auto self = static_cast<VisualSequenceGraph*>(userPointer);

            auto node = VSGUtil::FindNode(self->_nodes, nodeId);
            if (!node)
            {
                return 0;
            }
            
            if (data != nullptr)
            {
                memcpy(data, node->State.data(), node->State.size());
            }

            return node->State.size();
        };

    config.SaveNodeSettings = [](ed::NodeId nodeId, const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer) -> bool
        {
            auto self = static_cast<VisualSequenceGraph*>(userPointer);

            auto node = VSGUtil::FindNode(self->_nodes, nodeId);
            if (!node)
            {
                return false;
            }
            
            node->State.assign(data, size);

            //self->TouchNode(nodeId);

            return true;
        };

    
	_editorContext = ed::CreateEditor(&config);
	ed::SetCurrentEditor(_editorContext);
    
	Node* node = NodeGenerator::GenerateInputActionNode();
	_nodes.push_back(node); ed::SetNodePosition(node->ID, ImVec2(-252, 220));

	Node* node2 = NodeGenerator::GenerateDoNNode();
	_nodes.push_back(node2); ed::SetNodePosition(node2->ID, ImVec2(-300, 220));

	ed::NavigateToContent();
}

VisualSequenceGraph::~VisualSequenceGraph()
{
	// 노드가 소유한 출력 이미지 + 노드 + 링크를 모두 해제한다.
	for (Node* node : _nodes)
	{
		for (Pin& out : node->Outputs)
		{
			delete out.ImageValue;
			out.ImageValue = nullptr;
		}
		delete node;
	}
	_nodes.clear();

	for (Link* link : _links)
	{
		delete link;
	}
	_links.clear();

	ed::DestroyEditor(_editorContext);
	_editorContext = nullptr;
}

void VisualSequenceGraph::Update()
{
	if (_isVisible)
	{
        ed::SetCurrentEditor(_editorContext);

		ImGui::Begin(_title.c_str(), &_isVisible);
        
        ShowToolbar();

		Draw();

		ImGui::End();

        ed::SetCurrentEditor(nullptr);
	}
}

void VisualSequenceGraph::Render()
{
	
}

void VisualSequenceGraph::Draw()
{
    ed::Begin(_title.c_str(), ImVec2(0.0, 0.0f));

    auto cursorTopLeft = ImGui::GetCursorScreenPos();

	DrawNodes();

    DrawLinks();

    ImGui::SetCursorScreenPos(cursorTopLeft);

    DrawPopup();

    ed::End();
}

void VisualSequenceGraph::Cleanup()
{

}

// ---- 평가(실행) 엔진 --------------------------------------------------------

void VisualSequenceGraph::RunGraph()
{
    // ImageOutput(싱크)에서부터 pull-based 로 상류를 평가한다.
    // state 를 공유해 공통 상류 노드는 1회만 평가된다.
    std::unordered_map<uint64, int> state;

    for (auto& node : _nodes)
    {
        if (node->Op == NodeOp::ImageOutput)
        {
            EvaluateNode(node, state);
        }
    }
}

Node* VisualSequenceGraph::FindNodeOfPin(ed::PinId pinId)
{
    if (pinId == ed::PinId::Invalid)
    {
        return nullptr;
    }

    for (auto& node : _nodes)
    {
        for (auto& pin : node->Inputs)
        {
            if (pin.ID == pinId)
            {
                return node;
            }
        }

        for (auto& pin : node->Outputs)
        {
            if (pin.ID == pinId)
            {
                return node;
            }
        }
    }

    return nullptr;
}

Pin* VisualSequenceGraph::FindSourcePinForInput(const Pin& inputPin)
{
    // 링크 방향 반전 주의: 입력 핀에 연결된 소스(출력) 핀은
    // link->OutputPinID == inputPin.ID 인 링크의 link->InputPinID 이다.
    for (auto& link : _links)
    {
        if (link->OutputPinID == inputPin.ID)
        {
            return VSGUtil::FindPin(_nodes, link->InputPinID);
        }
    }

    return nullptr;
}

bool VisualSequenceGraph::EvaluateNode(Node* node, std::unordered_map<uint64, int>& state)
{
    const uint64 id = reinterpret_cast<uint64>(node->ID.AsPointer());

    auto it = state.find(id);
    if (it != state.end())
    {
        if (it->second == 1)
        {
            return true; // 이미 평가 완료
        }
        return false; // visiting 중 재방문 = 사이클 -> 출력 비움
    }
    state[id] = 0; // visiting

    // 입력 핀 해석: 소스 노드를 먼저 평가한 뒤 간선을 따라 값을 복사한다.
    // (평가 중 _nodes / 핀 벡터를 변경하지 않으므로 Pin* 는 유효)
    for (size_t i = 0; i < node->Inputs.size(); ++i)
    {
        Pin* srcPin = FindSourcePinForInput(node->Inputs[i]);
        if (srcPin == nullptr)
        {
            node->Inputs[i].ImageValue = nullptr; // 미연결
            continue;
        }

        Node* srcNode = FindNodeOfPin(srcPin->ID);
        if (srcNode)
        {
            EvaluateNode(srcNode, state);
        }

        node->Inputs[i].ImageValue = srcPin->ImageValue;
    }

    switch (node->Op)
    {
    case NodeOp::ImageSource:
        // 출력 핀 ImageValue 는 Promote 업로드 시 이미 채워짐 -> 통과
        break;

    case NodeOp::Grayscale:
    case NodeOp::Invert:
    {
        GpuImage* in = node->Inputs.empty() ? nullptr : node->Inputs[0].ImageValue; // 빌려쓰기 (소유 X)
        GpuImage* out = nullptr;

        if (in)
        {
            out = _processor.CreateTarget(in->Width, in->Height);
            if (out && _processor.Apply(node->Op, *in, *out) == false)
            {
                delete out;
                out = nullptr;
            }
        }

        if (node->Outputs.empty() == false)
        {
            // 이전 Run 의 결과(소유)는 새 결과로 덮어쓰기 전에 해제한다.
            if (node->Outputs[0].ImageValue != out)
            {
                delete node->Outputs[0].ImageValue;
            }
            node->Outputs[0].ImageValue = out; // null 이면 하류로 "데이터 없음" 전파
        }
        break;
    }

    case NodeOp::ImageOutput:
        // 입력[0].ImageValue 가 최종 결과. 미리보기는 DrawBlueprintNodes 에서 이 핀을 읽어 표시.
        break;

    default:
        break;
    }

    state[id] = 1; // done
    return true;
}

void VisualSequenceGraph::AddImageSourceNode(const DirectX::ScratchImage& image)
{
    GpuImage* gpu = _processor.UploadFromScratchImage(image);

    Node* node = NodeGenerator::GenerateImageSourceNode();
    if (node->Outputs.empty() == false)
    {
        node->Outputs[0].ImageValue = gpu;
    }

    _nodes.push_back(node);

    // 노드 위치 지정은 에디터 컨텍스트가 활성화된 상태여야 한다.
    // (이 함수는 VSG 자체 Update 밖에서 호출되므로 컨텍스트를 임시 설정)
    ed::SetCurrentEditor(_editorContext);
    const float stagger = 24.0f * static_cast<float>(_nodes.size());
    ed::SetNodePosition(node->ID, ImVec2(stagger, stagger));
    ed::SetCurrentEditor(nullptr);
}

void VisualSequenceGraph::ReleaseNodeImages(Node* node)
{
    // 이 노드가 소유한(출력 핀) GpuImage 를 해제한다.
    for (Pin& out : node->Outputs)
    {
        if (out.ImageValue == nullptr)
        {
            continue;
        }

        GpuImage* dead = out.ImageValue;

        // 다른 노드의 입력 핀이 이 포인터를 빌려쓰고 있었다면 무효화(댕글링 방지).
        for (Node* other : _nodes)
        {
            for (Pin& in : other->Inputs)
            {
                if (in.ImageValue == dead)
                {
                    in.ImageValue = nullptr;
                }
            }
        }

        out.ImageValue = nullptr;
        delete dead;
    }
}

void VisualSequenceGraph::ShowToolbar()
{
    ImGui::ArrowButton("Start", ImGuiDir_Right);
    ImGui::SameLine();

    if (ImGui::Button("Show Flow"))
    {
        for (auto& link : _links)
        {
            ed::Flow(link->ID);
        }
    }
    ImGui::SameLine();

    if (ImGui::Button("Zoom to Content"))
    {
        ed::NavigateToContent();
    }
    ImGui::SameLine();

    if (ImGui::Button("Run"))
    {
        RunGraph();
    }


    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z)))
    {
        for (auto& link : _links)
        {
            ed::Flow(link->ID);
        }
    }
}

void VisualSequenceGraph::DrawNodes()
{
    DrawBlueprintNodes();
    
    DrawTreeNodes();

    DrawHoudiniNodes();

    DrawCommentNodes();
}

void VisualSequenceGraph::DrawBlueprintNodes()
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
                    if (_newLinkPin && !VSGUtil::CanCreateLink(_newLinkPin, &output) && &output != _newLinkPin)
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
                    ax::Widgets::Icon(ImVec2(_pinIconSize, _pinIconSize), iconType, VSGUtil::IsPinLinked(_links, output.ID), color, ImColor(32, 32, 32, static_cast<int>(alpha)));

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
            if (_newLinkPin && !VSGUtil::CanCreateLink(_newLinkPin, &input) && &input != _newLinkPin)
            {
                alpha = alpha * (48.0f / 255.0f);
            }
            
            builder.Input(input.ID);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);

            ax::Drawing::IconType iconType = VSGUtil::GetPinIconType(input);
            ImColor color = VSGUtil::GetIconColor(input.Type);
            alpha *= 255;
            color.Value.w = alpha / 255.0f;
            ax::Widgets::Icon(ImVec2(_pinIconSize, _pinIconSize), iconType, VSGUtil::IsPinLinked(_links, input.ID), color, ImColor(32, 32, 32, static_cast<int>(alpha)));

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

        // ImageOutput 노드: 처리 결과를 노드 본문에 인라인 미리보기로 표시.
        // 입력[0].ImageValue 는 RunGraph 평가 시 채워지며 다음 Run 까지 유지된다.
        if (node->Op == NodeOp::ImageOutput &&
            node->Inputs.empty() == false &&
            node->Inputs[0].ImageValue &&
            node->Inputs[0].ImageValue->SRV)
        {
            builder.Middle();
            ImGui::Image(reinterpret_cast<ImTextureID>(node->Inputs[0].ImageValue->SRV.Get()), ImVec2(128, 128));
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
            if (_newLinkPin && !VSGUtil::CanCreateLink(_newLinkPin, &output) && &output != _newLinkPin)
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
            ax::Widgets::Icon(ImVec2(_pinIconSize, _pinIconSize), iconType, VSGUtil::IsPinLinked(_links, output.ID), color, ImColor(32, 32, 32, static_cast<int>(alpha)));
            
            ImGui::PopStyleVar();
            builder.EndOutput();
        }

        builder.End();
    }
}

void VisualSequenceGraph::DrawTreeNodes()
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

            if (_newLinkPin && (VSGUtil::CanCreateLink(_newLinkPin, &pin) == false) && &pin != _newLinkPin)
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

            if (_newLinkPin && !VSGUtil::CanCreateLink(_newLinkPin, &pin) && &pin != _newLinkPin)
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

void VisualSequenceGraph::DrawHoudiniNodes()
{
    namespace detail = ax::NodeEditor::Detail;

    // Houdini
    for (auto& node : _nodes)
    {
        if (node->Type != NodeType::Houdini)
        {
            continue;
        }
        
        const float rounding = 10.0f;
        const float padding = 12.0f;

        ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(229, 229, 229, 200));
        ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(125, 125, 125, 200));
        ed::PushStyleColor(ed::StyleColor_PinRect, ImColor(229, 229, 229, 60));
        ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor(125, 125, 125, 60));

        const auto pinBackground = ed::GetStyle().Colors[ed::StyleColor_NodeBg];

        ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(0, 0, 0, 0));
        ed::PushStyleVar(ed::StyleVar_NodeRounding, rounding);
        ed::PushStyleVar(ed::StyleVar_SourceDirection, ImVec2(0.0f, 1.0f));
        ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
        ed::PushStyleVar(ed::StyleVar_LinkStrength, 0.0f);
        ed::PushStyleVar(ed::StyleVar_PinBorderWidth, 1.0f);
        ed::PushStyleVar(ed::StyleVar_PinRadius, 6.0f);
        ed::BeginNode(node->ID);

        ImGui::BeginVertical(node->ID.AsPointer());
        if (node->Inputs.empty() == false)
        {
            ImGui::BeginHorizontal("inputs");
            ImGui::Spring(1, 0);

            ImRect inputsRect;
            int inputAlpha = 200;
            for (auto& pin : node->Inputs)
            {
                ImGui::Dummy(ImVec2(padding, padding));
                // diff
                inputsRect = detail::ImGui_GetItemRect();
                ImGui::Spring(1, 0);
                inputsRect.Min.y -= padding;
                inputsRect.Max.y -= padding;

#if IMGUI_VERSION_NUM > 18101
                const auto allRoundCornersFlags = ImDrawFlags_RoundCornersAll;
#else
                const auto allRoundCornersFlags = 15;
#endif
                //ed::PushStyleVar(ed::StyleVar_PinArrowSize, 10.0f);
                //ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 10.0f);
                ed::PushStyleVar(ed::StyleVar_PinCorners, allRoundCornersFlags);

                ed::BeginPin(pin.ID, ed::PinKind::Input);
                ed::PinPivotRect(inputsRect.GetCenter(), inputsRect.GetCenter());
                ed::PinRect(inputsRect.GetTL(), inputsRect.GetBR());
                ed::EndPin();
                //ed::PopStyleVar(3);
                ed::PopStyleVar(1);

                auto drawList = ImGui::GetWindowDrawList();
                drawList->AddRectFilled(inputsRect.GetTL(), inputsRect.GetBR(),
                    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, allRoundCornersFlags);
                drawList->AddRect(inputsRect.GetTL(), inputsRect.GetBR(),
                    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, allRoundCornersFlags);

                if (_newLinkPin && !VSGUtil::CanCreateLink(_newLinkPin, &pin) && &pin != _newLinkPin)
                {
                    inputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
                }
            }

            //ImGui::Spring(1, 0);
            ImGui::EndHorizontal();
        }

        ImGui::BeginHorizontal("content_frame");
        ImGui::Spring(1, padding);

        ImGui::BeginVertical("content", ImVec2(0.0f, 0.0f));
        ImGui::Dummy(ImVec2(160, 0));
        ImGui::Spring(1);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
        ImGui::TextUnformatted(node->Name.c_str());
        ImGui::PopStyleColor();
        ImGui::Spring(1);
        ImGui::EndVertical();
        auto contentRect = detail::ImGui_GetItemRect();

        ImGui::Spring(1, padding);
        ImGui::EndHorizontal();

        if (node->Outputs.empty() == false)
        {
            ImGui::BeginHorizontal("outputs");
            ImGui::Spring(1, 0);

            ImRect outputsRect;
            int outputAlpha = 200;
            for (auto& pin : node->Outputs)
            {
                ImGui::Dummy(ImVec2(padding, padding));
                outputsRect = detail::ImGui_GetItemRect();
                ImGui::Spring(1, 0);
                outputsRect.Min.y += padding;
                outputsRect.Max.y += padding;

#if IMGUI_VERSION_NUM > 18101
                const auto allRoundCornersFlags = ImDrawFlags_RoundCornersAll;
                const auto topRoundCornersFlags = ImDrawFlags_RoundCornersTop;
#else
                const auto allRoundCornersFlags = 15;
                const auto topRoundCornersFlags = 3;
#endif

                ed::PushStyleVar(ed::StyleVar_PinCorners, topRoundCornersFlags);
                ed::BeginPin(pin.ID, ed::PinKind::Output);
                ed::PinPivotRect(outputsRect.GetCenter(), outputsRect.GetCenter());
                ed::PinRect(outputsRect.GetTL(), outputsRect.GetBR());
                ed::EndPin();
                ed::PopStyleVar();

                auto drawList = ImGui::GetWindowDrawList();
                drawList->AddRectFilled(outputsRect.GetTL(), outputsRect.GetBR(),
                    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, allRoundCornersFlags);
                drawList->AddRect(outputsRect.GetTL(), outputsRect.GetBR(),
                    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, allRoundCornersFlags);

                if (_newLinkPin && (VSGUtil::CanCreateLink(_newLinkPin, &pin) == false) && &pin != _newLinkPin)
                {
                    outputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
                }
            }

            ImGui::EndHorizontal();
        }

        ImGui::EndVertical();

        ed::EndNode();
        ed::PopStyleVar(7);
        ed::PopStyleColor(4);

        // auto drawList = ed::GetNodeBackgroundDrawList(node.ID);

        //const auto fringeScale = ImGui::GetStyle().AntiAliasFringeScale;
        //const auto unitSize    = 1.0f / fringeScale;

        //const auto ImDrawList_AddRect = [](ImDrawList* drawList, const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners, float thickness)
        //{
        //    if ((col >> 24) == 0)
        //        return;
        //    drawList->PathRect(a, b, rounding, rounding_corners);
        //    drawList->PathStroke(col, true, thickness);
        //};

        //drawList->AddRectFilled(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
        //    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
        //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
        //drawList->AddRect(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
        //    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
        //ImGui::PopStyleVar();
        //drawList->AddRectFilled(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
        //    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);
        ////ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
        //drawList->AddRect(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
        //    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);
        ////ImGui::PopStyleVar();
        //drawList->AddRectFilled(contentRect.GetTL(), contentRect.GetBR(), IM_COL32(24, 64, 128, 200), 0.0f);
        //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
        //drawList->AddRect(
        //    contentRect.GetTL(),
        //    contentRect.GetBR(),
        //    IM_COL32(48, 128, 255, 100), 0.0f);
        //ImGui::PopStyleVar();
    }
}

void VisualSequenceGraph::DrawCommentNodes()
{
    namespace detail = ax::NodeEditor::Detail;

    for (auto& node : _nodes)
    {
        if (node->Type != NodeType::Comment)
            continue;

        const float commentAlpha = 0.75f;

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha);
        ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(255, 255, 255, 64));
        ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(255, 255, 255, 64));
        ed::BeginNode(node->ID);
        ImGui::PushID(node->ID.AsPointer());
        ImGui::BeginVertical("content");
        ImGui::BeginHorizontal("horizontal");
        ImGui::Spring(1);
        ImGui::TextUnformatted(node->Name.c_str());
        ImGui::Spring(1);
        ImGui::EndHorizontal();
        ed::Group(node->Size);
        ImGui::EndVertical();
        ImGui::PopID();
        ed::EndNode();
        ed::PopStyleColor(2);
        ImGui::PopStyleVar();

        if (ed::BeginGroupHint(node->ID))
        {
            //auto alpha   = static_cast<int>(commentAlpha * ImGui::GetStyle().Alpha * 255);
            auto bgAlpha = static_cast<int>(ImGui::GetStyle().Alpha * 255);

            //ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha * ImGui::GetStyle().Alpha);

            auto min = ed::GetGroupMin();
            //auto max = ed::GetGroupMax();

            ImGui::SetCursorScreenPos(min - ImVec2(-8, ImGui::GetTextLineHeightWithSpacing() + 4));
            ImGui::BeginGroup();
            ImGui::TextUnformatted(node->Name.c_str());
            ImGui::EndGroup();

            auto drawList = ed::GetHintBackgroundDrawList();

            auto hintBounds = detail::ImGui_GetItemRect();
            auto hintFrameBounds = ImRect_Expanded(hintBounds, 8, 4);

            drawList->AddRectFilled(hintFrameBounds.GetTL(), hintFrameBounds.GetBR(), IM_COL32(255, 255, 255, 64 * bgAlpha / 255), 4.0f);

            drawList->AddRect(hintFrameBounds.GetTL(), hintFrameBounds.GetBR(), IM_COL32(255, 255, 255, 128 * bgAlpha / 255), 4.0f);

            //ImGui::PopStyleVar();
        }
        ed::EndGroupHint();
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

    // TODO : �ߺ� ��ũ ����
    if (ed::QueryNewLink(&startPinId, &endPinId))
    {
        Pin* startPin = VSGUtil::FindPin(_nodes, startPinId);
        Pin* endPin = VSGUtil::FindPin(_nodes, endPinId);

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
            else if (endPin->Node == startPin->Node)
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
                    _links.emplace_back(new Link(Link::GetNextId(), startPinId, endPinId));
                    _links.back()->Color = VSGUtil::GetIconColor(startPin->Type);
                }
            }
        }
    }

    ed::PinId pinId = 0;
    if (ed::QueryNewNode(&pinId))
    {
        _newLinkPin = VSGUtil::FindPin(_nodes, pinId);

        if (ed::AcceptNewItem())
        {
            _createNewNode = true;
            Pin* newNodeLinkPin = VSGUtil::FindPin(_nodes, pinId);
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
            auto it = std::find_if(_nodes.begin(), _nodes.end(), [nodeId](Node* node) { return node->ID == nodeId; });
            if (it != _nodes.end())
            {
                ReleaseNodeImages(*it);
                delete *it;
                _nodes.erase(it);
            }
        }
    }

    ed::LinkId linkId = 0;
    while (ed::QueryDeletedLink(&linkId))
    {
        if (ed::AcceptDeletedItem())
        {
            auto it = std::find_if(_links.begin(), _links.end(), [linkId](Link* link) { return link->ID == linkId; });
            if (it != _links.end())
            {
                delete *it;
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
        Node* node = nullptr;

        // 이미지 처리 노드
        if (ImGui::MenuItem("Image Source"))
        {
            node = NodeGenerator::GenerateImageSourceNode();
        }

        if (ImGui::MenuItem("Grayscale"))
        {
            node = NodeGenerator::GenerateGrayscaleNode();
        }

        if (ImGui::MenuItem("Invert"))
        {
            node = NodeGenerator::GenerateInvertNode();
        }

        if (ImGui::MenuItem("Image Output"))
        {
            node = NodeGenerator::GenerateImageOutputNode();
        }

        ImGui::Separator();

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
            _nodes.push_back(node);

            _createNewNode = false;
            ed::SetNodePosition(node->ID, newNodePostion);

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