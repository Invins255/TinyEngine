#include "pch.h"
#include "SceneRenderer.h"
#include "Engine/Core/Core.h"
#include "Engine/Renderer/RenderPass.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Scene/Light.h"

namespace Engine
{
	struct SceneRendererData
	{
	    const Scene* m_ActiveScene = nullptr;
		struct SceneInfo
		{
			SceneRendererCamera SceneCamera;

			Light ActiveLight;
			LightEnvironment SceneLightEnvironment;

		}m_SceneData;

		//TEMP
		Ref<RenderPass> m_RenderPass;

		struct DrawCommand
		{
			Ref<Mesh> Mesh;
			Ref<MaterialInstance> Material;
			glm::mat4 Transform;
		};
		std::vector<DrawCommand> m_DrawList;

	};
	static Scope<SceneRendererData> s_Data;

	void SceneRenderer::Init()
	{		
		s_Data = CreateScope<SceneRendererData>();
		
		//TEMP
		FrameBufferSpecification geoFrameBufferSpec;
		geoFrameBufferSpec.Width = 1280;
		geoFrameBufferSpec.Height = 720;
		geoFrameBufferSpec.ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };		
		RenderPassSpecification geoRenderPassSpec;
		geoRenderPassSpec.TargetFramebuffer = FrameBuffer::Create(geoFrameBufferSpec);
		s_Data->m_RenderPass = RenderPass::Create(geoRenderPassSpec);
	}

	void SceneRenderer::Shutdown()
	{
		s_Data.reset();
	}

	void SceneRenderer::BeginScene(const Scene* scene, const SceneRendererCamera& camera)
	{
		ENGINE_ASSERT(scene, "Scene is nullptr!");

		s_Data->m_ActiveScene = scene;
		//Get scene data
		s_Data->m_SceneData.SceneCamera = camera;
		s_Data->m_SceneData.ActiveLight = scene->m_Light;
		s_Data->m_SceneData.SceneLightEnvironment = scene->m_LightEnvironment;
	}

	void SceneRenderer::EndScene()
	{
		ENGINE_ASSERT(s_Data->m_ActiveScene, "No active scene!");
		s_Data->m_ActiveScene = nullptr;

		FlushDrawList();
	}

	void SceneRenderer::SetViewportSize(uint32_t width, uint32_t height)
	{
		s_Data->m_RenderPass->GetSpecification().TargetFramebuffer->Resize(width, height);
	}

	void SceneRenderer::SubmitMesh(Ref<Mesh>& mesh, const glm::mat4& transform, Ref<MaterialInstance> overrideMaterial)
	{
		s_Data->m_DrawList.push_back({ mesh, overrideMaterial, transform });
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

		//Camera
		auto& sceneCamera = s_Data->m_SceneData.SceneCamera;
		auto viewProjection = sceneCamera.Camera.GetProjection() * sceneCamera.ViewMatrix;
		glm::vec3 cameraPosition = glm::inverse(sceneCamera.ViewMatrix)[3];

		//Render entities
		for (auto& dc : s_Data->m_DrawList)
		{
			auto baseMaterial = dc.Mesh->GetMaterial();
			baseMaterial->Set("u_ViewProjectionMatrix", viewProjection);
			baseMaterial->Set("u_CameraPosition", cameraPosition);
			//TODO: More uniforms 
			
			//Set lights
			auto directionalLight = s_Data->m_SceneData.SceneLightEnvironment.DirectionalLights[0];	//BUG: directionalLight方向可能出错
			baseMaterial->Set("u_DirectionalLights", directionalLight); 

			auto overrideMaterial = dc.Material;
			Renderer::SubmitMesh(dc.Mesh, dc.Transform, overrideMaterial);
		}

		Renderer::EndRenderPass();
	}

	void SceneRenderer::FlushDrawList()
	{
		ENGINE_ASSERT(!s_Data->m_ActiveScene, "");

		//TEMP
		RenderPass();

		s_Data->m_DrawList.clear();
	}
}