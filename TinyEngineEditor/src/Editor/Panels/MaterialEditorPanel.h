#pragma once

#include "Engine/Renderer/Texture.h"
#include "Engine/Scene/Entity.h"

namespace Engine
{
	class MaterialEditorPanel
	{
	public:
		MaterialEditorPanel();

		void OnImGuiRender();
		void SetSelectedEntity(Entity entity) { m_SelectedEntity = entity; }

	private:
		Ref<Texture2D> m_CheckerboardTex;
		Entity m_SelectedEntity;
	};
}