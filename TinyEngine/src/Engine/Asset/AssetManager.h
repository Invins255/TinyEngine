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

		static const std::unordered_map<AssetHandle, Ref<Asset>>& GetLoadedAssets() { return s_LoadedAssets; }
		static const std::unordered_map<AssetHandle, Ref<Asset>>& GetMemoryAssets() { return s_MemoryAssets; }

		template<typename T, typename... Args>
		static Ref<T> CreateNewAsset(Args&&... args)
		{
			static_assert(std::is_base_of<Asset, T>::value, "CreateNewAsset only works for types derived from Asset");

			Ref<T> asset = T::Create(std::forward<Args>(args)...);
			asset->Handle = AssetHandle();
			s_LoadedAssets[asset->Handle] = asset;

			return asset;
		}

		template<typename T, typename... Args>
		static Ref<T> CreateMemoryAsset(Args&&... args)
		{
			static_assert(std::is_base_of<Asset, T>::value, "CreateMemoryOnlyAsset only works for types derived from Asset");

			Ref<T> asset = T::Create(std::forward<Args>(args)...);
			asset->Handle = AssetHandle();
			s_MemoryAssets[asset->Handle] = asset;

			return asset;
		}

		static bool IsMemoryAsset(AssetHandle handle)
		{
			return s_MemoryAssets.find(handle) != s_MemoryAssets.end();
		}

		template<typename T>
		static Ref<T> GetAsset(AssetHandle handle)
		{
			if (IsMemoryAsset(handle))
				return std::dynamic_pointer_cast<T>(s_MemoryAssets[handle]);
			else
				return std::dynamic_pointer_cast<T>(s_LoadedAssets[handle]);
		}

		static void ClearUnusedMemoryAsset();

	private:
		static std::unordered_map<AssetHandle, Ref<Asset>> s_LoadedAssets; 
		static std::unordered_map<AssetHandle, Ref<Asset>> s_MemoryAssets;

	};
}
