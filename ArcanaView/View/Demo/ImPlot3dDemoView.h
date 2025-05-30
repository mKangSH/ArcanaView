#pragma once
#include "../UIComponentBase.h"

class ImPlot3dDemoView : public UIComponentBase
{
	using Super = UIComponentBase;

public:
	ImPlot3dDemoView();
	virtual ~ImPlot3dDemoView();

public:
	void Update() override;
	void Render() override;

private:
	void Draw();
};

