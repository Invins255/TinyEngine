#include "pch.h"
#include "Renderer.h"
#include "Engine/Platforms/OpenGL/OpenGLRendererAPI.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Renderer/VertexBuffer.h"
#include "Engine/Renderer/IndexBuffer.h"
#include "Engine/Renderer/VertexArray.h"
#include "Engine/Renderer/Pipeline.h"

#include <glad/glad.h>

namespace Engine
{
	struct RendererData
	{
		Scope<RendererAPI> m_RendererAPI;
		Scope<RenderCommandQueue> m_CommandQueue;
		Scope<ShaderLibrary> m_ShaderLibrary;
		Ref<RenderPass> m_ActiveRenderPass;

		Ref<VertexBuffer> m_FullScreenQuadVertexBuffer;
		Ref<IndexBuffer> m_FullScreenQuadIndexBuffer;
		Ref<VertexArray> m_FullScreenQuadVertexArray;
		Ref<Pipeline> m_FullScreenQuadPipeline;
		Ref<MaterialInstance> m_FullScreenQuadMaterial;
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
		
		//Load shader
		s_Data->m_ShaderLibrary->Load("assets/shaders/PBR.glsl");
		s_Data->m_ShaderLibrary->Load("assets/shaders/Skybox.glsl");
		s_Data->m_ShaderLibrary->Load("assets/shaders/FullScreenQuad.glsl");
		s_Data->m_ShaderLibrary->Load("assets/shaders/ShadowMap.glsl");

		SceneRenderer::Init();

		//Create full screen quad
		float x = -1;
		float y = -1;
		float width = 2, height = 2;
		struct QuadVertex
		{
			glm::vec3 Position;
			glm::vec2 TexCoord;
		};

		QuadVertex* data = new QuadVertex[4];
		data[0].Position = glm::vec3(x, y, 0.0f);
		data[0].TexCoord = glm::vec2(0, 0);
		data[1].Position = glm::vec3(x + width, y, 0.0f);
		data[1].TexCoord = glm::vec2(1, 0);
		data[2].Position = glm::vec3(x + width, y + height, 0.0f);
		data[2].TexCoord = glm::vec2(1, 1);
		data[3].Position = glm::vec3(x, y + height, 0.0f);
		data[3].TexCoord = glm::vec2(0, 1);

		uint32_t indices[6] = { 0, 1, 2, 2, 3, 0, };

		s_Data->m_FullScreenQuadVertexBuffer = VertexBuffer::Create(data, 4 * sizeof(QuadVertex));
		s_Data->m_FullScreenQuadIndexBuffer = IndexBuffer::Create(indices, 6 * sizeof(uint32_t));
		s_Data->m_FullScreenQuadVertexArray = VertexArray::Create();

		PipelineSpecification spec;
		spec.Layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		};
		s_Data->m_FullScreenQuadPipeline = Pipeline::Create(spec);

		auto shader = GetShaderLibrary().Get("FullScreenQuad");
		s_Data->m_FullScreenQuadMaterial = MaterialInstance::Create(Material::Create(shader));
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
		auto& frameBuffer = s_Data->m_ActiveRenderPass->GetSpecification().TargetFramebuffer;
		frameBuffer->Bind();
		const uint32_t width = frameBuffer->GetWidth();
		const uint32_t height = frameBuffer->GetHeight();
		const glm::vec4& clearColor = frameBuffer->GetSpecification().ClearColor;
		Renderer::Submit([=]()
			{
				RENDERCOMMAND_TRACE("RenderCommand: Clear current frameBuffer");
				s_Data->m_RendererAPI->SetViewport(0, 0, width, height);
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

					RENDERCOMMAND_TRACE("RenderCommand: Submit mesh. Mesh: '{0}', Node: '{1}'", submesh.MeshName, submesh.NodeName);
				}
			);
		}
	}

	void Renderer::SubmitFullScreenQuad(uint32_t textureID, Ref<MaterialInstance> overrideMaterial)
	{
		s_Data->m_FullScreenQuadVertexBuffer->Bind();
		s_Data->m_FullScreenQuadVertexArray->Bind();
		s_Data->m_FullScreenQuadPipeline->BindVertexLayout();
		s_Data->m_FullScreenQuadIndexBuffer->Bind();

		auto& material = overrideMaterial ? overrideMaterial : s_Data->m_FullScreenQuadMaterial;
		material->Bind();

		Renderer::Submit([=]()
			{
				glBindTexture(GL_TEXTURE_2D, textureID);

				s_Data->m_RendererAPI->DrawElements(6, PrimitiveType::Triangles, false);

				RENDERCOMMAND_TRACE("RenderCommand: Sumbit fullscreen quad");
			});
	}

}