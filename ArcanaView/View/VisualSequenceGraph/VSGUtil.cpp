#include "pch.h"
#include "VSGUtil.h"

ImColor VSGUtil::GetIconColor(PinType type)
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

ax::Drawing::IconType VSGUtil::GetPinIconType(const Pin& pin)
{
    switch (pin.Type)
    {
    case PinType::Flow:     return ax::Drawing::IconType::Flow;
    case PinType::Bool:     return ax::Drawing::IconType::Circle;
    case PinType::Int:      return ax::Drawing::IconType::Circle;
    case PinType::Float:    return ax::Drawing::IconType::Circle;
    case PinType::String:   return ax::Drawing::IconType::Circle;
    case PinType::Object:   return ax::Drawing::IconType::Circle;
    case PinType::Function: return ax::Drawing::IconType::Circle;
    case PinType::Delegate: return ax::Drawing::IconType::Square;
    default:
        return ax::Drawing::IconType::Circle;
    }
}

bool VSGUtil::IsPinLinked(const std::vector<std::shared_ptr<Link>>& links, ed::PinId id)
{
    if (id == ed::PinId::Invalid)
    {
        return false;
    }

    for (auto& link : links)
    {
        if (link->InputPinID == id || link->OutputPinID == id)
        {
            return true;
        }
    }

    return false;
}

bool VSGUtil::CanCreateLink(Pin* input, Pin* output)
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

Pin* VSGUtil::FindPin(const std::vector<std::shared_ptr<Node>>& nodes, ed::PinId id)
{
    if (id == ed::PinId::Invalid)
    {
        return nullptr;
    }

    for (auto& node : nodes)
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

std::shared_ptr<Node> VSGUtil::FindNode(const std::vector<std::shared_ptr<Node>>& nodes, ed::NodeId id)
{
    for (auto& node : nodes)
    {
        if (node->ID == id)
        {
            return node;
        }
    }

    return nullptr;
}