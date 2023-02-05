#include "pch.h"
#include "OpenGLTexture.h"
#include "Engine/Renderer/Renderer.h"

#include "stb_image.h"
#include <glad/glad.h>

namespace Engine
{
    static GLenum TextureFormatToOpenGLTextureFormat(TextureFormat format)
    {
        switch (format)
        {
        case Engine::TextureFormat::RGB:        return GL_RGB;
        case Engine::TextureFormat::RGBA:       return GL_RGBA;
        case Engine::TextureFormat::Float16:    return GL_RGBA16F;
        }
        ENGINE_ASSERT(false, "Unknown texture format!");
        return 0;
    }

    //-----------------------------------------------------------------------------------
    //OpenGLTexture2D
    //-----------------------------------------------------------------------------------
    OpenGLTexture2D::OpenGLTexture2D(const std::string& path, bool srgb)
        :m_Path(path)
    {
        int width, height, channels;
        if (stbi_is_hdr(path.c_str()))
        {
            m_Data.Data = (uint8_t*)stbi_loadf(path.c_str(), &width, &height, &channels, 0);
            m_IsHDR = true;
            m_Format = TextureFormat::Float16;
        }
        else
        {
            m_Data.Data = stbi_load(path.c_str(), &width, &height, &channels, srgb ? STBI_rgb : STBI_rgb_alpha);
            m_IsHDR = false;
            m_Format = TextureFormat::RGBA;
        }
        if (!m_Data.Data)
        {
            ENGINE_ERROR("Could not read image {0}", path);
            return;
        }

        m_Loaded = true;
        m_Width = width;
        m_Height = height;
        m_Channels = channels;

        //TODO: More details
        Renderer::Submit([this, srgb]() 
            {
                if (srgb)
                {
                    glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
                    int levels = Texture::CalculateMipMapCount(m_Width, m_Height);
                    glTextureStorage2D(m_RendererID, levels, GL_SRGB8, m_Width, m_Height);
                    glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
                    glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                    glTextureSubImage2D(
                        m_RendererID, 
                        0, 
                        0, 
                        0, 
                        m_Width, 
                        m_Height, 
                        GL_RGB, 
                        GL_UNSIGNED_BYTE, 
                        m_Data.Data
                    );
                    glGenerateTextureMipmap(m_RendererID);
                }
                else
                {
                    glGenTextures(1, &m_RendererID);
                    glBindTexture(GL_TEXTURE_2D, m_RendererID);

                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

                    GLenum internalFormat = TextureFormatToOpenGLTextureFormat(m_Format);
                    GLenum format = srgb ? GL_SRGB8 : (m_IsHDR ? GL_RGB : TextureFormatToOpenGLTextureFormat(m_Format)); // HDR = GL_RGB for now
                    GLenum type = internalFormat == GL_RGBA16F ? GL_FLOAT : GL_UNSIGNED_BYTE;
                    glTexImage2D(
                        GL_TEXTURE_2D,
                        0, 
                        internalFormat, 
                        m_Width, 
                        m_Height, 
                        0, 
                        format, 
                        type, 
                        m_Data.Data
                    );
                    glGenerateMipmap(GL_TEXTURE_2D);

                    glBindTexture(GL_TEXTURE_2D, 0);
                }
                stbi_image_free(m_Data.Data);

                RENDERCOMMAND_INFO("RenderCommand: Construct texture. Path: [{0}], ID: [{1}]", m_Path, m_RendererID);
            }
        );
    }

    OpenGLTexture2D::OpenGLTexture2D(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap)
        :m_Format(format), m_Width(width), m_Height(height), m_Wrap(wrap)
    {
        Renderer::Submit([this]()
            {
                glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
                glBindTexture(GL_TEXTURE_2D, m_RendererID);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                GLenum wrap = m_Wrap == TextureWrap::Clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT;
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

                glTexImage2D(
                    GL_TEXTURE_2D, 
                    0, 
                    TextureFormatToOpenGLTextureFormat(m_Format), 
                    m_Width, 
                    m_Height, 
                    0, 
                    TextureFormatToOpenGLTextureFormat(m_Format), 
                    GL_UNSIGNED_BYTE, 
                    nullptr
                );

                glBindTexture(GL_TEXTURE_2D, 0);

                RENDERCOMMAND_INFO("RenderCommand: Construct texture. ID: [{0}]", m_RendererID);
            }
        );

        m_Data.Allocate(width * height * Texture::GetBPP(m_Format));
    }

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        uint32_t rendererID = m_RendererID;
        Renderer::Submit([rendererID]()
            {
                RENDERCOMMAND_INFO("RenderCommand: Destroy texture. ID: [{0}]", rendererID);
                glDeleteTextures(1, &rendererID);
            }
        );
    }

    uint32_t OpenGLTexture2D::GetMipLevelCount() const
    {
        return Texture::CalculateMipMapCount(m_Width, m_Height);
    }

    void OpenGLTexture2D::Bind(uint32_t slot) const
    {
        Renderer::Submit([this, slot]()
            {
                RENDERCOMMAND_INFO("RenderCommand: Bind texture. ID: [{0}]", m_RendererID);
                glBindTextureUnit(slot, m_RendererID);
            }
        );
    }
    
    void OpenGLTexture2D::Lock()
    {
        m_Locked = true;
    }

    void OpenGLTexture2D::Unlock()
    {
        m_Locked = false;
        Renderer::Submit([this]()
            {
                glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, TextureFormatToOpenGLTextureFormat(m_Format), GL_UNSIGNED_BYTE, m_Data.Data);
            }
        );
    }
    
    void OpenGLTexture2D::Resize(uint32_t width, uint32_t height)
    {
        ENGINE_ASSERT(m_Locked, "Texture must be locked!");
        m_Data.Allocate(width * height * Texture::GetBPP(m_Format));
        m_Data.ZeroInitialize();
    }

    Buffer& OpenGLTexture2D::GetWritableBuffer()
    {
        ENGINE_ASSERT(m_Locked, "Texture must be locked!");
        return m_Data;
    }
    
}