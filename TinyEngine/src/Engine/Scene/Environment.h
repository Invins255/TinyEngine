#pragma once

#include "Engine/Renderer/Texture.h"

namespace Engine
{
	struct Environment
	{
		std::string Path;
		Ref<TextureCube> SkyboxMap;
		Ref<TextureCube> IrradianceMap;

		static Environment Create(const std::string& filepath);
	};
}