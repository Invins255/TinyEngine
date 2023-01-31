#pragma once

#include "Engine/Renderer/Texture.h"

#include <glad/glad.h>

namespace Engine
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::string& path);
		OpenGLTexture2D(uint32_t width, uint32_t height);
		virtual ~OpenGLTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; };

		virtual uint32_t GetRendererID() const { return m_RendererID; };
		virtual void Bind(uint32_t slot = 0) const override;

		virtual void SetData(void* data, uint32_t size) override;

		virtual bool operator==(const Texture& other) override 
		{ return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID; };
	private:
		uint32_t m_RendererID = 0;
		std::string m_Path;
		uint32_t m_Width, m_Height;
		GLenum m_InternalFormat, m_DataFormat;
	};
}