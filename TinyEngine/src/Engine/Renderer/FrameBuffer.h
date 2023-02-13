#pragma once

#include <glm/glm.hpp>
#include "Engine/Core/Core.h"

namespace Engine
{
	//TODO: More values
	enum class FrameBufferTextureFormat
	{
		None = 0,
		//Color
		RGBA8, RGBA16F, RGBA32F, 
		//Depth/Stencil
		DEPTH32F, DEPTH24STENCIL8
	};

	struct FrameBufferTextureSpecification
	{
		FrameBufferTextureSpecification() = default;
		FrameBufferTextureSpecification(FrameBufferTextureFormat format) : TextureFormat(format) {}

		FrameBufferTextureFormat TextureFormat;
		// TODO: filtering/wrap
	};

	using FrameBufferAttachmentSpecification = std::vector<FrameBufferTextureSpecification>;

	struct FrameBufferSpecification
	{
		uint32_t Width = 0; 
		uint32_t Height = 0;
		glm::vec4 ClearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		FrameBufferAttachmentSpecification Attachments;
		uint32_t Samples = 1; //BUG: Multisample

		bool SwapChainTarget = false;
	};

	class FrameBuffer
	{
	public:
		static const uint32_t MaxColorAttachmentCount = 4;

		virtual ~FrameBuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual const FrameBufferSpecification& GetSpecification() const = 0;
		virtual uint32_t GetRendererID() const = 0;
		virtual uint32_t GetColorAttachmentID(int index = 0) const = 0;
		virtual uint32_t GetDepthAttachmentID() const = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void BindTexture(uint32_t attachmentIndex = 0, uint32_t slot = 0) const = 0;

		static Ref<FrameBuffer> Create(const FrameBufferSpecification& spec);	
	};
}

