#pragma once

#include "pch.h"
#include "Engine/Core/Core.h"
#include "Engine/Events/Event.h"
#include "Engine/Core/TimeStep.h"

namespace Engine
{
	/// <summary>
	/// Layer基类。Layer将不同部分的事件与渲染分层进行处理。
	/// </summary>
	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		/// <summary>
		/// Layer入栈
		/// </summary>
		virtual void OnAttach() {}
		/// <summary>
		/// Layer出栈
		/// </summary>
		virtual void OnDetach() {}
		/// <summary>
		/// Layer更新
		/// </summary>
		virtual void OnUpdate(Timestep ts) {}
		/// <summary>
		/// Layer事件处理
		/// </summary>
		virtual void OnEvent(Event& e) {}
		/// <summary>
		/// Layer对ImGui层的影响。（Layer在更新过程中可能对UI造成影响，因此每个Layer需要对该函数进行实现）
		/// </summary>
		virtual void OnImGuiRender() {}

		const std::string& GetName() const { return m_Name; }
	protected:
		std::string m_Name;
	};
}
