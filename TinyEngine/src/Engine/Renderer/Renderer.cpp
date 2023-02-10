#include "pch.h"
#include "Renderer.h"
#include "Engine/Renderer/SceneRenderer.h"

#include <glad/glad.h>

namespace Engine
{
	struct RendererData
	{
		Ref<RenderPass> m_ActiveRenderPass;
		RenderCommandQueue m_CommandQueue;
		Ref<ShaderLibrary> m_ShaderLibrary;
	};
	static Scope<RendererData> s_Data;

	Ref<ShaderLibrary> Renderer::GetShaderLibrary()
	{
		return s_Data->m_ShaderLibrary;
	}

	void Renderer::Init()
	{
		s_Data = CreateScope<RendererData>();		
		RenderCommand::Init();

		s_Data->m_ShaderLibrary = CreateRef<ShaderLibrary>();
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
				RENDERCOMMAND_INFO("RenderCommand: Clear current frameBuffer");
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

	void Renderer::SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform, Ref<MaterialInstance> overrideMaterial)
	{
		//顺序不可改变，否则导致顶点属性链接出错
		mesh->m_VertexBuffer->Bind();
		mesh->m_Pipeline->Bind();
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
					
					RENDERCOMMAND_INFO("RenderCommand: Submit mesh. Mesh: [{0}], Node: [{1}]", submesh.MeshName, submesh.NodeName);
				}
			);
		}
	}

	RenderCommandQueue& Renderer::GetCommandQueue()
	{
		return s_Data->m_CommandQueue;
	}
}