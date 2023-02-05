#include "pch.h"
#include "Texture.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Platforms/OpenGL/OpenGLTexture.h"

namespace Engine
{

	Ref<Texture2D> Texture2D::Create(const std::string& path, bool srgb)
	{
		switch (Renderer::GetAPIType())
		{
		case RendererAPI::RendererAPIType::None:
			ENGINE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::RendererAPIType::OpenGL:
			return CreateRef<OpenGLTexture2D>(path, srgb);
		default:
			ENGINE_ASSERT(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}

	Ref<Texture2D> Texture2D::Create(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap)
	{
		switch (Renderer::GetAPIType())
		{
		case RendererAPI::RendererAPIType::None:
			ENGINE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::RendererAPIType::OpenGL:
			return CreateRef<OpenGLTexture2D>(format, width, height, wrap);
		default:
			ENGINE_ASSERT(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}

	uint32_t Texture::GetBPP(TextureFormat format)
	{
		switch (format)
		{
		case Engine::TextureFormat::RGB:	return 3;
		case Engine::TextureFormat::RGBA:	return 4;
		}
		ENGINE_ASSERT(false, "Unknown texture format!");
		return 0;
	}

	uint32_t Texture::CalculateMipMapCount(uint32_t width, uint32_t height)
	{
		uint32_t levels = 1;
		while ((width | height) >> levels)
			levels++;
		return levels;
	}
}