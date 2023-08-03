#pragma once

namespace Engine
{
	class PhysicsSettingsPanel
	{
	public:
		static void OnImGuiRender(bool& show);

	private:
		static void RenderWorldSettings();
		static void RenderLayerList();
		static void RenderLayerCollisionMatrix();
	};
}
