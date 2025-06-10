#include "pch.h"
#include "NodeGenerator.h"

uint16 Pin::UniquePinId = 1;
uint16 Node::UniqueNodeId = 1;
uint16 Link::UniqueLinkId = 1;

std::shared_ptr<Node> NodeGenerator::GenerateInputActionNode()
{
    std::shared_ptr<Node> node = std::make_shared<Node>(Node::GetNextId(), "Input Action");

    node->Outputs.emplace_back(Pin::GetNextId(), "asdf", PinType::Delegate);
    node->Outputs.emplace_back(Pin::GetNextId(), "Pressed", PinType::Flow);
    node->Outputs.emplace_back(Pin::GetNextId(), "Released", PinType::Flow);

    BuildNode(node);

    return node;
}

std::shared_ptr<Node> NodeGenerator::GenerateBranchNode()
{
    std::shared_ptr<Node> node = std::make_shared<Node>(Node::GetNextId(), "Branch");

    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);
    node->Inputs.emplace_back(Pin::GetNextId(), "Condition", PinType::Bool);
    node->Outputs.emplace_back(Pin::GetNextId(), "True", PinType::Flow);
    node->Outputs.emplace_back(Pin::GetNextId(), "False", PinType::Flow);

    BuildNode(node);

    return node;
}

std::shared_ptr<Node> NodeGenerator::GenerateDoNNode()
{
    std::shared_ptr<Node> node = std::make_shared<Node>(Node::GetNextId(), "Do N");

    node->Inputs.emplace_back(Pin::GetNextId(), "Enter", PinType::Flow);
    node->Inputs.emplace_back(Pin::GetNextId(), "N", PinType::Int);
    node->Inputs.emplace_back(Pin::GetNextId(), "Reset", PinType::Flow);
    node->Outputs.emplace_back(Pin::GetNextId(), "Exit", PinType::Flow);
    node->Outputs.emplace_back(Pin::GetNextId(), "Counter", PinType::Int);

    BuildNode(node);

    return node;
}

std::shared_ptr<Node> NodeGenerator::GenerateOutputActionNode()
{
    std::shared_ptr<Node> node = std::make_shared<Node>(Node::GetNextId(), "Output Action");

    node->Inputs.emplace_back(Pin::GetNextId(), "Sample", PinType::Float);
    node->Outputs.emplace_back(Pin::GetNextId(), "Condition", PinType::Bool);
    node->Inputs.emplace_back(Pin::GetNextId(), "Event", PinType::Delegate);

    BuildNode(node);

    return node;
}

std::shared_ptr<Node> NodeGenerator::GeneratePrintStringNode()
{
    std::shared_ptr<Node> node = std::make_shared<Node>(Node::GetNextId(), "Print String");

    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);
    node->Inputs.emplace_back(Pin::GetNextId(), "In String", PinType::String);
    node->Outputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);

    BuildNode(node);

    return node;
}

std::shared_ptr<Node> NodeGenerator::GenerateMessageNode()
{
    std::shared_ptr<Node> node = std::make_shared<Node>(Node::GetNextId(), "Generate Message");

    node->Type = NodeType::Simple;
    node->Outputs.emplace_back(Pin::GetNextId(), "Message", PinType::String);

    BuildNode(node);

    return node;
}

std::shared_ptr<Node> NodeGenerator::GenerateSetTimerNode()
{
    std::shared_ptr<Node> node = std::make_shared<Node>(Node::GetNextId(), "Set Timer");

    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);
    node->Inputs.emplace_back(Pin::GetNextId(), "Object", PinType::Object);
    node->Inputs.emplace_back(Pin::GetNextId(), "Function Name", PinType::Function);
    node->Inputs.emplace_back(Pin::GetNextId(), "Time", PinType::Float);
    node->Inputs.emplace_back(Pin::GetNextId(), "Looping", PinType::Bool);
    node->Outputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);

    BuildNode(node);

    return node;
}

std::shared_ptr<Node> NodeGenerator::GenerateLessNode()
{
    std::shared_ptr<Node> node = std::make_shared<Node>(Node::GetNextId(), "Generate Less");

    node->Type = NodeType::Simple;
    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Float);
    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Float);
    node->Outputs.emplace_back(Pin::GetNextId(), "", PinType::Float);

    BuildNode(node);

    return node;
}

std::shared_ptr<Node> NodeGenerator::GenerateWeirdNode()
{
    std::shared_ptr<Node> node = std::make_shared<Node>(Node::GetNextId(), "Generate Weird");

    node->Type = NodeType::Simple;
    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Float);
    node->Outputs.emplace_back(Pin::GetNextId(), "", PinType::Float);
    node->Outputs.emplace_back(Pin::GetNextId(), "", PinType::Float);

    BuildNode(node);

    return node;
}

std::shared_ptr<Node> NodeGenerator::GenerateTraceByChannelNode()
{
    std::shared_ptr<Node> node = std::make_shared<Node>(Node::GetNextId(), "Generate Trace By Channel");

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

std::shared_ptr<Node> NodeGenerator::GenerateTreeSequenceNode()
{
    std::shared_ptr<Node> node = std::make_shared<Node>(Node::GetNextId(), "Sequence");

    node->Type = NodeType::Tree;
    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);
    node->Outputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);

    BuildNode(node);

    return node;
}

std::shared_ptr<Node> NodeGenerator::GenerateTreeTaskNode()
{
    std::shared_ptr<Node> node = std::make_shared<Node>(Node::GetNextId(), "Move To");

    node->Type = NodeType::Tree;
    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);

    BuildNode(node);

    return node;
}

std::shared_ptr<Node> NodeGenerator::GenerateTreeTask2Node()
{
    std::shared_ptr<Node> node = std::make_shared<Node>(Node::GetNextId(), "Random Wait");

    node->Type = NodeType::Tree;
    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);

    BuildNode(node);

    return node;
}

std::shared_ptr<Node> NodeGenerator::GenerateComment()
{
    std::shared_ptr<Node> node = std::make_shared<Node>(Node::GetNextId(), "Test Comment");

    node->Type = NodeType::Comment;
    node->Size = ImVec2(300, 200);

    BuildNode(node);

    return node;
}

std::shared_ptr<Node> NodeGenerator::GenerateHoudiniTransformNode()
{
    std::shared_ptr<Node> node = std::make_shared<Node>(Node::GetNextId(), "Transform");

    node->Type = NodeType::Houdini;
    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);
    node->Outputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);

    BuildNode(node);

    return node;
}

std::shared_ptr<Node> NodeGenerator::GenerateHoudiniGroupNode()
{
    std::shared_ptr<Node> node = std::make_shared<Node>(Node::GetNextId(), "Group");

    node->Type = NodeType::Houdini;
    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);
    node->Inputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);
    node->Outputs.emplace_back(Pin::GetNextId(), "", PinType::Flow);

    BuildNode(node);

    return node;
}

void NodeGenerator::BuildNode(std::shared_ptr<Node> node)
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
