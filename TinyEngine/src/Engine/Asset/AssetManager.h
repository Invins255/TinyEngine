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

		template<typename T, typename... Args>
		static Ref<T> CreateNewAsset(Args&&... args)
		{
			static_assert(std::is_base_of<Asset, T>::value, "CreateNewAsset only works for types derived from Asset");

			Ref<T> asset = T::Create(std::forward<Args>(args)...);
			asset->Handle = AssetHandle();
			s_LoadedAssets[asset->Handle] = asset;

			return asset;
		}

	private:
		static std::unordered_map<AssetHandle, Ref<Asset>> s_LoadedAssets;

	};
}
