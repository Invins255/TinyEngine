#include "pch.h"
#include "Application.h"
#include "Engine/Core/Input.h"
#include "Engine/Renderer/Renderer.h"

#include <GLFW/glfw3.h>

namespace Engine 
{

#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application(const std::string& name)
	{
		ENGINE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		//����Window
		m_Window = std::unique_ptr<Window>(Window::Create(name));
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		Renderer::Init();
		Renderer::WaitAndRender();
	}

	Application::~Application()
	{
		Renderer::Shutdown();
	}

	void Application::Run()
	{		
		this->OnInit();
		while (m_Running)
		{
			float time = glfwGetTime();
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if(!m_Minimized)
			{
				//�ɵ������Ը���Layer����
				for (auto layer : m_LayerStack)
					layer->OnUpdate(timestep);

				Application* app = this;
				Renderer::Submit([app]() 
					{ 
						ENGINE_INFO("RenderCommand: Render ImGui");
						app->RenderImGui(); 
					}
				);

				//ִ��RenderCommand
				Renderer::WaitAndRender();
			}
			m_Window->OnUpdate();
		}
		this->OnShutdown();
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::RenderImGui()
	{
		//�ɵ������Ը���Layer�漰��GUI���и���
		m_ImGuiLayer->Begin();
		for (auto layer : m_LayerStack)
			layer->OnImGuiRender();
		m_ImGuiLayer->End();
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

		//�ɶ�������ѯLayer��Event���д���Խ�ӽ�ջ����Layer���¼����и��ߵĴ������ȼ���
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); ) 
		{
			if (e.Handled)
				break;
			(*--it)->OnEvent(e);
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PopLayer(Layer* layer)
	{
		m_LayerStack.PopLayer(layer);
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushOverlay(overlay);
	}

	void Application::PopOverlay(Layer* overlay)
	{
		m_LayerStack.PopOverlay(overlay);
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		Close();
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		int width = e.GetWidth(), height = e.GetHeight();
		if (width == 0 || height == 0)
		{
			m_Minimized = true;
			return false;
		}
		
		m_Minimized = false;

		Renderer::OnWindowResize(width, height);

		return false;
	}
}
