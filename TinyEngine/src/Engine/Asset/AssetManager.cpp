#include "pch.h"
#include "AssetManager.h"
#include "Engine/Core/Log.h"

namespace Engine
{
	std::unordered_map<AssetHandle, Ref<Asset>> AssetManager::s_LoadedAssets;
	std::unordered_map<AssetHandle, Ref<Asset>> AssetManager::s_MemoryAssets;

	void Engine::AssetManager::Init()
	{

	}

	void Engine::AssetManager::Shutdown()
	{
		s_LoadedAssets.clear();
		s_MemoryAssets.clear();
	}

	void AssetManager::ClearUnusedMemoryAsset()
	{
		for (auto iter = s_MemoryAssets.begin(); iter != s_MemoryAssets.end(); iter++)
		{
			auto& assetRef = iter->second;
			if (assetRef.use_count() == 1)
			{
				ENGINE_WARN("Clear Asset {0}", assetRef->Handle);
			}
		}
	}

}