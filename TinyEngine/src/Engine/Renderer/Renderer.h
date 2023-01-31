#pragma once

#include "Engine/Renderer/RenderPass.h"
#include "Engine/Renderer/RenderCommandQueue.h"
#include "Engine/Renderer/RenderCommand.h"

namespace Engine
{
	class Renderer
	{
	public:
		typedef void(*RenderCommandFn)(void*);

		static RendererAPI::RendererAPIType GetAPIType() { return RendererAPI::GetAPIType(); }

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
		static void SetClearColor(float r, float g, float b, float a = 1.0f);
		static void Clear();
		static void WaitAndRender();

		static void BeginRenderPass(const Ref<RenderPass>& renderPass);
		static void EndRenderPass();

		static void OnWindowResize(uint32_t width, uint32_t height);
	
	private:
		static RenderCommandQueue& GetCommandQueue();
	};
}