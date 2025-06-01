#include "pch.h"
#include "LayoutManager.h"
#include "View/UIComponentBase.h"
#include "View/Demo/ImGuiDemoView.h"
#include "View/Demo/ImPlotDemoView.h"
#include "View/Demo/ImPlot3dDemoView.h"
#include "View/ImageView.h"

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

	for (auto it = _imageViewComponents.begin(); it != _imageViewComponents.end();) 
	{
		std::shared_ptr<UIComponentBase> imageView = (*it).second;
		bool* isVisible = imageView->IsVisible();
		if (*isVisible == false)
		{
			_imageViewComponents.erase(it++);
		}
		else
		{
			imageView->Update();
			++it;
		}
	}
}

void LayoutManager::Render()
{
	for (auto uiComponent : _uiComponents)
	{
		uiComponent->Render();
	}

	for (auto& imageView : _imageViewComponents)
	{
		(imageView.second)->Render();
	}
}

void LayoutManager::Cleanup()
{
	_uiComponents.clear();
}

void LayoutManager::AddImageView(const std::wstring& imageFile)
{
	std::shared_ptr<ImageView> imageView = std::make_shared<ImageView>(imageFile);
	_imageViewComponents.insert({ imageView->GetTitle(), imageView });
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