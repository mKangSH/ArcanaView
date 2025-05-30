#include "pch.h"
#include "ImPlotDemoView.h"

ImPlotDemoView::ImPlotDemoView() : Super("ImPlot Demo Window")
{
}

ImPlotDemoView::~ImPlotDemoView()
{
}

void ImPlotDemoView::Update()
{
	if (_isVisible)
	{
		Draw();
	}
}

void ImPlotDemoView::Render()
{
}

void ImPlotDemoView::Draw()
{
	ImPlot::ShowDemoWindow(&_isVisible);
}
