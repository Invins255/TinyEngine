#include "pch.h"
#include "SceneRenderer.h"
#include "Engine/Core/Core.h"
#include "Engine/Core/Ref.h"
#include "Engine/Renderer/RenderPass.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Pipeline.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/Light.h"
#include "Engine/Renderer/MeshFactory.h"
#include "Engine/Asset/AssetManager.h"

#include <glad/glad.h>

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

		Ref<RenderPass> m_ShadowMapPass;
		Ref<RenderPass> m_GeometryPass;
		Ref<RenderPass> m_CompositePass;

		Ref<Mesh> m_SkyboxMesh;

		Ref<Material> m_ShadowMapMaterial;
		uint32_t m_ShadowMapSampler;
		glm::mat4 m_LightSpaceMatrix;

		Ref<Texture2D> m_BRDFLUTMap;

		struct DrawCommand
		{
			Ref<Mesh> Mesh;
			glm::mat4 Transform;
			Ref<MaterialInstance> Material;
		};
		std::vector<DrawCommand> m_DrawList;
		std::vector<DrawCommand> m_ShadowPassDrawList;

		//Pipeline
		Ref<Pipeline> m_SkyboxPipeline;
		Ref<Pipeline> m_ShadowMapPipeline;
		Ref<Pipeline> m_GeometryPipeline;
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
		shadowMapFrameBufferSpec.Attachments = { FrameBufferTextureFormat::RGBA16F, FrameBufferTextureFormat::DEPTH32F };
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

		auto shadowMapShader = Renderer::GetShaderLibrary().Get("ShadowMap");
		s_Data->m_ShadowMapMaterial = Material::Create(shadowMapShader);
		s_Data->m_ShadowMapMaterial->SetFlags(MaterialFlag::DepthTest);

		s_Data->m_BRDFLUTMap = AssetManager::CreateNewAsset<Texture2D>("assets\\textures\\IBL_BRDF_LUT.png", true);

		Renderer::Submit([]()
			{
				glGenSamplers(1, &(s_Data->m_ShadowMapSampler));

				// Setup the shadowmap depth sampler
				glSamplerParameteri(s_Data->m_ShadowMapSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glSamplerParameteri(s_Data->m_ShadowMapSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glSamplerParameteri(s_Data->m_ShadowMapSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glSamplerParameteri(s_Data->m_ShadowMapSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			});

		//ShadowMap pipeline
		{
			VertexBufferLayout vertexLayout;
			vertexLayout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float3, "a_Normal" },
				{ ShaderDataType::Float3, "a_Tangent" },
				{ ShaderDataType::Float3, "a_Binormal" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			};
			PipelineSpecification spec;
			spec.Layout = vertexLayout;
			s_Data->m_ShadowMapPipeline = Pipeline::Create(spec);
		}
		//Skybox pipeline
		{
			VertexBufferLayout vertexLayout;
			vertexLayout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float3, "a_Normal" },
				{ ShaderDataType::Float3, "a_Tangent" },
				{ ShaderDataType::Float3, "a_Binormal" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			};
			PipelineSpecification spec;
			spec.Layout = vertexLayout;
			s_Data->m_SkyboxPipeline = Pipeline::Create(spec);
		}
		//Geometry pipeline
		{
			VertexBufferLayout vertexLayout;
			vertexLayout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float3, "a_Normal" },
				{ ShaderDataType::Float3, "a_Tangent" },
				{ ShaderDataType::Float3, "a_Binormal" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			};
			PipelineSpecification spec;
			spec.Layout = vertexLayout;
			s_Data->m_GeometryPipeline = Pipeline::Create(spec);
		}
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
		s_Data->m_DrawList.push_back({ mesh, transform, overrideMaterial });
		s_Data->m_ShadowPassDrawList.push_back({ mesh, transform, overrideMaterial });
	}

	uint32_t SceneRenderer::GetFinalColorBufferRendererID()
	{
		//return s_Data->m_CompositePass->GetSpecification().TargetFramebuffer->GetColorAttachmentID();
		return s_Data->m_ShadowMapPass->GetSpecification().TargetFramebuffer->GetDepthAttachmentID();
	}

	Ref<FrameBuffer> SceneRenderer::GetFinalFrameBuffer()
	{
		//return s_Data->m_CompositePass->GetSpecification().TargetFramebuffer;
		return s_Data->m_ShadowMapPass->GetSpecification().TargetFramebuffer;
	}

	struct FrustumBounds
	{
		float Right, Left, Bottom, Top, FarClip, NearClip;
	};

	struct CascadeData
	{
		glm::mat4 ViewProjection;
		glm::mat4 View;
		float SplitDepth;
	};

	static CascadeData CalculateCascade(const glm::vec3& lightDirection)
	{
		auto& sceneCamera = s_Data->m_SceneData.SceneCamera;
		auto viewProjection = sceneCamera.Camera.GetProjection() * sceneCamera.ViewMatrix;

		// TODO: less hard-coding!
		float nearClip = 0.01f;
		float farClip = 1000.0f;
		float clipRange = farClip - nearClip;

		float minZ = nearClip;
		float maxZ = nearClip + clipRange;

		float range = maxZ - minZ;
		float ratio = maxZ / minZ;

		// Calculate orthographic projection matrix for each cascade
		float lastSplitDist = 0.0;
		float splitDist = 0.1f;
		glm::vec3 frustumCorners[8] =
		{
			glm::vec3(-1.0f,  1.0f, -1.0f),
			glm::vec3(1.0f,  1.0f, -1.0f),
			glm::vec3(1.0f, -1.0f, -1.0f),
			glm::vec3(-1.0f, -1.0f, -1.0f),
			glm::vec3(-1.0f,  1.0f,  1.0f),
			glm::vec3(1.0f,  1.0f,  1.0f),
			glm::vec3(1.0f, -1.0f,  1.0f),
			glm::vec3(-1.0f, -1.0f,  1.0f),
		};
		// Project frustum corners into world space
		glm::mat4 invCam = glm::inverse(viewProjection);
		for (uint32_t i = 0; i < 8; i++)
		{
			glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
			frustumCorners[i] = invCorner / invCorner.w;
		}

		for (uint32_t i = 0; i < 4; i++)
		{
			glm::vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
			frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
			frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
		}

		// Get frustum center
		glm::vec3 frustumCenter = glm::vec3(0.0f);
		for (uint32_t i = 0; i < 8; i++)
			frustumCenter += frustumCorners[i];
		frustumCenter /= 8.0f;

		float radius = 0.0f;
		for (uint32_t i = 0; i < 8; i++)
		{
			float distance = glm::length(frustumCorners[i] - frustumCenter);
			radius = glm::max(radius, distance);
		}
		radius = std::ceil(radius * 16.0f) / 16.0f;

		glm::vec3 maxExtents = glm::vec3(radius);
		glm::vec3 minExtents = -maxExtents;

		//Light matrix
		glm::vec3 lightDir = -lightDirection;
		glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);

		CascadeData cascade;
		cascade.SplitDepth = (nearClip + splitDist * clipRange) * -1.0f;
		cascade.View = lightViewMatrix;
		cascade.ViewProjection = lightOrthoMatrix * lightViewMatrix;
		return cascade;
	}

	static void CalculateCascades(CascadeData* cascades, const glm::vec3& lightDirection)
	{
		auto& sceneCamera = s_Data->m_SceneData.SceneCamera;
		auto viewProjection = sceneCamera.Camera.GetProjection() * sceneCamera.ViewMatrix;

		float nearClip = 0.1f;
		float farClip = 1000.0f;
		float clipRange = farClip - nearClip;

		float minZ = nearClip;
		float maxZ = nearClip + clipRange;

		float range = maxZ - minZ;
		float ratio = maxZ / minZ;

		const int SHADOW_MAP_CASCADE_COUNT = 4;
		float cascadeSplits[SHADOW_MAP_CASCADE_COUNT];
		const float cascadeSplitLambda = 0.91f;

		//Calculate split depths based on view camera frustum
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			float p = (i + 1) / static_cast<float>(SHADOW_MAP_CASCADE_COUNT);
			float log = minZ * std::pow(ratio, p);
			float uniform = minZ + p * range;
			float d = cascadeSplitLambda * log + (1.0 - cascadeSplitLambda) * uniform;
			cascadeSplits[i] = (d - nearClip) / clipRange;
		}

		//Calculate orthographic projection matrix for each cascade
		float lastSplitDist = 0.0;
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			float splitDist = cascadeSplits[i];

			glm::vec3 frustumCorners[8] =
			{
				glm::vec3(-1.0f,  1.0f, -1.0f),
				glm::vec3(1.0f,  1.0f, -1.0f),
				glm::vec3(1.0f, -1.0f, -1.0f),
				glm::vec3(-1.0f, -1.0f, -1.0f),
				glm::vec3(-1.0f,  1.0f,  1.0f),
				glm::vec3(1.0f,  1.0f,  1.0f),
				glm::vec3(1.0f, -1.0f,  1.0f),
				glm::vec3(-1.0f, -1.0f,  1.0f),
			};

			// Project frustum corners into world space
			glm::mat4 invCam = glm::inverse(viewProjection);
			for (uint32_t i = 0; i < 8; i++)
			{
				glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
				frustumCorners[i] = invCorner / invCorner.w;
			}

			for (uint32_t i = 0; i < 4; i++)
			{
				glm::vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
				frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
				frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
			}

			// Get frustum center
			glm::vec3 frustumCenter = glm::vec3(0.0f);
			for (uint32_t i = 0; i < 8; i++)
				frustumCenter += frustumCorners[i];
			frustumCenter /= 8.0f;

			//Calculate light frustum size
			float radius = 0.0f;
			for (uint32_t i = 0; i < 8; i++)
			{
				float distance = glm::length(frustumCorners[i] - frustumCenter);
				radius = glm::max(radius, distance);
			}
			radius = std::ceil(radius * 16.0f) / 16.0f;

			glm::vec3 maxExtents = glm::vec3(radius);
			glm::vec3 minExtents = -maxExtents;

			glm::vec3 lightDir = -lightDirection;
			glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.001f, maxExtents.z - minExtents.z);

			cascades[i].SplitDepth = (nearClip + splitDist * clipRange) * -1.0f;
			cascades[i].ViewProjection = lightOrthoMatrix * lightViewMatrix;
			cascades[i].View = lightViewMatrix;

			lastSplitDist = cascadeSplits[i];
		}
	}

	void SceneRenderer::ShadowMapPass()
	{
		//Only use the first directional light to calculate shadow map
		auto& directionalLights = s_Data->m_SceneData.SceneLightEnvironment.DirectionalLights;
		if (directionalLights[0].Intensity == 0.0f || !directionalLights[0].CastShadows)
		{
			//Clear shadow map
			Renderer::BeginRenderPass(s_Data->m_ShadowMapPass);
			Renderer::EndRenderPass();
			return;
		}

		CascadeData cascade = CalculateCascade(directionalLights[0].Direction);
		glm::mat4 shadowMapVP = cascade.ViewProjection;
		
		//CascadeData cascades[4];
		//CalculateCascades(cascades, directionalLights[0].Direction);
		//glm::mat4 shadowMapVP = cascades[2].ViewProjection;

		s_Data->m_LightSpaceMatrix = shadowMapVP;

		Renderer::BeginRenderPass(s_Data->m_ShadowMapPass);			
		for (auto& dc : s_Data->m_ShadowPassDrawList)
		{
			auto material = s_Data->m_ShadowMapMaterial;
			material->Set("u_ViewProjectionMatrix", shadowMapVP);
			auto mi = MaterialInstance::Create(material);

			Renderer::SubmitMesh(dc.Mesh, dc.Transform, s_Data->m_ShadowMapPipeline, mi);
		}	

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
			
			Renderer::SubmitMesh(s_Data->m_SkyboxMesh, glm::mat4(1.0f), s_Data->m_SkyboxPipeline, s_Data->m_SceneData.SkyboxMaterial);
		}

		//Render entities
		for (auto& dc : s_Data->m_DrawList)
		{
			auto baseMaterial = dc.Mesh->GetMaterial();
			baseMaterial->Set("u_ViewProjectionMatrix", viewProjection);
			baseMaterial->Set("u_CameraPosition", cameraPosition);
			baseMaterial->Set("u_LightSpaceMatrix", s_Data->m_LightSpaceMatrix);
			//TODO: More uniforms 
			
			//Set lights 
			//TODO: 目前只使用了1个方向光, 需要补充为4个; 部分变量不需要对每个mesh都进行设置, 可以移出循环
			auto directionalLight = s_Data->m_SceneData.SceneLightEnvironment.DirectionalLights[0];
			baseMaterial->Set("u_DirectionalLight", directionalLight); 
			
			//Set environment
			baseMaterial->Set("u_IrradianceMap", s_Data->m_SceneData.SceneEnvironment.IrradianceMap);
			baseMaterial->Set("u_EnvPrefliteredMap", s_Data->m_SceneData.SceneEnvironment.PrefliteredMap);
			baseMaterial->Set("u_BRDFLUTMap", s_Data->m_BRDFLUTMap);

			//Shadow map
			auto resource = baseMaterial->FindShaderResource("u_ShadowMapTexture");
			if (resource)
			{
				auto reg = resource->GetRegister();
				uint32_t texID = s_Data->m_ShadowMapPass->GetSpecification().TargetFramebuffer->GetDepthAttachmentID();
				
				Renderer::Submit([reg, texID]() mutable
					{
						glBindTextureUnit(reg, texID);
					});
			}

			Renderer::SubmitMesh(dc.Mesh, dc.Transform, s_Data->m_GeometryPipeline, dc.Material);
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

		Renderer::Submit([]() {RENDERCOMMAND_TRACE("RenderCommand: ShadowMapPass Begin:"); });
		ShadowMapPass();
		Renderer::Submit([]() {RENDERCOMMAND_TRACE("RenderCommand: ShadowMapPass End"); });

		Renderer::Submit([]() {RENDERCOMMAND_TRACE("RenderCommand: GeometryPass Begin:"); });
		GeometryPass();
		Renderer::Submit([]() {RENDERCOMMAND_TRACE("RenderCommand: GeometryPass End"); });

		Renderer::Submit([]() {RENDERCOMMAND_TRACE("RenderCommand: CompositePass Begin:"); });
		CompositePass();
		Renderer::Submit([]() {RENDERCOMMAND_TRACE("RenderCommand: CompositePass End"); });

		s_Data->m_DrawList.clear();
		s_Data->m_ShadowPassDrawList.clear();
		s_Data->m_SceneData = {};
	}
}