#include "pch.h"
#include "SceneRenderer.h"
#include "Engine/Core/Core.h"
#include "Engine/Renderer/RenderPass.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Pipeline.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/Light.h"
#include "Engine/Renderer/MeshFactory.h"

namespace Engine
{
	struct SceneRendererData
	{
	    const Scene* m_ActiveScene = nullptr;
		struct SceneInfo
		{
			SceneRendererCamera SceneCamera;

			LightEnvironment SceneLightEnvironment;
			Environment SceneEnvironment;
			Ref<MaterialInstance> SkyboxMaterial;

		}m_SceneData;

		//TEMP
		Ref<RenderPass> m_ShadowMapPass;
		Ref<RenderPass> m_GeometryPass;
		Ref<RenderPass> m_CompositePass;

		Ref<Mesh> m_SkyboxMesh;

		struct DrawCommand
		{
			Ref<Mesh> Mesh;
			Ref<MaterialInstance> Material;
			glm::mat4 Transform;
		};
		std::vector<DrawCommand> m_DrawList;
		std::vector<DrawCommand> m_ShadowPassDrawList;

		//Pipeline
		Ref<Pipeline> m_Pipeline;
	};
	static Scope<SceneRendererData> s_Data;

	void SceneRenderer::Init()
	{		
		s_Data = CreateScope<SceneRendererData>();
		
		//ShadowMap pass
		FrameBufferSpecification shadowMapFrameBufferSpec;
		shadowMapFrameBufferSpec.Width = 2048;
		shadowMapFrameBufferSpec.Height = 2048;
		shadowMapFrameBufferSpec.ClearColor = { 0.0f,0.0f,0.0f,1.0f };
		shadowMapFrameBufferSpec.Attachments = { FrameBufferTextureFormat::DEPTH32F };
		RenderPassSpecification shadowMapRenderPassSpec;
		shadowMapRenderPassSpec.TargetFramebuffer = FrameBuffer::Create(shadowMapFrameBufferSpec);
		s_Data->m_ShadowMapPass = RenderPass::Create(shadowMapRenderPassSpec);
		//Geometry pass
		FrameBufferSpecification geoFrameBufferSpec;
		geoFrameBufferSpec.Width = 1280;
		geoFrameBufferSpec.Height = 720;
		geoFrameBufferSpec.ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
		geoFrameBufferSpec.Attachments = { FrameBufferTextureFormat::RGBA16F, FrameBufferTextureFormat::DEPTH24STENCIL8 };
		RenderPassSpecification geoRenderPassSpec;
		geoRenderPassSpec.TargetFramebuffer = FrameBuffer::Create(geoFrameBufferSpec);
		s_Data->m_GeometryPass = RenderPass::Create(geoRenderPassSpec);
		//Composite pass
		FrameBufferSpecification compFrameBufferSpec;
		compFrameBufferSpec.Width = 1280;
		compFrameBufferSpec.Height = 720;
		compFrameBufferSpec.ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
		compFrameBufferSpec.Attachments = { FrameBufferTextureFormat::RGBA16F };
		RenderPassSpecification compRenderPassSpec;
		compRenderPassSpec.TargetFramebuffer = FrameBuffer::Create(compFrameBufferSpec);
		s_Data->m_CompositePass = RenderPass::Create(compRenderPassSpec);

		s_Data->m_SkyboxMesh = MeshFactory::CreateBox({ 2.0f, 2.0f, 2.0f });

		//Create pipeline
		VertexBufferLayout vertexLayout;
		vertexLayout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float3, "a_Tangent" },
			{ ShaderDataType::Float3, "a_Binormal" },
			{ ShaderDataType::Float2, "a_TexCoord" },
		};
		PipelineSpecification spec;
		spec.Layout = vertexLayout;
		s_Data->m_Pipeline = Pipeline::Create(spec);
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
		s_Data->m_SceneData.SceneLightEnvironment = scene->m_LightEnvironment;
		s_Data->m_SceneData.SceneEnvironment = scene->m_Environment;
		s_Data->m_SceneData.SkyboxMaterial = scene->m_SkyboxMaterial;
	}

	void SceneRenderer::EndScene()
	{
		ENGINE_ASSERT(s_Data->m_ActiveScene, "No active scene!");
		s_Data->m_ActiveScene = nullptr;

		FlushDrawList();
	}

	void SceneRenderer::SetViewportSize(uint32_t width, uint32_t height)
	{
		s_Data->m_GeometryPass->GetSpecification().TargetFramebuffer->Resize(width, height);
		s_Data->m_CompositePass->GetSpecification().TargetFramebuffer->Resize(width, height);
	}

	void SceneRenderer::SubmitMesh(Ref<Mesh>& mesh, const glm::mat4& transform, Ref<MaterialInstance> overrideMaterial)
	{
		s_Data->m_DrawList.push_back({ mesh, overrideMaterial, transform });
		s_Data->m_ShadowPassDrawList.push_back({ mesh, overrideMaterial, transform });
	}

	uint32_t SceneRenderer::GetFinalColorBufferRendererID()
	{
		return s_Data->m_CompositePass->GetSpecification().TargetFramebuffer->GetColorAttachmentID();
	}

	Ref<FrameBuffer> SceneRenderer::GetFinalFrameBuffer()
	{
		return s_Data->m_CompositePass->GetSpecification().TargetFramebuffer;
	}

	void SceneRenderer::ShadowMapPass()
	{
		auto& directionalLights = s_Data->m_SceneData.SceneLightEnvironment.DirectionalLights;
		if (directionalLights[0].Intensity == 0.0f || !directionalLights[0].CastShadows)
		{
			//Clear shadow map
			Renderer::BeginRenderPass(s_Data->m_ShadowMapPass);
			Renderer::EndRenderPass();

			return;
		}

		Renderer::BeginRenderPass(s_Data->m_ShadowMapPass);

		//TODO: ShadowMap Pass

		Renderer::EndRenderPass();
	}

	void SceneRenderer::GeometryPass()
	{
		Renderer::BeginRenderPass(s_Data->m_GeometryPass);

		//Camera
		auto& sceneCamera = s_Data->m_SceneData.SceneCamera;
		auto viewProjection = sceneCamera.Camera.GetProjection() * sceneCamera.ViewMatrix;
		glm::vec3 cameraPosition = glm::inverse(sceneCamera.ViewMatrix)[3];

		//Render skybox
		if(s_Data->m_SceneData.SceneEnvironment.SkyboxMap)
		{
			s_Data->m_SceneData.SkyboxMaterial->Set("u_Skybox", s_Data->m_SceneData.SceneEnvironment.SkyboxMap);
			s_Data->m_SceneData.SkyboxMaterial->Set("u_ViewMatrix", glm::mat4(glm::mat3(sceneCamera.ViewMatrix)));
			s_Data->m_SceneData.SkyboxMaterial->Set("u_ProjectionMatrix", sceneCamera.Camera.GetProjection());
			
			Renderer::SubmitMesh(s_Data->m_SkyboxMesh, glm::mat4(1.0f), s_Data->m_Pipeline, s_Data->m_SceneData.SkyboxMaterial);
		}

		//Render entities
		for (auto& dc : s_Data->m_DrawList)
		{
			auto baseMaterial = dc.Mesh->GetMaterial();
			baseMaterial->Set("u_ViewProjectionMatrix", viewProjection);
			baseMaterial->Set("u_CameraPosition", cameraPosition);
			//TODO: More uniforms 
			
			//Set lights 
			//TODO: 目前只使用了1个方向光, 需要补充为4个
			auto directionalLight = s_Data->m_SceneData.SceneLightEnvironment.DirectionalLights[0];	//BUG: directionalLight方向可能出错
			baseMaterial->Set("u_DirectionalLight", directionalLight); 

			auto overrideMaterial = dc.Material;
			Renderer::SubmitMesh(dc.Mesh, dc.Transform, s_Data->m_Pipeline);
		}

		Renderer::EndRenderPass();
	}

	void SceneRenderer::CompositePass()
	{
		Renderer::BeginRenderPass(s_Data->m_CompositePass);

		Renderer::SubmitFullScreenQuad(s_Data->m_GeometryPass->GetSpecification().TargetFramebuffer->GetColorAttachmentID());

		Renderer::EndRenderPass();
	}

	void SceneRenderer::FlushDrawList()
	{
		ENGINE_ASSERT(!s_Data->m_ActiveScene, "No active scene!");

		ShadowMapPass();
		GeometryPass();
		CompositePass();

		s_Data->m_DrawList.clear();
		s_Data->m_ShadowPassDrawList.clear();
		s_Data->m_SceneData = {};
	}
}