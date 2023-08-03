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
		Ref<RenderPass> m_ShadowMapPasses[4];
		Ref<RenderPass> m_GeometryPass;
		Ref<RenderPass> m_CompositePass;

		Ref<Mesh> m_SkyboxMesh;

		Ref<Material> m_ShadowMapMaterial;
		uint32_t m_ShadowMapSampler;
		glm::mat4 m_LightSpaceMatrix;
		//CSM 
		float m_CascadeSplits[4];
		glm::mat4 m_LightCascadeMatrices[4];

		Ref<Texture2D> m_BRDFLUTMap;

		struct DrawCommand
		{
			Ref<Mesh> Mesh;
			glm::mat4 Transform;
			Ref<MaterialInstance> Material;
		};
		std::vector<DrawCommand> m_DrawList;
		std::vector<DrawCommand> m_ShadowPassDrawList;
		std::vector<DrawCommand> m_ColliderDrawList;

		//Pipeline
		Ref<Pipeline> m_SkyboxPipeline;
		Ref<Pipeline> m_ShadowMapPipeline;
		Ref<Pipeline> m_GeometryPipeline;
		Ref<Pipeline> m_ColliderPipeline;

		//Editor Material
		Ref<MaterialInstance> m_ColliderMaterial;
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

		for (int i = 0; i < 4; i++)
		{
			FrameBufferSpecification shadowMapFrameBufferSpec;
			shadowMapFrameBufferSpec.Width = 2048;
			shadowMapFrameBufferSpec.Height = 2048;
			shadowMapFrameBufferSpec.ClearColor = { 0.0f,0.0f,0.0f,1.0f };
			shadowMapFrameBufferSpec.Attachments = { FrameBufferTextureFormat::RGBA16F, FrameBufferTextureFormat::DEPTH32F };
			RenderPassSpecification shadowMapRenderPassSpec;
			shadowMapRenderPassSpec.TargetFramebuffer = FrameBuffer::Create(shadowMapFrameBufferSpec);
			s_Data->m_ShadowMapPasses[i] = RenderPass::Create(shadowMapRenderPassSpec);
		}

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
		//Collider pipeline
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
			s_Data->m_ColliderPipeline = Pipeline::Create(spec);
		}

		auto colliderShader = Renderer::GetShaderLibrary().Get("Collider");
		s_Data->m_ColliderMaterial = MaterialInstance::Create(Material::Create(colliderShader), "Collider");
		s_Data->m_ColliderMaterial->SetFlag(MaterialFlag::DepthTest, false);
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

	void SceneRenderer::SubmitColliderMesh(const BoxColliderComponent& component, const glm::mat4& parentTransform)
	{
		s_Data->m_ColliderDrawList.push_back({ component.DebugMesh, glm::translate(parentTransform, component.Offset), nullptr });
	}

	void SceneRenderer::SubmitColliderMesh(const SphereColliderComponent& component, const glm::mat4& parentTransform)
	{
		s_Data->m_ColliderDrawList.push_back({ component.DebugMesh, parentTransform, nullptr });
	}

	void SceneRenderer::SubmitColliderMesh(const CapsuleColliderComponent& component, const glm::mat4& parentTransform)
	{
		s_Data->m_ColliderDrawList.push_back({ component.DebugMesh, parentTransform, nullptr });
	}

	void SceneRenderer::SubmitColliderMesh(const MeshColliderComponent& component, const glm::mat4& parentTransform)
	{
		for (auto debugMesh : component.ProcessedMeshes)
			s_Data->m_ColliderDrawList.push_back({ debugMesh, parentTransform, nullptr });
	}

	uint32_t SceneRenderer::GetFinalColorBufferRendererID()
	{
		return s_Data->m_CompositePass->GetSpecification().TargetFramebuffer->GetColorAttachmentID();
	}

	Ref<FrameBuffer> SceneRenderer::GetFinalFrameBuffer()
	{
		return s_Data->m_CompositePass->GetSpecification().TargetFramebuffer;
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
		float farClip = 1500.0f;
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

		//Static cascade split
		//cascadeSplits[0] = 0.05f;
		//cascadeSplits[1] = 0.15f;
		//cascadeSplits[2] = 0.3f;
		//cascadeSplits[3] = 1.0f;

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

			for (int i = 0; i < 4; i++)
			{
				Renderer::BeginRenderPass(s_Data->m_ShadowMapPasses[i]);
				Renderer::EndRenderPass();
			}

			return;
		}

		{
			CascadeData cascade = CalculateCascade(directionalLights[0].Direction);
			glm::mat4 shadowMapVP = cascade.ViewProjection;

			s_Data->m_LightSpaceMatrix = shadowMapVP;

			Renderer::BeginRenderPass(s_Data->m_ShadowMapPass);
			for (auto& dc : s_Data->m_ShadowPassDrawList)
			{
				auto& material = s_Data->m_ShadowMapMaterial;
				auto mi = MaterialInstance::Create(material);
				mi->Set("u_ViewProjectionMatrix", shadowMapVP);

				Renderer::SubmitMesh(dc.Mesh, dc.Transform, s_Data->m_ShadowMapPipeline, mi);
			}
			Renderer::EndRenderPass();
		}

		
		{
			CascadeData cascades[4];
			CalculateCascades(cascades, directionalLights[0].Direction);
			for (int i = 0; i < 4; i++)
			{
				s_Data->m_CascadeSplits[i] = cascades[i].SplitDepth;
				s_Data->m_LightCascadeMatrices[i] = cascades[i].ViewProjection;

				glm::mat4 shadowMapVP = cascades[i].ViewProjection;
				Renderer::BeginRenderPass(s_Data->m_ShadowMapPasses[i]);
				for (auto& dc : s_Data->m_ShadowPassDrawList)
				{
					auto& material = s_Data->m_ShadowMapMaterial;
					auto mi = MaterialInstance::Create(material);
					mi->Set("u_ViewProjectionMatrix", shadowMapVP);

					Renderer::SubmitMesh(dc.Mesh, dc.Transform, s_Data->m_ShadowMapPipeline, mi);
				}
				Renderer::EndRenderPass();
			}
		}
		
	}

	void SceneRenderer::GeometryPass()
	{
		bool collider = !s_Data->m_ColliderDrawList.empty();

		if (collider)
		{
			Renderer::Submit([]() 
				{ 
					glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); 
				});
		}

		Renderer::BeginRenderPass(s_Data->m_GeometryPass);

		if (collider)
		{
			Renderer::Submit([]()
				{
					glStencilMask(0);
				});
		}

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
			baseMaterial->Set("u_ViewMatrix", sceneCamera.ViewMatrix);
			baseMaterial->Set("u_CameraPosition", cameraPosition);
			baseMaterial->Set("u_LightSpaceMatrix", s_Data->m_LightSpaceMatrix);
			baseMaterial->Set("u_LightCascadeMatrix0", s_Data->m_LightCascadeMatrices[0]);
			baseMaterial->Set("u_LightCascadeMatrix1", s_Data->m_LightCascadeMatrices[1]);
			baseMaterial->Set("u_LightCascadeMatrix2", s_Data->m_LightCascadeMatrices[2]);
			baseMaterial->Set("u_LightCascadeMatrix3", s_Data->m_LightCascadeMatrices[3]);
			baseMaterial->Set("u_CascadeSplits", glm::vec4(s_Data->m_CascadeSplits[0], s_Data->m_CascadeSplits[1], s_Data->m_CascadeSplits[2], s_Data->m_CascadeSplits[3]));
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

			auto res = baseMaterial->FindShaderResource("u_ShadowMapTextures");
			if (res)
			{
				auto reg = res->GetRegister();
				uint32_t texID[] =
				{
					s_Data->m_ShadowMapPasses[0]->GetSpecification().TargetFramebuffer->GetDepthAttachmentID(),
					s_Data->m_ShadowMapPasses[1]->GetSpecification().TargetFramebuffer->GetDepthAttachmentID(),
					s_Data->m_ShadowMapPasses[2]->GetSpecification().TargetFramebuffer->GetDepthAttachmentID(),
					s_Data->m_ShadowMapPasses[3]->GetSpecification().TargetFramebuffer->GetDepthAttachmentID()
				};

				Renderer::Submit([reg, texID]() mutable
					{
						glBindTextureUnit(reg++, texID[0]);
						glBindTextureUnit(reg++, texID[1]);
						glBindTextureUnit(reg++, texID[2]);
						glBindTextureUnit(reg++, texID[3]);
					});
			}

			Renderer::SubmitMesh(dc.Mesh, dc.Transform, s_Data->m_GeometryPipeline, dc.Material);
		}


		if (collider)
		{
			Renderer::Submit([]()
				{
					glStencilFunc(GL_ALWAYS, 1, 0xff);
					glStencilMask(0xff);
				});
		}

		//Render collider debug meshes
		if (collider)
		{
			Renderer::Submit([]()
				{
					glStencilFunc(GL_NOTEQUAL, 1, 0xff);
					glStencilMask(0);

					glLineWidth(1);
					glEnable(GL_LINE_SMOOTH);
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glDisable(GL_DEPTH_TEST);
				});

			s_Data->m_ColliderMaterial->Set("u_ViewProjection", viewProjection);
			for (auto& dc : s_Data->m_ColliderDrawList)
			{
				if (dc.Mesh)
					Renderer::SubmitMesh(dc.Mesh, dc.Transform, s_Data->m_GeometryPipeline, s_Data->m_ColliderMaterial);
			}

			Renderer::Submit([]()
				{
					glPointSize(1);
					glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
				});

			for (auto& dc : s_Data->m_ColliderDrawList)
			{
				if (dc.Mesh)
					Renderer::SubmitMesh(dc.Mesh, dc.Transform, s_Data->m_GeometryPipeline, s_Data->m_ColliderMaterial);
			}

			Renderer::Submit([]()
				{
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glStencilMask(0xff);
					glStencilFunc(GL_ALWAYS, 1, 0xff);
					glEnable(GL_DEPTH_TEST);
				});
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
		s_Data->m_ColliderDrawList.clear();
		s_Data->m_SceneData = {};
	}
}