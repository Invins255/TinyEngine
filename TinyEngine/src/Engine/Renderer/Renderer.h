#pragma once

#include "Engine/Renderer/RenderCommand.h"

#include "Engine/Renderer/OrthographicCamera.h"
#include "Engine/Renderer/Shader.h"

namespace Engine
{
	class Renderer
	{
	public:
		static void Init();

		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		static void BeginScene(OrthographicCamera& camera);
		static void EndScene();
		static void Submit(
			const Ref<VertexArray>& vertexArray,
			const Ref<Shader>& shader,
			const glm::mat4& transform = glm::mat4(1.0f));

		static void OnWindowResize(uint32_t width, uint32_t height);
	
	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static SceneData* m_SceneData;
	};
}