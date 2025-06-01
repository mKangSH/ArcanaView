#pragma once

#pragma once

#define DECLARE_SINGLETON(ClassName)	\
private:								\
	ClassName() {}						\
										\
public:									\
	static ClassName* GetInstance()		\
	{									\
		static ClassName s_instance;	\
		return &s_instance;				\
	}									\


#define GET_SINGLE(ClassName) ClassName::GetInstance()

#define GRAPHICS		GET_SINGLE(Graphics)
#define DEVICE			GRAPHICS->GetDevice()
#define DEVICECONTEXT	GRAPHICS->GetDeviceContext()
#define GUI				GET_SINGLE(ImGuiManager)
#define LAYOUT			GET_SINGLE(LayoutManager)