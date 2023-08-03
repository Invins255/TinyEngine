#include "pch.h"
#include "OpenGLRenderPass.h"

namespace Engine
{
	OpenGLRenderPass::OpenGLRenderPass(const RenderPassSpecification& spec)
		:m_Specification(spec)
	{
	}

	OpenGLRenderPass::~OpenGLRenderPass()
	{
	}
}