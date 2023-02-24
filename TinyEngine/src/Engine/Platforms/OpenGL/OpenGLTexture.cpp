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
        case Engine::TextureFormat::RGBA16F:    return GL_RGBA16F;
        }
        ENGINE_ASSERT(false, "Unknown texture format!");
        return 0;
    }

    //-----------------------------------------------------------------------------------
    //OpenGLTexture2D
    //-----------------------------------------------------------------------------------
    OpenGLTexture2D::OpenGLTexture2D(const std::string& path, bool srgb, TextureSpecification spec)
        :m_Path(path), m_Specification(spec)
    {
        if(m_Specification.Flip == TextureFlip::None)
            stbi_set_flip_vertically_on_load(false);
        if (m_Specification.Flip == TextureFlip::Vertical)
            stbi_set_flip_vertically_on_load(true);

        int width, height, channels;
        if (stbi_is_hdr(path.c_str()))
        {
            m_Data.Data = (uint8_t*)stbi_loadf(path.c_str(), &width, &height, &channels, 0);
            m_IsHDR = true;
            m_Format = TextureFormat::RGBA16F;
        }
        else
        {
            m_Data.Data = stbi_load(path.c_str(), &width, &height, &channels, srgb ? STBI_rgb : STBI_rgb_alpha);
            m_IsHDR = false;
            m_Format = TextureFormat::RGBA;
        }
        if (!m_Data.Data)
        {
            ENGINE_ERROR("Could not read image '{0}'", path);
            stbi_image_free(m_Data.Data);
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

                RENDERCOMMAND_TRACE("RenderCommand: Construct texture2D. Path: '{0}', ID: ({1})", m_Path, m_RendererID);
            });
    }

    OpenGLTexture2D::OpenGLTexture2D(TextureFormat format, uint32_t width, uint32_t height, TextureSpecification spec)
        :m_Format(format), m_Width(width), m_Height(height), m_Specification(spec)
    {
        Renderer::Submit([this]()
            {
                glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
                glBindTexture(GL_TEXTURE_2D, m_RendererID);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                GLenum wrap = m_Specification.Wrap == TextureWrap::Clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT;
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

                RENDERCOMMAND_TRACE("RenderCommand: Construct texture. ID: ({0})", m_RendererID);
            });

        m_Data.Allocate(width * height * Texture::GetBPP(m_Format));
    }

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        uint32_t rendererID = m_RendererID;
        Renderer::Submit([rendererID]()
            {
                RENDERCOMMAND_TRACE("RenderCommand: Destroy texture({0})", rendererID);
                glDeleteTextures(1, &rendererID);
            });
    }

    uint32_t OpenGLTexture2D::GetMipLevelCount() const
    {
        return Texture::CalculateMipMapCount(m_Width, m_Height);
    }

    void OpenGLTexture2D::Bind(uint32_t slot) const
    {
        Renderer::Submit([this, slot]()
            {
                RENDERCOMMAND_TRACE("RenderCommand: Bind texture({0})", m_RendererID);
                glBindTextureUnit(slot, m_RendererID);
            });
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
            });
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
    
    //--------------------------------------------------------------------------------
    // OpenGLTextureCube
    //--------------------------------------------------------------------------------
    OpenGLTextureCube::OpenGLTextureCube(
        const std::string& right, const std::string& left,
        const std::string& top, const std::string& bottom,
        const std::string& front, const std::string& back)
    {
        m_Path[0] = right;
        m_Path[1] = left;
        m_Path[2] = top;
        m_Path[3] = bottom;
        m_Path[4] = front;
        m_Path[5] = back;

        int width, height, channels;
        for (uint32_t i = 0; i < 6; i++)
        {
            m_Data[i].Data = stbi_load(m_Path[i].c_str(), &width, &height, &channels, STBI_rgb);
            if (!m_Data[i].Data)
            {
                ENGINE_ERROR("Could not read image '{0}'", m_Path[i]);
                for (uint32_t j = 0; j <= i; j++)
                    stbi_image_free(m_Data[j].Data);
                return;
            }
        }

        m_Loaded = true;
        m_Width = width;
        m_Height = height;
        m_Channels = channels;
        m_Format = TextureFormat::RGB;

        Renderer::Submit([this]()
            {
                glGenTextures(1, &m_RendererID);
                glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            
                auto format = TextureFormatToOpenGLTextureFormat(m_Format);

                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, format, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, m_Data[0].Data);//right
                glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, format, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, m_Data[1].Data);//left
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, format, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, m_Data[2].Data);//top
                glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, format, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, m_Data[3].Data);//bottom
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, format, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, m_Data[4].Data);//front
                glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, format, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, m_Data[5].Data);//back
            
                glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

                for (uint32_t i = 0; i < 6; i++)
                    stbi_image_free(m_Data[i].Data);
            });
    }

    OpenGLTextureCube::OpenGLTextureCube(TextureFormat format, uint32_t width, uint32_t height)
    {
        m_Width = width;
        m_Height = height;
        m_Format = format;

        uint32_t levels = Texture::CalculateMipMapCount(width, height);
        Renderer::Submit([this, levels]()
            {
                glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererID);
                glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

                auto format = TextureFormatToOpenGLTextureFormat(m_Format);
                glTextureStorage2D(m_RendererID, levels, format, m_Width, m_Height);
                glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
                glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            });
    }

    OpenGLTextureCube::~OpenGLTextureCube()
    {
        uint32_t rendererID = m_RendererID;
        Renderer::Submit([rendererID]() 
            {
                glDeleteTextures(1, &rendererID);
            });
    }

    const std::vector<std::string> OpenGLTextureCube::GetPath() const
    {
        std::vector<std::string> path;
        for (uint32_t i = 0; i < 6; i++)
            path.push_back(std::string(m_Path[i]));
        return path;
    }

    uint32_t OpenGLTextureCube::GetMipLevelCount() const
    {
        return Texture::CalculateMipMapCount(m_Width, m_Height);
    }

    void OpenGLTextureCube::Bind(uint32_t slot) const
    {
        Renderer::Submit([this, slot]()
            {
                glBindTextureUnit(slot, m_RendererID);
            });
    }
}