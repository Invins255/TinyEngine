#pragma once

namespace Engine
{
	/// <summary>
	/// Input基类，基于不同平台对子类进行实现。Input是一个单例。
	/// </summary>  
	class Input
	{
	public:
		static bool IsKeyPressed(int keyCode);

		static bool IsMouseButtonPressed(int button);
		static float GetMouseX();
		static float GetMouseY();
		static std::pair<float, float> GetMousePosition();
	};
}