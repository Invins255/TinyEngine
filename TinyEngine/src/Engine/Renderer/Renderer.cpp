#include "pch.h"
#include "Renderer.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Platforms/OpenGL/OpenGLRendererAPI.h"

#include <glad/glad.h>

namespace Engine
{
	struct RendererData
	{
		Scope<RendererAPI> m_RendererAPI;
		Scope<RenderCommandQueue> m_CommandQueue;
		Scope<ShaderLibrary> m_ShaderLibrary;
		Ref<RenderPass> m_ActiveRenderPass;
	};
	static Scope<RendererData> s_Data;

	RendererAPI& Renderer::GetAPI()
	{
		return *(s_Data->m_RendererAPI);
	}

	ShaderLibrary& Renderer::GetShaderLibrary()
	{
		return *(s_Data->m_ShaderLibrary);
	}

	RenderCommandQueue& Renderer::GetCommandQueue()
	{
		return *(s_Data->m_CommandQueue);
	}

	void Renderer::Init()
	{
		s_Data = CreateScope<RendererData>();
		
		//Initialize rendererAPI
		s_Data->m_RendererAPI = CreateScope<OpenGLRendererAPI>();
		s_Data->m_RendererAPI->Init();

		s_Data->m_CommandQueue = CreateScope<RenderCommandQueue>();
		s_Data->m_ShaderLibrary = CreateScope<ShaderLibrary>();
		
		//TEMP
		s_Data->m_ShaderLibrary->Load("assets/shaders/BlinnPhong.glsl");
		s_Data->m_ShaderLibrary->Load("assets/shaders/Skybox.glsl");

		SceneRenderer::Init();
	}

	void Renderer::Shutdown()
	{
		SceneRenderer::Shutdown();
		s_Data.reset();
	}

	void Renderer::WaitAndRender()
	{
		s_Data->m_CommandQueue->Execute();
	}

	void Renderer::BeginRenderPass(const Ref<RenderPass>& renderPass)
	{
		ENGINE_ASSERT(renderPass, "Render pass is nullptr!");
		s_Data->m_ActiveRenderPass = renderPass;
		s_Data->m_ActiveRenderPass->GetSpecification().TargetFramebuffer->Bind();
		const glm::vec4& clearColor = s_Data->m_ActiveRenderPass->GetSpecification().TargetFramebuffer->GetSpecification().ClearColor;
		Renderer::Submit([=]()
			{
				RENDERCOMMAND_TRACE("RenderCommand: Clear current frameBuffer");
				s_Data->m_RendererAPI->SetClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
				s_Data->m_RendererAPI->Clear();
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
		s_Data->m_RendererAPI->SetViewport(0, 0, width, height);
	}

	void Renderer::SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform, Ref<Pipeline> pipeline, Ref<MaterialInstance> overrideMaterial)
	{
		mesh->m_VertexBuffer->Bind();
		mesh->m_VertexArray->Bind();
		pipeline->BindVertexLayout();
		mesh->m_IndexBuffer->Bind();

		auto materials = mesh->GetMaterials();
		for (Submesh& submesh : mesh->m_Submeshes)
		{
			//Material
			auto material = overrideMaterial ? overrideMaterial : materials[submesh.MaterialIndex];
			auto shader = material->GetShader();
			material->Set("u_Transform", transform * submesh.Transform);
			material->Bind();

			Renderer::Submit([submesh, material]
				{
					if (material->GetFlag(MaterialFlag::DepthTest))
						glEnable(GL_DEPTH_TEST);
					else
						glDisable(GL_DEPTH_TEST);
					/*
					if (material->GetFlag(MaterialFlag::TwoSided))
						glDisable(GL_CULL_FACE);
					else
						glEnable(GL_CULL_FACE);
					*/
					glDrawElementsBaseVertex(
						GL_TRIANGLES,
						submesh.IndexCount,
						GL_UNSIGNED_INT,
						(void*)(sizeof(uint32_t) * submesh.BaseIndex),
						submesh.BaseVertex
					);

					RENDERCOMMAND_TRACE("RenderCommand: Submit mesh. Mesh: [{0}], Node: [{1}]", submesh.MeshName, submesh.NodeName);
				}
			);
		}
	}
}