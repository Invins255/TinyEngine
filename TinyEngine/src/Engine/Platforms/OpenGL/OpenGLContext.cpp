#include "pch.h"
#include "OpenGLContext.h"
#include "Engine/Core/Core.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Engine
{
    OpenGLContext::OpenGLContext(GLFWwindow* windowHandle):
        m_WindowHandle(windowHandle)
    {
        ENGINE_ASSERT(windowHandle, "WindowHandle is nullptr");
    }

    void OpenGLContext::Init()
    {
        //创建Glfw上下文
        glfwMakeContextCurrent(m_WindowHandle);
        //初始化Glad
        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        ENGINE_ASSERT(status, "Initialize Glad failed!");

        ENGINE_INFO("OpenGL Info:");
        ENGINE_INFO("  Vendor: {0}", (char*)glGetString(GL_VENDOR));
        ENGINE_INFO("  Renderer: {0}", (char*)glGetString(GL_RENDERER));
        ENGINE_INFO("  Version: {0}", (char*)glGetString(GL_VERSION));
    }

    void OpenGLContext::SwapBuffers()
    {
        glfwSwapBuffers(m_WindowHandle);
    }
}