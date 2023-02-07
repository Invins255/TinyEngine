#pragma once

//--------------------------------------
//Precompile Header
//--------------------------------------

#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <string>
#include <functional>
#include <array>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Engine/Core/Log.h"

#ifdef ENGINE_PLATFORM_WINDOWS
	#include <Windows.h>
#endif // ENGINE_PLATFORM_WINDOWS
