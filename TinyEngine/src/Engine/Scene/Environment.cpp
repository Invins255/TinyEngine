#include "pch.h"
#include "Environment.h"
#include "Engine/Renderer/Renderer.h"

#include <glad/glad.h>

namespace Engine
{
	Environment Environment::Create(const std::string& filepath)
	{

		//Create skybox map and irradiance map from HDR image by using compute shader
		const uint32_t cubemapSize = 2048;
		const uint32_t irradianceMapSize = 32;

		//Convert Equirectangular map to Cube map
		TextureSpecification spec;
		spec.Flip = TextureFlip::None;
		Ref<Texture2D> envEquirect = Texture2D::Create(filepath, false, spec);
		ENGINE_ASSERT(envEquirect->GetFormat() == TextureFormat::RGBA16F, "Texture is not HDR");

		Ref<TextureCube> envUnfiltered = TextureCube::Create(TextureFormat::RGBA16F, cubemapSize, cubemapSize);		
		auto equirectangularConversionShader = Renderer::GetShaderLibrary().Get("EquirectangularToCubeMap");		
		equirectangularConversionShader->Bind();
		envEquirect->Bind();
		Renderer::Submit([envUnfiltered, cubemapSize] 
			{
				glBindImageTexture(0, envUnfiltered->GetRendererID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
				glDispatchCompute(cubemapSize / 32, cubemapSize / 32, 6);
				glGenerateTextureMipmap(envUnfiltered->GetRendererID());
			});
			
		Ref<TextureCube> envFiltered = TextureCube::Create(TextureFormat::RGBA16F, cubemapSize, cubemapSize);		
		auto envFilteringShader = Renderer::GetShaderLibrary().Get("EnvironmentMipFilter");			
		
		//Genarate mipmap
		envFilteringShader->Bind();
		envUnfiltered->Bind(); 
		Renderer::Submit([envFilteringShader, envFiltered, cubemapSize]()
			{
				const float deltaRoughness = 1.0f / glm::max((float)(envFiltered->GetMipLevelCount() - 1.0f), 1.0f);
				for (int level = 1, size = cubemapSize / 2; level < envFiltered->GetMipLevelCount(); level++, size /= 2) 
				{
					const GLuint numGroups = glm::max(1, size / 32);
					glBindImageTexture(0, envFiltered->GetRendererID(), level, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
					glProgramUniform1f(envFilteringShader->GetRendererID(), 0, level * deltaRoughness);
					glDispatchCompute(numGroups, numGroups, 6);
				}			
			});
		
		
		//Genarate irradiance map
		/*
		Ref<TextureCube> irradianceMap = TextureCube::Create(TextureFormat::RGBA16F, irradianceMapSize, irradianceMapSize);		
		auto envIrradianceShader = Renderer::GetShaderLibrary().Get("EnvironmentIrradiance");		
		envIrradianceShader->Bind();
		envFiltered->Bind();
		Renderer::Submit([irradianceMap]()
			{
				glBindImageTexture(0, irradianceMap->GetRendererID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
				glDispatchCompute(irradianceMap->GetWidth() / 32, irradianceMap->GetHeight() / 32, 6);	
				glGenerateTextureMipmap(irradianceMap->GetRendererID());
			});
		*/
		Ref<TextureCube> irradianceMap = TextureCube::Create(TextureFormat::RGBA16F, 32, 32);
		auto envIrradianceShader = Renderer::GetShaderLibrary().Get("EnvironmentIrradianceDiffuse");
		envIrradianceShader->Bind();
		envUnfiltered->Bind();
		Renderer::Submit([irradianceMap]()
			{
				glBindImageTexture(0, irradianceMap->GetRendererID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
				glDispatchCompute(irradianceMap->GetWidth() / 32, irradianceMap->GetHeight() / 32, 6);
				glGenerateTextureMipmap(irradianceMap->GetRendererID());
			});
		
		return Environment{filepath, irradianceMap, irradianceMap};
	}
}