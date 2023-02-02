#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/Texture.h"
#include <unordered_set>

namespace Engine
{
	enum class MaterialFlag
	{
		None		= BIT(0),
		DepthTest	= BIT(1),
		Blend		= BIT(2)
	};

	class Material
	{
	public:
		static Ref<Material> Create(const Ref<Shader>& shader);

		Material(const Ref<Shader>& shader);
		virtual ~Material();

		void Bind() const;



	private:
		Ref<Shader> m_Shader;
		uint32_t m_MaterialFlags;
		//std::unordered_set<MaterialInstance*> m_MaterialInstances;
	};

	class MaterialInstance
	{

	};
}