#include "pch.h"
#include "SceneRenderer.h"
#include "Engine/Core/Core.h"
#include "Engine/Renderer/RenderPass.h"
#include "Engine/Renderer/Renderer.h"

namespace Engine
{
	struct SceneRendererData
	{
		Scene* m_ActiveScene = nullptr;
		Ref<RenderPass> m_RenderPass;
	};
	static Scope<SceneRendererData> s_Data;

	void SceneRenderer::Init()
	{		
		s_Data = CreateScope<SceneRendererData>();
		
		FrameBufferSpecification geoFrameBufferSpec;
		geoFrameBufferSpec.Width = 1280;
		geoFrameBufferSpec.Height = 720;
		geoFrameBufferSpec.ClearColor = { 0.5f, 0.0f, 0.0f, 1.0f };		
		RenderPassSpecification geoRenderPassSpec;
		geoRenderPassSpec.TargetFramebuffer = FrameBuffer::Create(geoFrameBufferSpec);
		s_Data->m_RenderPass = RenderPass::Create(geoRenderPassSpec);
	}

	void SceneRenderer::Shutdown()
	{
		s_Data.reset();
	}

	void SceneRenderer::BeginScene(Scene* scene)
	{
		ENGINE_ASSERT(scene, "Scene is nullptr!");
		s_Data->m_ActiveScene = scene;
	}

	void SceneRenderer::EndScene()
	{
		ENGINE_ASSERT(s_Data->m_ActiveScene, "No active scene!");
		s_Data->m_ActiveScene = nullptr;

		RenderPass();
	}

	void SceneRenderer::SetViewportSize(uint32_t width, uint32_t height)
	{
		s_Data->m_RenderPass->GetSpecification().TargetFramebuffer->Resize(width, height);
	}

	void SceneRenderer::SubmitMesh(Ref<Mesh>& mesh, const glm::mat4& transform)
	{

	}

	uint32_t SceneRenderer::GetFinalColorBufferRendererID()
	{
		return s_Data->m_RenderPass->GetSpecification().TargetFramebuffer->GetColorAttachmentID();
	}

	Ref<FrameBuffer> SceneRenderer::GetFinalFrameBuffer()
	{
		return s_Data->m_RenderPass->GetSpecification().TargetFramebuffer;
	}

	void SceneRenderer::RenderPass()
	{
		Renderer::BeginRenderPass(s_Data->m_RenderPass);



		Renderer::EndRenderPass();
	}
}