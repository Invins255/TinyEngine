#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "Engine/Core/Core.h"

#include <memory>

namespace Engine
{
	class Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

//Engine log
#define ENGINE_TRACE(...)	::Engine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define ENGINE_INFO(...)	::Engine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define ENGINE_WARN(...)	::Engine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define ENGINE_ERROR(...)	::Engine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define ENGINE_FATAL(...)	::Engine::Log::GetCoreLogger()->critical(__VA_ARGS__)

//Client log
#define APP_TRACE(...)	::Engine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define APP_INFO(...)	::Engine::Log::GetClientLogger()->info(__VA_ARGS__)
#define APP_WARN(...)	::Engine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define APP_ERROR(...)	::Engine::Log::GetClientLogger()->error(__VA_ARGS__)
#define APP_FATAL(...)	::Engine::Log::GetClientLogger()->critical(__VA_ARGS__)


