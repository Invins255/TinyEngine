#pragma once

#include <memory>
#include "Engine/Core/Log.h"
#include "Engine/Core/Assert.h"

#ifdef ENGINE_PLATFORM_WINDOWS
	#if ENGINE_DYNAMIC_LINK
		#ifdef ENGINE_BUILD_DLL
			#define ENGINE_API __declspec(dllexport)
		#else 
			#define ENGINE_API __declspec(dllimport)
		#endif 
	#else
		#define ENGINE_API
	#endif
#else 
	#error Engine Only supports Windows!
#endif 

#define BIT(x) (1 << x)

#define ENGINE_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)
