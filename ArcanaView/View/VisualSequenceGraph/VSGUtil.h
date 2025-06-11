#pragma once
#include "VSGType.h"
#include <BlueprintUtility/drawing.h>

class VSGUtil
{
public:
	static ImColor GetIconColor(PinType type);
	static ax::Drawing::IconType GetPinIconType(const Pin& pin);
};

