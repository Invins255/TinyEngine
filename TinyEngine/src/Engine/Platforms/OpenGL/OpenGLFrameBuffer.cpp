#include "pch.h"
#include "OpenGLFrameBuffer.h"
#include "Engine/Renderer/Renderer.h"

#include <glad/glad.h>

namespace Engine
{
    static const uint32_t s_MaxFrameBufferSize = 8192;

    static GLenum TextureTarget(bool multisampled)
    {
        return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
    }

    static GLenum DataType(GLenum format)
    {
        switch (format)
        {
        case GL_RGBA8: return GL_UNSIGNED_BYTE;
        case GL_RG16F:
        case GL_RG32F:
        case GL_RGBA16F:
        case GL_RGBA32F: return GL_FLOAT;
        case GL_DEPTH24_STENCIL8: return GL_UNSIGNED_INT_24_8;
        }

        ENGINE_ASSERT(false, "Unknown format!");
        return 0;
    }

    static bool IsDepthFormat(FrameBufferTextureFormat format)
    {
        switch (format)
        {
        case FrameBufferTextureFormat::DEPTH24STENCIL8:
        case FrameBufferTextureFormat::DEPTH32F:
            return true;
        }
        return false;
    }

    static void AttachColorTexture(uint32_t id, int samples, GLenum format, uint32_t width, uint32_t height, int index)
    {
        bool multisampled = samples > 1;
        if (multisampled)
        {
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
        }
        else
        {
            // Only RGBA access for now
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, DataType(format), nullptr);

            glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
    }

    static void AttachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height)
    {
        bool multisampled = samples > 1;
        if (multisampled)
        {
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
        }
        else
        {
            glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

            glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0);
    }

    //--------------------------------------------------------------------------------
    // OpenGLFrameBuffer
    //--------------------------------------------------------------------------------
    OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecification& spec):
        m_Specification(spec)
    {
        for (auto format : m_Specification.Attachments)
        {
            if (!IsDepthFormat(format.TextureFormat))
                m_ColorAttachmentFormats.emplace_back(format.TextureFormat);
            else
                m_DepthAttachmentFormat = format.TextureFormat;
        }

        Create(); 
    }

    OpenGLFrameBuffer::~OpenGLFrameBuffer()
    {
        uint32_t rendererID = m_RendererID;
        std::vector<uint32_t> colorAttachments = m_ColorAttachments;
        uint32_t depthAttachment = m_DepthAttachment;
        Renderer::Submit([rendererID, colorAttachments, depthAttachment]()
            {
                RENDERCOMMAND_TRACE("RenderCommand: Destroy frameBuffer({0})", rendererID);

                glDeleteTextures(colorAttachments.size(), colorAttachments.data());
                glDeleteTextures(1, &depthAttachment);
                glDeleteFramebuffers(1, &rendererID);
            }
        );
    }

    void OpenGLFrameBuffer::Bind()
    {
        Renderer::Submit([this]()
            {
                RENDERCOMMAND_TRACE("RenderCommand: Bind frameBuffer({0})", m_RendererID);

                glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
                glViewport(0, 0, m_Specification.Width, m_Specification.Height);
            }
        );
    }

    void OpenGLFrameBuffer::Unbind()
    {
        Renderer::Submit([this]()
            {
                RENDERCOMMAND_TRACE("RenderCommand: Unbind frameBuffer({0})", m_RendererID);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }
        );
    }

    void OpenGLFrameBuffer::Resize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0 || width > s_MaxFrameBufferSize || height > s_MaxFrameBufferSize)
        {
            ENGINE_WARN("Invalid frameBuffer size: ({0}, {1})", width, height);
            return;
        }

        m_Specification.Width = width;
        m_Specification.Height = height;

        Create();
    }

    void OpenGLFrameBuffer::BindTexture(uint32_t attachmentIndex, uint32_t slot) const
    {
        Renderer::Submit([this, attachmentIndex, slot]() {
            glBindTextureUnit(slot, m_ColorAttachments[attachmentIndex]);
        });
    }

    void OpenGLFrameBuffer::Create()
    {
        Renderer::Submit([this]()
            {
                if (m_RendererID)
                {
                    glDeleteFramebuffers(1, &m_RendererID);
                    glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
                    glDeleteTextures(1, &m_DepthAttachment);

                    m_ColorAttachments.clear();
                    m_DepthAttachment = 0;
                }

                glCreateFramebuffers(1, &m_RendererID);
                glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

                bool multisampled = m_Specification.Samples > 1;

                //Color attachments
                if (m_ColorAttachmentFormats.size())
                {
                    m_ColorAttachments.resize(m_ColorAttachmentFormats.size());
                    glCreateTextures(TextureTarget(multisampled), 1, m_ColorAttachments.data());

                    for (uint32_t i = 0; i < m_ColorAttachments.size(); i++)
                    {
                        glBindTexture(TextureTarget(multisampled), m_ColorAttachments[i]);
                        switch (m_ColorAttachmentFormats[i])
                        {
                        case FrameBufferTextureFormat::RGBA8:
                            AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGBA8, m_Specification.Width, m_Specification.Height, i);
                            break;
                        case FrameBufferTextureFormat::RGBA16F:
                            AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGBA16F, m_Specification.Width, m_Specification.Height, i);
                            break;
                        case FrameBufferTextureFormat::RGBA32F:
                            AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGBA32F, m_Specification.Width, m_Specification.Height, i);
                            break;
                        }
                    }
                }
                
                //Depth and stencil attachment
                if (m_DepthAttachmentFormat != FrameBufferTextureFormat::None)
                {
                    glCreateTextures(TextureTarget(multisampled), 1, &m_DepthAttachment);
                    glBindTexture(TextureTarget(multisampled), m_DepthAttachment);
                    switch (m_DepthAttachmentFormat)
                    {
                    case FrameBufferTextureFormat::DEPTH24STENCIL8:
                        AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
                        break;
                    case FrameBufferTextureFormat::DEPTH32F:
                        AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT, m_Specification.Width, m_Specification.Height);
                        break;
                    }
                }
                
                if (m_ColorAttachments.size() > 1)
                {
                    ENGINE_ASSERT(m_ColorAttachments.size() <= MaxColorAttachmentCount, "ColorAttachment count exceeds the maximum!");
                    GLenum buffers[MaxColorAttachmentCount] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
                    glDrawBuffers(m_ColorAttachments.size(), buffers);
                }
                else if (m_ColorAttachments.size() == 0)
                {
                    glDrawBuffer(GL_NONE);
                }

                ENGINE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "FrameBuffer is incomplete!");

                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                RENDERCOMMAND_TRACE("RenderCommand: Construct frameBuffer({0})", m_RendererID);
            }
        );
    }
}