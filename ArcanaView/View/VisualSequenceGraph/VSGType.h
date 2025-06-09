#pragma once

namespace ed = ax::NodeEditor;

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

struct Pin
{
	ed::PinId ID;
    PinType Type;
};

struct Node
{
	ed::NodeId ID;
	ed::PinId ExecutionInputPinID;
	ed::PinId ExecutionOutputPinID;
};

struct Link
{
	ed::LinkId ID;
	ed::PinId InputID;
	ed::PinId OutputID;
};