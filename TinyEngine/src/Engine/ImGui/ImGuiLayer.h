#pragma once

#include "Engine/Core/Layer.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/Events/ApplicationEvent.h"

namespace Engine 
{
	/// <summary>
	/// ImGuiLayer����ImGui�Ŀ��������
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
		/// ImGuiLayer����ʱһ֡�Ŀ�ʼ���������һ֡ǰ��׼������
		/// </summary>
		void Begin();
		/// <summary>
		/// ImGuiLayer����ʱһ֡����ֹ���������һ֡��Ľ�������
		/// </summary>
		void End();

		void BlockEvents(bool value) { m_BlockEvents = value; }
	private:
		float m_Time = 0.0f;
		bool m_BlockEvents = true;
	};

}

