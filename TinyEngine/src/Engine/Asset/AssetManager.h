#pragma once

#include "Engine/Core/Ref.h"
#include "Engine/Asset/Asset.h"
#include <unordered_map>

namespace Engine
{
	class AssetManager
	{
	public:
		static void Init();
		static void Shutdown();

	private:
		static std::unordered_map<AssetHandle, Ref<Asset>> s_LoadedAssets;

	};
}
