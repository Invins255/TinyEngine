#pragma once

#include <functional>
#include <string>
#include "Engine/Core/Core.h"
#include "Engine/Events/Event.h"

namespace Engine 
{
	struct WindowProps
	{  
		std::string Title;
		uint32_t Width;
		uint32_t Height;

		WindowProps(const std::string& title = "Tiny Engine",
			uint32_t width = 1280,
			uint32_t height = 720
		) 
			:Title(title), Width(width), Height(height)
		{
		}
	};
	 
	/// <summary>
	/// Window����, ���ڲ�ͬƽ̨���о���ʵ��
	/// </summary>
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void OnUpdate() = 0;
		
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual std::pair<float, float> GetWindowPos() const = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;

		/// <summary>
		/// Window�������������������ʵ��
		/// </summary>
		/// <returns>����ƽ̨��Windowָ��</returns>
		static Scope<Window> Create(const WindowProps& props = WindowProps());
	};
}