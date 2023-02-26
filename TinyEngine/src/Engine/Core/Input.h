#pragma once

namespace Engine
{
	/// <summary>
	/// Input���࣬���ڲ�ͬƽ̨���������ʵ�֡�Input��һ��������
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