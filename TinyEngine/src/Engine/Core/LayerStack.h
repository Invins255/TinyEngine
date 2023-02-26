#pragma once

#include "pch.h"
#include "Engine/Core/Layer.h"

namespace Engine 
{
	/// <summary>
	/// Layer栈，由Application使用并对Layer进行存储管理。
	/// </summary>
	class LayerStack
	{
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);

		std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
		std::vector<Layer*>::iterator end() { return m_Layers.end(); }

		void Clear() { m_Layers.clear(); }
	private:
		std::vector<Layer*> m_Layers;
		unsigned int m_LayerInsertIndex = 0;
	};

}

