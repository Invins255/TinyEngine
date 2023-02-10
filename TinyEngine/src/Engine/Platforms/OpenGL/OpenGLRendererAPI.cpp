#include "pch.h"
#include "OpenGLRendererAPI.h"

#include <glad/glad.h>

namespace Engine{
	static void OpenGLLogMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:
			ENGINE_ERROR("[OpenGL Debug HIGH] {0}", message);
			ENGINE_ASSERT(false, "GL_DEBUG_SEVERITY_HIGH");
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			ENGINE_WARN("[OpenGL Debug MEDIUM] {0}", message);
			break;
		case GL_DEBUG_SEVERITY_LOW:
			ENGINE_INFO("[OpenGL Debug LOW] {0}", message);
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			//ENGINE_TRACE("[OpenGL Debug NOTIFICATION] {0}", message);
			break;
		}
	}

	void OpenGLRendererAPI::Init()
	{
		//Debug Message
		glDebugMessageCallback(OpenGLLogMessage, nullptr);
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

		GLenum error = glGetError();
		while (error != GL_NO_ERROR)
		{
			ENGINE_ERROR("OpenGL Error: {0}", error);
			error = glGetError();
		}
	}

	void OpenGLRendererAPI::SetClearColor(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
	}

	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}
}