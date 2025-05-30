#pragma once

class ImGuiManager
{
public:
	void Init(HWND hwnd);
	void Update();
	void Render();

	void Cleanup();
};

