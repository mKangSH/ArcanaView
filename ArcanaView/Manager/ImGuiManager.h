#pragma once

class ImGuiManager
{
	DECLARE_SINGLETON(ImGuiManager)

public:
	void Init(HWND hwnd);
	void Update();
	void Render();

	void Cleanup();
};

