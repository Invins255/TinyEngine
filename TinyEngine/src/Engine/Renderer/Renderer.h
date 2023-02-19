#pragma once

#include "Engine/Renderer/RendererAPI.h"
#include "Engine/Renderer/RenderPass.h"
#include "Engine/Renderer/RenderCommandQueue.h"
#include "Engine/Renderer/Mesh.h"


namespace Engine
{

#define RENDERCOMMAND_DEBUG 0
#if RENDERCOMMAND_DEBUG
#define RENDERCOMMAND_TRACE(...)		ENGINE_TRACE(__VA_ARGS__)
#else
#define RENDERCOMMAND_TRACE(...)
#endif 

	class ShaderLibrary;

	class Renderer
	{
	public:
		static RendererAPI::RendererAPIType GetAPIType() { return RendererAPI::GetAPIType(); }
		static RendererAPI& GetAPI();
		static ShaderLibrary& GetShaderLibrary();
		static RenderCommandQueue& GetCommandQueue();

		template<typename FuncT>
		static void Submit(FuncT&& func)
		{
			auto renderCmd = [](void* ptr) {
				auto pFunc = (FuncT*)ptr;
				(*pFunc)();
				pFunc->~FuncT();
			};
			auto storageBuffer = GetCommandQueue().Allocate(renderCmd, sizeof(func));
			new (storageBuffer) FuncT(std::forward<FuncT>(func));
		}

		static void Init();
		static void Shutdown();
		static void WaitAndRender();

		static void BeginRenderPass(const Ref<RenderPass>& renderPass);
		static void EndRenderPass();

		static void OnWindowResize(uint32_t width, uint32_t height);

		static void SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform, Ref<Pipeline> pipeline, Ref<MaterialInstance> overrideMaterial = nullptr);
		static void SubmitFullScreenQuad(uint32_t textureID, Ref<MaterialInstance> overrideMaterial = nullptr);
	};
}