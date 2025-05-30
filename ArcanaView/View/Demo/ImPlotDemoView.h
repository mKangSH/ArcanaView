#pragma once
#include "../UIComponentBase.h"

class ImPlotDemoView : public UIComponentBase
{
	using Super = UIComponentBase;

public:
	ImPlotDemoView();
	virtual ~ImPlotDemoView();

public:
	void Update() override;
	void Render() override;

private:
	void Draw();
};

