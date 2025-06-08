#include "pch.h"
#include "ImPlotDemoView.h"

ImPlotDemoView::ImPlotDemoView() : Super("ImPlot Demo Window")
{
	_isVisible = false;
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
