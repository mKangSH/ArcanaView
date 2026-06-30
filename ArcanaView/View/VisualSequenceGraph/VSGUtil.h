#pragma once
#include "VSGType.h"
#include <BlueprintUtility/drawing.h>

class VSGUtil
{
public:
	static ImColor GetIconColor(PinType type);
	static ax::Drawing::IconType GetPinIconType(const Pin& pin);

	static bool IsPinLinked(const std::vector<Link*>& links, ed::PinId id);
	static bool CanCreateLink(Pin* input, Pin* output);
	static Pin* FindPin(const std::vector<Node*>& nodes, ed::PinId id);
	static Node* FindNode(const std::vector<Node*>& nodes, ed::NodeId id);
};

