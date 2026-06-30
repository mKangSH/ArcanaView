#pragma once

namespace ed = ax::NodeEditor;

struct Node;
struct GpuImage; // 정의는 Utility/ImageProcess/GpuImageProcessor.h

enum class PinType
{
    Flow,
    Bool,
    Int,
    Float,
    String,
    Object,
    Function,
    Delegate,
    Image,
};

// 노드가 수행하는 이미지 처리 연산 식별자 (시각적 NodeType 과는 별개).
// None 인 노드는 평가기에서 무시된다 (기존 데모 노드).
enum class NodeOp
{
    None,
    ImageSource,
    Grayscale,
    Invert,
    ImageOutput,
};

enum class PinKind
{
    Output,
    Input
};

enum class NodeType
{
    Blueprint,
    Simple,
    Tree,
    Comment,
    Houdini
};

struct Pin
{
	ed::PinId ID;
    std::string Name;

    Node* Node = nullptr; // 소유하지 않는 부모 노드 역참조

    PinType Type;
    PinKind Kind;

    // 평가 시 핀이 운반하는 런타임 값 (그래프 실행 중에만 채워짐).
    // 출력 핀이 GpuImage 를 소유하고, 입력 핀은 빌려쓰기만 한다(소유하지 않음).
    GpuImage* ImageValue = nullptr;
    float ScalarValue = 0.0f;

    Pin(uint64 id, std::string name, PinType type)
        : ID(id), Name(name), Type(type), Kind(PinKind::Input)
    {

    }

public:
    static uint64 GetNextId()
    {
        assert(UniquePinId != USHORT_MAX);
        return (UniquePinId++);
    }

private:
    static uint16 UniquePinId;
};

struct Node
{
	ed::NodeId ID;
    std::string Name;
    ImColor Color;
    std::vector<Pin> Inputs;
    std::vector<Pin> Outputs;

    NodeType Type;
    ImVec2 Size;

    // 이미지 처리 연산 종류 (기본 None -> 평가기 무시).
    NodeOp Op = NodeOp::None;

    std::string State;
    std::string SavedState;

    Node(uint64 id, std::string name, ImColor color = ImColor(255, 255, 255))
        : ID(id), Name(name), Color(color), Type(NodeType::Blueprint), Size(0, 0)
    {

    }

public:
    static uint64 GetNextId()
    {
        uint64 id = static_cast<uint64>(UniqueNodeId) << 8;
        assert(UniqueNodeId != USHORT_MAX);
        UniqueNodeId++;
        return id;
    }

private:
    static uint16 UniqueNodeId;
};

struct Link
{
	ed::LinkId ID;
	ed::PinId InputPinID;
	ed::PinId OutputPinID;
    ImColor Color;

    Link(ed::LinkId id, ed::PinId inputPinId, ed::PinId outputPinId)
        : ID(id), InputPinID(inputPinId), OutputPinID(outputPinId), Color(255, 255, 255)
    {
    }

public:
    static uint64 GetNextId()
    {
        uint64 id = static_cast<uint64>(UniqueLinkId) << 16;
        assert(UniqueLinkId != USHORT_MAX);
        UniqueLinkId++;
        return id;
    }

private:
    static uint16 UniqueLinkId;
};