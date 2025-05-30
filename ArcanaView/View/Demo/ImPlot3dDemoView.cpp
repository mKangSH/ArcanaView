#include "pch.h"
#include "ImPlot3dDemoView.h"

ImPlot3dDemoView::ImPlot3dDemoView() : Super("ImPlot3D Demo Window")
{
}

ImPlot3dDemoView::~ImPlot3dDemoView()
{
}

void ImPlot3dDemoView::Update()
{
	if (_isVisible)
	{
		Draw();
	}
}

void ImPlot3dDemoView::Render()
{
}

void ImPlot3dDemoView::Draw()
{
	ImPlot3D::ShowDemoWindow(&_isVisible);
}
