#pragma once

#include "Engine/Scene/Scene.h"
#include "Engine/Renderer/FrameBuffer.h"
#include "Engine/Renderer/Mesh.h"

namespace Engine
{
	class SceneRenderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(Scene* scene);
		static void EndScene();

		static void SetViewportSize(uint32_t width, uint32_t height);
		static void SubmitMesh(Ref<Mesh>& mesh, const glm::mat4& transform = glm::mat4(1.0f));

		static uint32_t GetFinalColorBufferRendererID();
		static Ref<FrameBuffer> GetFinalFrameBuffer();
	private:
		static void RenderPass();

		friend class Scene;
	};
}