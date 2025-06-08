#pragma once
#include "UIComponentBase.h"

class HelpView : public UIComponentBase
{
	using Super = UIComponentBase;

public:
	HelpView();
	virtual ~HelpView();

public:
	void Update() override;
	void Render() override;

private:
	void Draw();
	void Cleanup();

private:
	ImGuiWindowFlags _windowFlags;
};