#pragma once
#include "Engine/Core/Core.h"

//--------------------------------------------------------
//Entry Point
//--------------------------------------------------------
#ifdef  ENGINE_PLATFORM_WINDOWS

extern Engine::Application* Engine::CreateApplication();

int main(int argc, char** argv) {

	Engine::Log::Init();

	auto app = Engine::CreateApplication();	
	app->Run();
	delete app;

	return 0;
}

#endif //  ENGINE_PLATFORM_WINDOWS
