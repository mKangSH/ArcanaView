#pragma once
#include "VSGType.h"

class NodeGenerator
{
public:
	static std::shared_ptr<Node> GenerateInputActionNode();
	static std::shared_ptr<Node> GenerateBranchNode();
	static std::shared_ptr<Node> GenerateDoNNode();
	static std::shared_ptr<Node> GenerateOutputActionNode();
	static std::shared_ptr<Node> GeneratePrintStringNode();
	static std::shared_ptr<Node> GenerateMessageNode();
	static std::shared_ptr<Node> GenerateSetTimerNode();
	static std::shared_ptr<Node> GenerateLessNode();
	static std::shared_ptr<Node> GenerateWeirdNode();
	static std::shared_ptr<Node> GenerateTraceByChannelNode();
	static std::shared_ptr<Node> GenerateTreeSequenceNode();
	static std::shared_ptr<Node> GenerateTreeTaskNode();
	static std::shared_ptr<Node> GenerateTreeTask2Node();
	static std::shared_ptr<Node> GenerateComment();
	static std::shared_ptr<Node> GenerateHoudiniTransformNode();
	static std::shared_ptr<Node> GenerateHoudiniGroupNode();

private:
	static void BuildNode(std::shared_ptr<Node> node);
};

