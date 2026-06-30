#pragma once
#include "VSGType.h"

class NodeGenerator
{
public:
	static Node* GenerateInputActionNode();
	static Node* GenerateBranchNode();
	static Node* GenerateDoNNode();
	static Node* GenerateOutputActionNode();
	static Node* GeneratePrintStringNode();
	static Node* GenerateMessageNode();
	static Node* GenerateSetTimerNode();
	static Node* GenerateLessNode();
	static Node* GenerateWeirdNode();
	static Node* GenerateTraceByChannelNode();
	static Node* GenerateTreeSequenceNode();
	static Node* GenerateTreeTaskNode();
	static Node* GenerateTreeTask2Node();
	static Node* GenerateComment();
	static Node* GenerateHoudiniTransformNode();
	static Node* GenerateHoudiniGroupNode();

public:
	// 이미지 처리 노드
	static Node* GenerateImageSourceNode();
	static Node* GenerateGrayscaleNode();
	static Node* GenerateInvertNode();
	static Node* GenerateImageOutputNode();

public:
	static void BuildNode(Node* node);
};

