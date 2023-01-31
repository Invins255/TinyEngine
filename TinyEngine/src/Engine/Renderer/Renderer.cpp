#include "pch.h"
#include "Renderer.h"

#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Renderer/SceneRenderer.h"

namespace Engine
{
	struct RendererData
	{
		Ref<RenderPass> m_ActiveRenderPass;
		RenderCommandQueue m_CommandQueue;
	};
	static Scope<RendererData> s_Data;

	void Renderer::Init()
	{
		s_Data = CreateScope<RendererData>();
		RenderCommand::Init();
		SceneRenderer::Init();
	}

	void Renderer::Shutdown()
	{
		SceneRenderer::Shutdown();
		s_Data.reset();
	}

	void Renderer::SetClearColor(float r, float g, float b, float a)
	{
		Renderer::Submit([=]()
			{
				RenderCommand::SetClearColor(r, g, b, a);
			}
		);
	}

	void Renderer::Clear()
	{
		Renderer::Submit([]()
			{
				RenderCommand::Clear();
			}
		);
	}

	void Renderer::WaitAndRender()
	{
		s_Data->m_CommandQueue.Execute();
	}

	void Renderer::BeginRenderPass(const Ref<RenderPass>& renderPass)
	{
		ENGINE_ASSERT(renderPass, "Render pass is nullptr!");
		s_Data->m_ActiveRenderPass = renderPass;
		s_Data->m_ActiveRenderPass->GetSpecification().TargetFramebuffer->Bind();
		const glm::vec4& clearColor = s_Data->m_ActiveRenderPass->GetSpecification().TargetFramebuffer->GetSpecification().ClearColor;
		Renderer::Submit([=]()
			{
				ENGINE_INFO("RenderCommand: Clear frameBuffer");
				RenderCommand::SetClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
				RenderCommand::Clear();
			}
		);
	}

	void Renderer::EndRenderPass()
	{
		ENGINE_ASSERT(s_Data->m_ActiveRenderPass, "No active render pass!");
		s_Data->m_ActiveRenderPass->GetSpecification().TargetFramebuffer->Unbind();
		s_Data->m_ActiveRenderPass = nullptr;
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	RenderCommandQueue& Renderer::GetCommandQueue()
	{
		return s_Data->m_CommandQueue;
	}
}