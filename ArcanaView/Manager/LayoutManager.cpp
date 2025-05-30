#include "pch.h"
#include "LayoutManager.h"
#include "View/UIComponentBase.h"
#include "View/Demo/ImGuiDemoView.h"
#include "View/Demo/ImPlotDemoView.h"
#include "View/Demo/ImPlot3dDemoView.h"

void LayoutManager::Init()
{
	std::shared_ptr<ImGuiDemoView> imGuiDemoView = std::make_shared<ImGuiDemoView>();
	_uiComponents.push_back(imGuiDemoView);

	std::shared_ptr<ImPlotDemoView> imPlotDemoView = std::make_shared<ImPlotDemoView>();
	_uiComponents.push_back(imPlotDemoView);

	std::shared_ptr<ImPlot3dDemoView> imPlot3dDemoView = std::make_shared<ImPlot3dDemoView>();
	_uiComponents.push_back(imPlot3dDemoView);
}

void LayoutManager::Update()
{
	ImGui::DockSpaceOverViewport();

	ConstructLayout();

	for (auto uiComponent : _uiComponents)
	{
		uiComponent->Update();
	}
}

void LayoutManager::Render()
{
	for (auto uiComponent : _uiComponents)
	{
		uiComponent->Render();
	}
}

void LayoutManager::Cleanup()
{
	_uiComponents.clear();
}

void LayoutManager::ConstructLayout()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("View"))
		{
			for (auto uiComponent : _uiComponents)
			{
				ImGui::MenuItem(uiComponent->GetTitle().c_str(), "", uiComponent->IsVisible());
			}

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}