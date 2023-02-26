#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Engine/Core/Core.h"
#include "Engine/Core/Ref.h"
#include "Engine/Core/Window.h"
#include "Engine/Renderer/RendererContext.h"

namespace Engine
{
	/// <summary>
	/// 基于Windows平台的Window实现（GLFW）
	/// </summary>
	class WindowsWindow: public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		virtual void OnUpdate() override;
		
		virtual unsigned int GetWidth() const override { return m_Data.Width; }
		virtual unsigned int GetHeight() const override { return m_Data.Height; }
		virtual std::pair<float, float> GetWindowPos() const override;

		virtual void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		
		virtual void SetVSync(bool enabled) override;
		virtual bool IsVSync() const override;

		virtual void* GetNativeWindow() const { return m_Window; }
	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();

	private:
		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		GLFWwindow* m_Window;
		WindowData m_Data;
		Ref<RendererContext> m_Context;
	};

}


