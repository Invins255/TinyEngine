#pragma once

#include "Engine/Scene/Scene.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Renderer/FrameBuffer.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Material.h"

namespace Engine
{
	struct SceneRendererCamera
	{
		Camera Camera;
		glm::mat4 ViewMatrix;
	};

	class SceneRenderer
	{
		friend class Scene;

	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Scene* scene, const SceneRendererCamera& camera);
		static void EndScene();

		static void SetViewportSize(uint32_t width, uint32_t height);
		static void SubmitMesh(Ref<Mesh>& mesh, const glm::mat4& transform = glm::mat4(1.0f), Ref<MaterialInstance> overrideMaterial = nullptr);

		static uint32_t GetFinalColorBufferRendererID();
		static Ref<FrameBuffer> GetFinalFrameBuffer();
	private:
		static void ShadowMapPass();
		static void GeometryPass();
		static void CompositePass();

		static void FlushDrawList();
	};
}