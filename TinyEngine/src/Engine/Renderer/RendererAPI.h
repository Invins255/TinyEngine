#pragma once

namespace Engine
{
	//TODO: More type
	enum class PrimitiveType
	{
		None = 0, 
		Triangles, 
		Lines
	};

	class RendererAPI
	{
	public:
		enum class RendererAPIType
		{
			None = 0, OpenGL = 1
		};
	public:
		static RendererAPIType GetAPIType() { return s_API; }

	public:
		virtual ~RendererAPI() = default;

		virtual void Init() = 0;
		virtual void SetClearColor(float r, float g, float b, float a) = 0;
		virtual void Clear() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;		
		virtual void DrawElements(uint32_t count, PrimitiveType type, bool depthTest = true) = 0;

	private:
		static RendererAPIType s_API;
	};
}