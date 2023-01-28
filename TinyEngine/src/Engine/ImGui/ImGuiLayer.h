#pragma once

#include "Engine/Core/Layer.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/Events/ApplicationEvent.h"

namespace Engine 
{
	/// <summary>
	/// ImGuiLayer负责ImGui的控制与操作
	/// </summary>
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach()	override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;

		/// <summary>
		/// ImGuiLayer更新时一帧的开始，负责更新一帧前的准备工作
		/// </summary>
		void Begin();
		/// <summary>
		/// ImGuiLayer更新时一帧的终止，负责更新一帧后的结束工作
		/// </summary>
		void End();

		void BlockEvents(bool value) { m_BlockEvents = value; }
	private:
		float m_Time = 0.0f;
		bool m_BlockEvents = true;
	};

}

