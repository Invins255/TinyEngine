#include "pch.h"
#include "WindowsWindow.h"
#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/Renderer/RendererContext.h"


namespace Engine 
{
	static bool s_GLFWInitialized = false;

	static void GLFWErrorCallback(int error, const char* description)
	{
		ENGINE_ERROR("GLFW ERROR {0}: {1}", error, description);
	}

	Engine::WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		Init(props);
	}

	Engine::WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void Engine::WindowsWindow::OnUpdate()
	{
		glfwPollEvents();
		m_Context->SwapBuffers();
	}

	std::pair<float, float> WindowsWindow::GetWindowPos() const
	{
		int x, y;
		glfwGetWindowPos(m_Window, &x, &y);
		return { x, y };
	}

	void Engine::WindowsWindow::SetVSync(bool enabled)
	{
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.VSync = enabled;
	}

	bool Engine::WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}

	void Engine::WindowsWindow::Init(const WindowProps& props)
	{
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		ENGINE_INFO("Creating Windows window {0} ({1}, {2})", props.Title, props.Width, props.Height);

		//初始化Glfw
		if (!s_GLFWInitialized)
		{
			int success = glfwInit();		
			ENGINE_ASSERT(success, "Initialize GLFW failed!");
			glfwSetErrorCallback(GLFWErrorCallback);
			s_GLFWInitialized = true;
		}
		//创建Glfw窗口
		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
		
		//创建Context
		m_Context = RendererContext::Create(m_Window);
		m_Context->Init();

		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		//--------------------------------------------------------------------------
		//设置GLFW callback
		//--------------------------------------------------------------------------
		{
			glfwSetWindowSizeCallback(m_Window,
				[](GLFWwindow* window, int width, int height)
				{
					WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
					data.Width = width;
					data.Height = height;

					WindowResizeEvent event(width, height);
					data.EventCallback(event);
				}
			);
			glfwSetWindowCloseCallback(m_Window,
				[](GLFWwindow* window)
				{
					WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
					WindowCloseEvent event;
					data.EventCallback(event);
				}
			);
			glfwSetKeyCallback(m_Window,
				[](GLFWwindow* window, int key, int scancode, int action, int mods)
				{
					WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
					switch (action)
					{
					case GLFW_PRESS:
					{
						KeyPressedEvent event(key, 0);
						data.EventCallback(event);
						break;
					}
					case GLFW_RELEASE:
					{
						KeyReleasedEvent event(key);
						data.EventCallback(event);
						break;
					}
					case GLFW_REPEAT:
					{
						KeyPressedEvent event(key, 1);
						data.EventCallback(event);
						break;
					}
					}
				}
			);
			glfwSetCharCallback(m_Window,
				[](GLFWwindow* window, unsigned int keyCode)
				{
					WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
					KeyTypedEvent event(keyCode);
					data.EventCallback(event);
				}
			);
			glfwSetMouseButtonCallback(m_Window,
				[](GLFWwindow* window, int button, int action, int mods)
				{
					WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
					switch (action)
					{
					case GLFW_PRESS:
					{
						MouseButtonPressedEvent event(button);
						data.EventCallback(event);
						break;
					}
					case GLFW_RELEASE:
					{
						MouseButtonReleasedEvent event(button);
						data.EventCallback(event);
						break;
					}
					}
				}
			);
			glfwSetScrollCallback(m_Window,
				[](GLFWwindow* window, double xOffset, double yOffset)
				{
					WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

					MouseScrolledEvent event(xOffset, yOffset);
					data.EventCallback(event);
				}
			);
			glfwSetCursorPosCallback(m_Window,
				[](GLFWwindow* window, double xPos, double yPos)
				{
					WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

					MouseMovedEvent event(xPos, yPos);
					data.EventCallback(event);
				}
			);
		}
	}

	void Engine::WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(m_Window);
	}

}