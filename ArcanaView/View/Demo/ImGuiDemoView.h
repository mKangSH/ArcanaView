#pragma once
#include "../UIComponentBase.h"

class ImGuiDemoView : public UIComponentBase
{
	using Super = UIComponentBase;

public:
	ImGuiDemoView();
	virtual ~ImGuiDemoView();

public:
	void Update() override;
	void Render() override;

private:
	void Draw();
};

