#include "pch.h"
#include "RenderCommand.h"

#include "Engine/Platforms/OpenGL/OpenGLRendererAPI.h"

namespace Engine
{
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI();
}