#pragma once

//--------------------------------------
//Precompile Header
//--------------------------------------

#include <iostream>
#include <sstream>
#include <fstream>

#include <functional>

#include <memory>
#include <string>
#include <array>
#include <vector>
#include <tuple>
#include <unordered_map>

#define _USE_MATH_DEFINES
#include <math.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Engine/Core/Log.h"

#ifdef ENGINE_PLATFORM_WINDOWS
	#include <Windows.h>
#endif // ENGINE_PLATFORM_WINDOWS
