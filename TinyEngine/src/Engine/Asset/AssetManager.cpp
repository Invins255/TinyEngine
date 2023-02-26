#include "pch.h"
#include "AssetManager.h"

namespace Engine
{
	std::unordered_map<AssetHandle, Ref<Asset>> AssetManager::s_LoadedAssets;

	void Engine::AssetManager::Init()
	{

	}

	void Engine::AssetManager::Shutdown()
	{
		s_LoadedAssets.clear();
	}

}