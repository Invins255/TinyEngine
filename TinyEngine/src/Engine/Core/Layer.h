#pragma once

#include "pch.h"
#include "Engine/Core/Core.h"
#include "Engine/Events/Event.h"
#include "Engine/Core/TimeStep.h"

namespace Engine
{
	/// <summary>
	/// Layer���ࡣLayer����ͬ���ֵ��¼�����Ⱦ�ֲ���д���
	/// </summary>
	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		/// <summary>
		/// Layer��ջ
		/// </summary>
		virtual void OnAttach() {}
		/// <summary>
		/// Layer��ջ
		/// </summary>
		virtual void OnDetach() {}
		/// <summary>
		/// Layer����
		/// </summary>
		virtual void OnUpdate(Timestep ts) {}
		/// <summary>
		/// Layer�¼�����
		/// </summary>
		virtual void OnEvent(Event& e) {}
		/// <summary>
		/// Layer��ImGui���Ӱ�졣��Layer�ڸ��¹����п��ܶ�UI���Ӱ�죬���ÿ��Layer��Ҫ�Ըú�������ʵ�֣�
		/// </summary>
		virtual void OnImGuiRender() {}

		const std::string& GetName() const { return m_Name; }
	protected:
		std::string m_Name;
	};
}
