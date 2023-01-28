#include "pch.h"
#include "Window.h"

#ifdef ENGINE_PLATFORM_WINDOWS
	#include "Engine/Platforms/Windows/WindowsWindow.h"
#endif


namespace Engine
{
	Scope<Window> Window::Create(const WindowProps& props)
	{
#ifdef ENGINE_PLATFORM_WINDOWS
		return CreateScope<WindowsWindow>(props);
#else
		ENGINE_ASSERT(false, "Unknown platform!");
		return nullptr;
#endif	
	}

}