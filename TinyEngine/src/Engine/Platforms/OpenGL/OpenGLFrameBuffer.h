#pragma once

#include <vector>
#include "Engine/Renderer/FrameBuffer.h"


namespace Engine
{
	class OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(const FrameBufferSpecification& spec);
		virtual ~OpenGLFrameBuffer();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual uint32_t GetWidth() const override { return m_Specification.Width; };
		virtual uint32_t GetHeight() const override { return m_Specification.Height; };

		virtual const FrameBufferSpecification& GetSpecification() const override { return m_Specification; }
		virtual uint32_t GetRendererID() const { return m_RendererID; };
		virtual uint32_t GetColorAttachmentID(int index = 0) const { ENGINE_ASSERT(index <= m_ColorAttachments.size(), ""); return m_ColorAttachments[index]; }
		virtual uint32_t GetDepthAttachmentID() const { return m_DepthAttachment; };

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual void BindTexture(uint32_t attachmentIndex = 0, uint32_t slot = 0) const override;

	private:
		void Create();

	private:
		FrameBufferSpecification m_Specification;

		uint32_t m_RendererID;
		std::vector<uint32_t> m_ColorAttachments;
		uint32_t m_DepthAttachment;

		std::vector<FrameBufferTextureFormat> m_ColorAttachmentFormats;
		FrameBufferTextureFormat m_DepthAttachmentFormat = FrameBufferTextureFormat::None;
	};
}

