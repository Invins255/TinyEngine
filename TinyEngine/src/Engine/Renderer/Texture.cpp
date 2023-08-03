#include "pch.h"
#include "Texture.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Platforms/OpenGL/OpenGLTexture.h"

namespace Engine
{
	Ref<Texture2D> Texture2D::Create(const std::string& path, bool srgb, TextureSpecification spec)
	{
		Ref<Texture2D> result = nullptr;
		switch (Renderer::GetAPIType())
		{
		case RendererAPI::RendererAPIType::None:
			ENGINE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::RendererAPIType::OpenGL:
			result = CreateRef<OpenGLTexture2D>(path, srgb, spec);
			return result;
		default:
			ENGINE_ASSERT(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}

	Ref<Texture2D> Texture2D::Create(TextureFormat format, uint32_t width, uint32_t height, TextureSpecification spec)
	{
		Ref<Texture2D> result = nullptr;
		switch (Renderer::GetAPIType())
		{
		case RendererAPI::RendererAPIType::None:
			ENGINE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::RendererAPIType::OpenGL:
			result = CreateRef<OpenGLTexture2D>(format, width, height, spec);
			return result;
		default:
			ENGINE_ASSERT(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}

	uint32_t Texture::GetBPP(TextureFormat format)
	{
		switch (format)
		{
		case Engine::TextureFormat::RG:		
		case Engine::TextureFormat::RG16F:		return 2;
		case Engine::TextureFormat::RGB:		return 3;
		case Engine::TextureFormat::RGBA:
		case Engine::TextureFormat::RGBA16F:	return 4;
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

	Ref<TextureCube> TextureCube::Create(const std::string& path, TextureSpecification spec)
	{
		Ref<TextureCube> result = nullptr;
		switch (Renderer::GetAPIType())
		{
		case RendererAPI::RendererAPIType::None:
			ENGINE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::RendererAPIType::OpenGL:
			result = CreateRef<OpenGLTextureCube>(path, spec);
			return result;
		default:
			ENGINE_ASSERT(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}

	Ref<TextureCube> TextureCube::Create(TextureFormat format, uint32_t width, uint32_t height)
	{
		Ref<TextureCube> result = nullptr;
		switch (Renderer::GetAPIType())
		{
		case RendererAPI::RendererAPIType::None:
			ENGINE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::RendererAPIType::OpenGL:
			result = CreateRef<OpenGLTextureCube>(format, width, height);
			return result;
		default:
			ENGINE_ASSERT(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}
}