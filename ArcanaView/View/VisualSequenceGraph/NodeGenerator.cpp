#include "pch.h"
#include "NodeGenerator.h"

uint16 Pin::UniquePinId = 1;
uint16 Node::UniqueNodeId = 1;
uint16 Link::UniqueLinkId = 1;

Node* NodeGenerator::GenerateInputActionNode()
{
    Node* node = new Node(Node::GetNextId(), "Input Action");

    node->Outputs.emplace_back(Pin::GetNextId(), "asdf", PinType::Delegate);
    node->Outputs.emplace_back(Pin::GetNextId(), "Pressed", PinType::Flow);
    node->Outputs.emplace_back(Pin::GetNextId(), "Released", PinType::Flow);

    BuildNode(node);

    return node;
}

Node* NodeGenerator::GenerateBranchNode()
{
    Node* node = new Node(Node::GetNextId(), "Branch");

    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);
    node->Inputs.emplace_back(Pin::GetNextId(), "Condition", PinType::Bool);
    node->Outputs.emplace_back(Pin::GetNextId(), "True", PinType::Flow);
    node->Outputs.emplace_back(Pin::GetNextId(), "False", PinType::Flow);

    BuildNode(node);

    return node;
}

Node* NodeGenerator::GenerateDoNNode()
{
    Node* node = new Node(Node::GetNextId(), "Do N");

    node->Inputs.emplace_back(Pin::GetNextId(), "Enter", PinType::Flow);
    node->Inputs.emplace_back(Pin::GetNextId(), "N", PinType::Int);
    node->Inputs.emplace_back(Pin::GetNextId(), "Reset", PinType::Flow);
    node->Outputs.emplace_back(Pin::GetNextId(), "Exit", PinType::Flow);
    node->Outputs.emplace_back(Pin::GetNextId(), "Counter", PinType::Int);

    BuildNode(node);

    return node;
}

Node* NodeGenerator::GenerateOutputActionNode()
{
    Node* node = new Node(Node::GetNextId(), "Output Action");

    node->Inputs.emplace_back(Pin::GetNextId(), "Sample", PinType::Float);
    node->Outputs.emplace_back(Pin::GetNextId(), "Condition", PinType::Bool);
    node->Inputs.emplace_back(Pin::GetNextId(), "Event", PinType::Delegate);

    BuildNode(node);

    return node;
}

Node* NodeGenerator::GeneratePrintStringNode()
{
    Node* node = new Node(Node::GetNextId(), "Print String");

    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);
    node->Inputs.emplace_back(Pin::GetNextId(), "In String", PinType::String);
    node->Outputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);

    BuildNode(node);

    return node;
}

Node* NodeGenerator::GenerateMessageNode()
{
    Node* node = new Node(Node::GetNextId(), "Generate Message");

    node->Type = NodeType::Simple;
    node->Outputs.emplace_back(Pin::GetNextId(), "Message", PinType::String);

    BuildNode(node);

    return node;
}

Node* NodeGenerator::GenerateSetTimerNode()
{
    Node* node = new Node(Node::GetNextId(), "Set Timer");

    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);
    node->Inputs.emplace_back(Pin::GetNextId(), "Object", PinType::Object);
    node->Inputs.emplace_back(Pin::GetNextId(), "Function Name", PinType::Function);
    node->Inputs.emplace_back(Pin::GetNextId(), "Time", PinType::Float);
    node->Inputs.emplace_back(Pin::GetNextId(), "Looping", PinType::Bool);
    node->Outputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);

    BuildNode(node);

    return node;
}

Node* NodeGenerator::GenerateLessNode()
{
    Node* node = new Node(Node::GetNextId(), "Generate Less");

    node->Type = NodeType::Simple;
    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Float);
    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Float);
    node->Outputs.emplace_back(Pin::GetNextId(), "", PinType::Float);

    BuildNode(node);

    return node;
}

Node* NodeGenerator::GenerateWeirdNode()
{
    Node* node = new Node(Node::GetNextId(), "Generate Weird");

    node->Type = NodeType::Simple;
    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Float);
    node->Outputs.emplace_back(Pin::GetNextId(), "", PinType::Float);
    node->Outputs.emplace_back(Pin::GetNextId(), "", PinType::Float);

    BuildNode(node);

    return node;
}

Node* NodeGenerator::GenerateTraceByChannelNode()
{
    Node* node = new Node(Node::GetNextId(), "Generate Trace By Channel");

    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);
    node->Inputs.emplace_back(Pin::GetNextId(), "Start", PinType::Flow);
    node->Inputs.emplace_back(Pin::GetNextId(), "End", PinType::Int);
    node->Inputs.emplace_back(Pin::GetNextId(), "Trace Channel", PinType::Float);
    node->Inputs.emplace_back(Pin::GetNextId(), "Trace Complex", PinType::Bool);
    node->Inputs.emplace_back(Pin::GetNextId(), "Actors to Ignore", PinType::Int);
    node->Inputs.emplace_back(Pin::GetNextId(), "Draw Debug Type", PinType::Bool);
    node->Inputs.emplace_back(Pin::GetNextId(), "Ignore Self", PinType::Bool);
    node->Outputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);
    node->Outputs.emplace_back(Pin::GetNextId(), "Out Hit", PinType::Float);
    node->Outputs.emplace_back(Pin::GetNextId(), "Return Value", PinType::Bool);

    BuildNode(node);

    return node;
}

Node* NodeGenerator::GenerateTreeSequenceNode()
{
    Node* node = new Node(Node::GetNextId(), "Sequence");

    node->Type = NodeType::Tree;
    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);
    node->Outputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);

    BuildNode(node);

    return node;
}

Node* NodeGenerator::GenerateTreeTaskNode()
{
    Node* node = new Node(Node::GetNextId(), "Move To");

    node->Type = NodeType::Tree;
    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);

    BuildNode(node);

    return node;
}

Node* NodeGenerator::GenerateTreeTask2Node()
{
    Node* node = new Node(Node::GetNextId(), "Random Wait");

    node->Type = NodeType::Tree;
    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);

    BuildNode(node);

    return node;
}

Node* NodeGenerator::GenerateComment()
{
    Node* node = new Node(Node::GetNextId(), "Test Comment");

    node->Type = NodeType::Comment;
    node->Size = ImVec2(300, 200);

    BuildNode(node);

    return node;
}

Node* NodeGenerator::GenerateHoudiniTransformNode()
{
    Node* node = new Node(Node::GetNextId(), "Transform");

    node->Type = NodeType::Houdini;
    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);
    node->Outputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);

    BuildNode(node);

    return node;
}

Node* NodeGenerator::GenerateHoudiniGroupNode()
{
    Node* node = new Node(Node::GetNextId(), "Group");

    node->Type = NodeType::Houdini;
    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);
    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);
    node->Outputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);

    BuildNode(node);

    return node;
}

Node* NodeGenerator::GenerateImageSourceNode()
{
    Node* node = new Node(Node::GetNextId(), "Image Source", ImColor(120, 180, 90));

    node->Op = NodeOp::ImageSource;
    node->Outputs.emplace_back(Pin::GetNextId(), "Image", PinType::Image);

    BuildNode(node);

    return node;
}

Node* NodeGenerator::GenerateGrayscaleNode()
{
    Node* node = new Node(Node::GetNextId(), "Grayscale", ImColor(90, 140, 180));

    node->Op = NodeOp::Grayscale;
    node->Inputs.emplace_back(Pin::GetNextId(), "Image", PinType::Image);
    node->Outputs.emplace_back(Pin::GetNextId(), "Image", PinType::Image);

    BuildNode(node);

    return node;
}

Node* NodeGenerator::GenerateInvertNode()
{
    Node* node = new Node(Node::GetNextId(), "Invert", ImColor(90, 140, 180));

    node->Op = NodeOp::Invert;
    node->Inputs.emplace_back(Pin::GetNextId(), "Image", PinType::Image);
    node->Outputs.emplace_back(Pin::GetNextId(), "Image", PinType::Image);

    BuildNode(node);

    return node;
}

Node* NodeGenerator::GenerateImageOutputNode()
{
    Node* node = new Node(Node::GetNextId(), "Image Output", ImColor(180, 120, 90));

    node->Op = NodeOp::ImageOutput;
    node->Inputs.emplace_back(Pin::GetNextId(), "Image", PinType::Image);

    BuildNode(node);

    return node;
}

void NodeGenerator::BuildNode(Node* node)
{
    for (auto& input : node->Inputs)
    {
        input.Node = node;
        input.Kind = PinKind::Input;
    }

    for (auto& output : node->Outputs)
    {
        output.Node = node;
        output.Kind = PinKind::Output;
    }
}
