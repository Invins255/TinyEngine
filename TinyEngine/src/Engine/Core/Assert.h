#pragma once

#ifdef ENGINE_DEBUG
	#define ENGINE_ENABLE_ASSERTS
#endif

#ifdef ENGINE_ENABLE_ASSERTS
	#define ENGINE_ASSERT(condition, ...) { if(!(condition)) { ENGINE_ERROR("Assertion Failed: {0}", __VA_ARGS__);__debugbreak(); } }
	#define APP_ASSERT(condition, ...) { if(!(condition)) { APP_ERROR("Assertion Failed: {0}", __VA_ARGS__);__debugbreak(); } }
#else
	#define ENGINE_ASSERT(condition, ...)
	#define APP_ASSERT(condition, ...) 
#endif 