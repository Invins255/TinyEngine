#pragma once

namespace Engine
{
	class RendererAPI
	{
	public:
		enum class RendererAPIType
		{
			None = 0, OpenGL = 1
		};
	public:
		virtual ~RendererAPI() = default;

		virtual void Init() = 0;
		virtual void SetClearColor(float r, float g, float b, float a) = 0;
		virtual void Clear() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		
		static RendererAPIType GetAPIType() { return s_API; }
	private:
		static RendererAPIType s_API;
	};
}