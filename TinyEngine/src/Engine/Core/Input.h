#pragma once

#include "Engine/Core/KeyCodes.h"
#include "Engine/Core/MouseButtonCodes.h"

namespace Engine
{
	/// <summary>
	/// Input���࣬���ڲ�ͬƽ̨���������ʵ�֡�Input��һ��������
	/// </summary>  
	class Input
	{
	public:
		//Key
		static bool IsKeyPressed(int keyCode);

		//Momuse
		static bool IsMouseButtonPressed(int button);
		static float GetMouseX();
		static float GetMouseY();
		static std::pair<float, float> GetMousePosition();

		//Cursor
		static void SetCursorMode(CursorMode mode);
		static CursorMode GetCursorMode();
	};
}