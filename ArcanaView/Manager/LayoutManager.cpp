#include "pch.h"
#include "LayoutManager.h"
#include "View/UIComponentBase.h"
#include "View/HelpView.h"
#include "View/VisualSequenceGraph/VisualSequenceGraph.h"
#include "View/Demo/ImGuiDemoView.h"
#include "View/Demo/ImPlotDemoView.h"
#include "View/Demo/ImPlot3dDemoView.h"
#include "View/ImageView.h"

void LayoutManager::Init()
{
	ImGuiDemoView* imGuiDemoView = new ImGuiDemoView();
	_uiComponents.push_back(imGuiDemoView);

	ImPlotDemoView* imPlotDemoView = new ImPlotDemoView();
	_uiComponents.push_back(imPlotDemoView);

	ImPlot3dDemoView* imPlot3dDemoView = new ImPlot3dDemoView();
	_uiComponents.push_back(imPlot3dDemoView);

	HelpView* helpView = new HelpView();
	_uiComponents.push_back(helpView);

	VisualSequenceGraph* visualSequenceGraph = new VisualSequenceGraph();
	_uiComponents.push_back(visualSequenceGraph);
	_visualSequenceGraph = visualSequenceGraph;
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
		UIComponentBase* imageView = (*it).second;
		bool* isVisible = imageView->IsVisible();
		if (*isVisible == false)
		{
			delete imageView;
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
	// _visualSequenceGraph 는 _uiComponents 가 소유하므로 별도 delete 하지 않는다.
	_visualSequenceGraph = nullptr;

	for (UIComponentBase* uiComponent : _uiComponents)
	{
		delete uiComponent;
	}
	_uiComponents.clear();

	for (auto& imageView : _imageViewComponents)
	{
		delete imageView.second;
	}
	_imageViewComponents.clear();
}

void LayoutManager::AddImageView(const std::wstring& imageFile)
{
	ImageView* imageView = new ImageView(imageFile);
	_imageViewComponents.insert({ imageView->GetTitle(), imageView });
}

void LayoutManager::PromoteImageToGraph(const DirectX::ScratchImage& image)
{
	if (_visualSequenceGraph)
	{
		_visualSequenceGraph->AddImageSourceNode(image);
	}
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