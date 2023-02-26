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

		Ref<TextureCube> envUnfiltered = TextureCube::Create(filepath);

		//Prefliter
		Ref<TextureCube> envFiltered = TextureCube::Create(TextureFormat::RGBA16F, cubemapSize, cubemapSize);
		auto envFilteringShader = Renderer::GetShaderLibrary().Get("EnvironmentMipFilter");
		envFilteringShader->Bind();
		envUnfiltered->Bind(); 
		Renderer::Submit([envFilteringShader, envFiltered, cubemapSize]()
			{
				const float deltaRoughness = 1.0f / glm::max((float)(envFiltered->GetMipLevelCount() - 1.0f), 1.0f);
				for (int level = 1, size = cubemapSize / 2; level < envFiltered->GetMipLevelCount(); level++, size /= 2) 
				{
					glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
					const GLuint numGroups = glm::max(1, size / 32);
					glBindImageTexture(0, envFiltered->GetRendererID(), level, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
					glProgramUniform1f(envFilteringShader->GetRendererID(), 0, level * deltaRoughness);
					glDispatchCompute(numGroups, numGroups, 6);
				}			
			});

		//BUG: ���ܵ��±�����ԭ��������ڶ�ͼƬ�Ķ�ȡ��ʽ����ش洢�������
		Ref<TextureCube> irradianceMap = TextureCube::Create(TextureFormat::RGBA16F, irradianceMapSize, irradianceMapSize);
		auto envIrradianceShader = Renderer::GetShaderLibrary().Get("EnvironmentIrradianceDiffuse");
		envIrradianceShader->Bind();
		envUnfiltered->Bind();
		Renderer::Submit([irradianceMap]()
			{
				glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
				glBindImageTexture(0, irradianceMap->GetRendererID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
				glDispatchCompute(irradianceMap->GetWidth() / 32, irradianceMap->GetHeight() / 32, 6);
				glGenerateTextureMipmap(irradianceMap->GetRendererID());
			});
		
		return Environment{filepath, envUnfiltered, irradianceMap, envFiltered};
	}
}