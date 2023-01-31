#include "pch.h"
#include "OpenGLFrameBuffer.h"
#include "Engine/Renderer/Renderer.h"

#include <glad/glad.h>

namespace Engine
{
    static const uint32_t s_MaxFrameBufferSize = 8192;

    OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecification& spec):
        m_Specification(spec)
    {
        Initailize(); 
    }

    OpenGLFrameBuffer::~OpenGLFrameBuffer()
    {
        uint32_t rendererID = m_RendererID;
        uint32_t colorAttachment = m_ColorAttachment;
        uint32_t depthAttachment = m_DepthAttachment;
        Renderer::Submit([rendererID, colorAttachment, depthAttachment]()
            {
                ENGINE_INFO("RenderCommand: Destroy frameBuffer({0})", rendererID);

                glDeleteTextures(1, &colorAttachment);
                glDeleteTextures(1, &depthAttachment);
                glDeleteFramebuffers(1, &rendererID);
            }
        );
    }

    void OpenGLFrameBuffer::Bind()
    {
        Renderer::Submit([this]()
            {
                ENGINE_INFO("RenderCommand: Bind frameBuffer({0})", m_RendererID);

                glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
                glViewport(0, 0, m_Specification.Width, m_Specification.Height);
            }
        );
    }

    void OpenGLFrameBuffer::Unbind()
    {
        Renderer::Submit([this]()
            {
                ENGINE_INFO("RenderCommand: Unbind frameBuffer({0})", m_RendererID);

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

        Initailize();
    }

    void OpenGLFrameBuffer::Initailize()
    {
        Renderer::Submit([this]()
            {
                if (m_RendererID)
                {
                    glDeleteFramebuffers(1, &m_RendererID);
                    glDeleteTextures(1, &m_ColorAttachment);
                    glDeleteTextures(1, &m_DepthAttachment);
                }

                glCreateFramebuffers(1, &m_RendererID);
                glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

                //Color attachment
                glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttachment);
                glBindTexture(GL_TEXTURE_2D, m_ColorAttachment);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Specification.Width, m_Specification.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0);

                //Depth and stencil attachment
                glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachment);
                glBindTexture(GL_TEXTURE_2D, m_DepthAttachment);
                glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, m_Specification.Width, m_Specification.Height);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0);

                ENGINE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "FrameBuffer is incomplete!");

                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                ENGINE_INFO("RenderCommand: Construct frameBuffer({0})", m_RendererID);
            }
        );
    }
}