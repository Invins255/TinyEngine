#pragma once

#include <string>
#include "Engine/Core/Core.h"

namespace Engine
{
	enum class AssetFlag
	{
		None = 0,
		Missing = BIT(0),
		Invalid = BIT(1)
	};

	enum class AssetType
	{
		None = 0,
		Scene,
		Mesh,
		Texture,
		Material
	};

	inline AssetType AssetTypeFormString(const std::string& assetType)
	{
		if (assetType == "None")		return AssetType::None;
		if (assetType == "Scene")		return AssetType::Scene;
		if (assetType == "Mesh")		return AssetType::Mesh;
		if (assetType == "Texture")		return AssetType::Mesh;
		if (assetType == "Material")	return AssetType::Mesh;

		ENGINE_ASSERT(false, "Unknown Asset Type");
		return AssetType::None;
	}

	inline std::string AssetTypeToString(AssetType assetType)
	{
		switch (assetType)
		{
		case Engine::AssetType::Scene:		return "Scene";
		case Engine::AssetType::Mesh:		return "Mesh";
		case Engine::AssetType::Texture:	return "Texture";
		case Engine::AssetType::Material:	return "Material";
		}

		ENGINE_ASSERT(false, "Unknown Asset Type");
		return "None";
	}
}