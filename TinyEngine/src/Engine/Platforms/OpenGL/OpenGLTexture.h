#pragma once

#include "Engine/Core/Buffer.h"
#include "Engine/Renderer/Texture.h"

namespace Engine
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::string& path, bool srgb);
		OpenGLTexture2D(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap);
		virtual ~OpenGLTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; };
		virtual uint32_t GetChannels() const override { return m_Channels; }
		virtual uint32_t GetMipLevelCount() const override;
		virtual TextureFormat GetFormat() const override { return m_Format; }
		virtual const std::string& GetPath() const override { return m_Path; }

		virtual bool IsLoaded() const override { return m_Loaded; }

		virtual uint32_t GetRendererID() const { return m_RendererID; };
		virtual void Bind(uint32_t slot = 0) const override;

		virtual void Lock() override;
		virtual void Unlock() override;

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual Buffer& GetWritableBuffer() override;

		virtual bool operator==(const Texture& other) override 
		{ return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID; };
	private:
		uint32_t m_RendererID = 0;
		std::string m_Path;

		TextureFormat m_Format;
		TextureWrap m_Wrap = TextureWrap::Clamp;
		uint32_t m_Width, m_Height, m_Channels;

		Buffer m_Data;
		bool m_IsHDR = false;
		bool m_Locked = false;
		bool m_Loaded = false;
	};
}