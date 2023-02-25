#pragma once

#include <string>
#include "Engine/Core/Core.h"
#include "Engine/Core/Buffer.h"

namespace Engine
{
	//TODO: More options

	enum class TextureFormat
	{
		None = 0,
		RG,
		RG16F,
		RGB,
		RGBA,
		RGBA16F
	};

	enum class TextureWrap
	{
		None = 0,
		Clamp,
		Repeat
	};

	enum class TextureFlip
	{
		None = 0,
		Vertical
	};

	struct TextureSpecification
	{
		TextureWrap Wrap = TextureWrap::Clamp;
		TextureFlip Flip = TextureFlip::Vertical;
	};

	class Texture
	{
	public:
		static std::vector<Ref<Texture>> s_AllTextures;

	public:
		static uint32_t GetBPP(TextureFormat format);
		static uint32_t CalculateMipMapCount(uint32_t width, uint32_t height);

	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetChannels() const = 0;
		virtual uint32_t GetMipLevelCount() const = 0;
		virtual TextureFormat GetFormat() const = 0;
		
		virtual uint32_t GetRendererID() const = 0;
		virtual void Bind(uint32_t slot = 0) const = 0;
	
		virtual bool operator==(const Texture& other) = 0;
	};
		
	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(const std::string& path, bool srgb = false, TextureSpecification spec = {});
		static Ref<Texture2D> Create(TextureFormat format, uint32_t width, uint32_t height, TextureSpecification spec = {});

		virtual void Lock() = 0;
		virtual void Unlock() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual Buffer& GetWritableBuffer() = 0;

		virtual bool IsLoaded() const = 0;

		virtual const std::string& GetPath() const = 0;
	};

	class TextureCube : public Texture
	{
	public:
		static Ref<TextureCube> Create(
			const std::string& right, const std::string& left,
			const std::string& top, const std::string& bottom,
			const std::string& front, const std::string& back
		);

		static Ref<TextureCube> Create(TextureFormat format, uint32_t width, uint32_t height);

		virtual bool IsLoaded() const = 0;
		virtual const std::vector<std::string> GetPath() const = 0;
	};
}