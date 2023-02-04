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
		SceneRenderer::Init();

		s_Data->m_ShaderLibrary = CreateRef<ShaderLibrary>();
		//TEMP
		s_Data->m_ShaderLibrary->Load("assets/Shaders/FlatColor3D.glsl");
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

	void Renderer::SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform, Ref<MaterialInstance> overrideMaterial)
	{
		mesh->m_Pipeline->Bind();
		mesh->m_VertexBuffer->Bind();
		mesh->m_IndexBuffer->Bind();

		auto materials = mesh->GetMaterials();
		for (Submesh& submesh : mesh->m_Submeshes)
		{
			//Material
			auto material = overrideMaterial ? overrideMaterial : materials[submesh.MaterialIndex];
			auto shader = material->GetShader();
			material->Bind();

			shader->SetMat4("u_Transform", transform * submesh.Transform);
			/*
			auto transformMatrix = transform * submesh.Transform;
			ENGINE_INFO("Transform: \n{0}, {1}, {2}, {3},\n{4}, {5}, {6}, {7},\n{8}, {9}, {10}, {11},\n{12}, {13}, {14}, {15}",
				transformMatrix[0][0], transformMatrix[0][1], transformMatrix[0][2], transformMatrix[0][3],
				transformMatrix[1][0], transformMatrix[1][1], transformMatrix[1][2], transformMatrix[1][3],
				transformMatrix[2][0], transformMatrix[2][1], transformMatrix[2][2], transformMatrix[2][3],
				transformMatrix[3][0], transformMatrix[3][1], transformMatrix[3][2], transformMatrix[3][3]);
			*/

			Renderer::Submit([submesh, material]
				{
					/*
					if (material->GetFlag(MaterialFlag::DepthTest))
						glEnable(GL_DEPTH_TEST);
					else
						glDisable(GL_DEPTH_TEST);
					
					if (material->GetFlag(MaterialFlag::TwoSided))
						glDisable(GL_CULL_FACE);
					else
						glEnable(GL_CULL_FACE);
					*/
					glEnable(GL_DEPTH_TEST);

					glDrawElementsBaseVertex(
						GL_TRIANGLES,
						submesh.IndexCount,
						GL_UNSIGNED_INT,
						(void*)(sizeof(uint32_t) * submesh.BaseIndex),
						submesh.BaseVertex
					);
					
				}
			);
		}
	}

	RenderCommandQueue& Renderer::GetCommandQueue()
	{
		return s_Data->m_CommandQueue;
	}
}