#pragma once

namespace ed = ax::NodeEditor;

struct Node;

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

    std::weak_ptr<Node> Node;

    PinType Type;
    PinKind Kind;

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