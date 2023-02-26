#pragma once

#include "Engine/Core/Ref.h"
#include "Engine/Core/UUID.h"
#include "Engine/Asset/AssetType.h"

namespace Engine
{
	using AssetHandle = UUID;

	class Asset : public RefCounted
	{
	public:
		static AssetType GetStaticType() { return AssetType::None; }

		AssetHandle Handle = 0;
		uint16_t Flags = (uint16_t)AssetFlag::None;

	public:
		virtual ~Asset() {};

		virtual AssetType GetAssetType() const { return AssetType::None; }

		bool IsValid() const { return ((Flags & (uint16_t)AssetFlag::Missing) | (Flags & (uint16_t)AssetFlag::Invalid)) == 0; }
		
		bool IsFlagSet(AssetFlag flag) const { return (uint16_t)flag & Flags; }
		void SetFlag(AssetFlag flag, bool value = true)
		{
			if (value)
				Flags |= (uint16_t)flag;
			else
				Flags &= ~(uint16_t)flag;
		}

		virtual bool operator==(const Asset& other) const
		{
			return Handle == other.Handle;
		}

		virtual bool operator!=(const Asset& other) const
		{
			return !(*this == other);
		}
	};
}