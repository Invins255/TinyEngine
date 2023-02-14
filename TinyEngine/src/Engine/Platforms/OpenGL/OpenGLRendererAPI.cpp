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

	static GLenum GetGLPrimitiveType(PrimitiveType type)
	{
		switch (type)
		{
		case Engine::PrimitiveType::Triangles:	return GL_TRIANGLES;
		case Engine::PrimitiveType::Lines:		return GL_LINES;
		default:
			ENGINE_ASSERT(false, "Unknown primitive type!");
			return 0;
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

	void OpenGLRendererAPI::DrawElements(uint32_t count, PrimitiveType type, bool depthTest)
	{
		if (!depthTest)
			glDisable(GL_DEPTH_TEST);

		glDrawElements(GetGLPrimitiveType(type), count, GL_UNSIGNED_INT, nullptr);

		if (!depthTest)
			glEnable(GL_DEPTH_TEST);
	}
}