#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/Ref.h"
#include "Engine/Core/Window.h"
#include "Engine/Core/TimeStep.h"
#include "Engine/Core/LayerStack.h"
#include "Engine/Events/Event.h"
#include "Engine/Events/ApplicationEvent.h"
#include "Engine/ImGui/ImGuiLayer.h" 

namespace Engine 
{
	/// <summary>
	/// Application基类，由GameApplication进行继承。Application是一个单例。
	/// </summary>
	class Application
	{
	public:
		Application(const std::string& name = "Application");
		virtual ~Application();

		static Application& Get() { return *s_Instance; }

		Window& GetWindow() { return *m_Window; }
		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		/// <summary>
		/// Application主循环
		/// </summary>
		void Run();
		void Close();
		void RenderImGui();

		virtual void OnInit() {};
		virtual void OnShutdown() {};

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopOverlay(Layer* overlay);

		std::string OpenFile(const char* filter = "All\0*.*\0") const;
		std::string SaveFile(const char* filter = "All\0*.*\0") const;

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		static Application* s_Instance;
		
		Scope<Window> m_Window;
		bool m_Running = true;
		bool m_Minimized = false;

		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;

		float m_LastFrameTime = 0.0f;
	};

	/// <summary>
	/// Application工厂函数
	/// </summary>
	/// <returns>GameApplication</returns>
	Application* CreateApplication();
}
