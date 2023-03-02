#pragma once

#include "Engine/Renderer/Texture.h"

namespace Engine
{
	/// <summary>
	/// Scene environment textures, Skybox and IBL textures
	/// </summary>
	struct Environment
	{
		std::string Path;
		Ref<TextureCube> SkyboxMap;
		Ref<TextureCube> IrradianceMap;		//IBL Diffuse
		Ref<TextureCube> PrefliteredMap;	//IBL Specular

		static Environment Create(const std::string& filepath);
	};
}