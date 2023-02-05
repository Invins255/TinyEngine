workspace "TinyEngine"
    architecture "x64"
    startproject "TinyEngineEditor"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "TinyEngine/vendor/GLFW/include"
IncludeDir["Glad"] = "TinyEngine/vendor/Glad/include"
IncludeDir["ImGui"] = "TinyEngine/vendor/ImGui"
IncludeDir["glm"] = "TinyEngine/vendor/glm"
IncludeDir["stb_image"] = "TinyEngine/vendor/stb_image"
IncludeDir["entt"] = "TinyEngine/vendor/entt/include"

include "TinyEngine/vendor/GLFW"
include "TinyEngine/vendor/Glad"
include "TinyEngine/vendor/ImGui"

-----------------------------------------------
-- TinyEngine        
-----------------------------------------------
project "TinyEngine"
    location "TinyEngine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"    
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "pch.h"
    pchsource "TinyEngine/src/pch.cpp"

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
        "%{prj.name}/vendor/stb_image/**.h",
        "%{prj.name}/vendor/stb_image/**.cpp"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS"
    }

    includedirs
    {
        "%{prj.name}/src",
        "%{prj.name}/vendor/spdlog/include",
        "%{prj.name}/vendor/assimp/include",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.entt}"
    }

    links
    {
        "GLFW",
        "Glad",
        "ImGui",
        "opengl32.lib"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "ENGINE_PLATFORM_WINDOWS",
            "ENGINE_BUILD_DLL",
            "GLFW_INCLUDE_NONE"
        }

    filter "configurations:Debug"
        defines "ENGINE_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "ENGINE_RELEASE"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        defines "ENGINE_DIST"
        runtime "Release"
        optimize "on"

-----------------------------------------------
-- TinyEngineEditor        
-----------------------------------------------
project "TinyEngineEditor"
    location "TinyEngineEditor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "TinyEngine/src",
        "TinyEngine/vendor/spdlog/include", 
        "TinyEngine/vendor",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}",
        "TinyEngine/vendor/assimp/include"
    }

    links
    {
        "TinyEngine"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "ENGINE_PLATFORM_WINDOWS";
        }

    filter "configurations:Debug"
        defines "ENGINE_DEBUG"
        runtime "Debug"
        symbols "on"

        links
        {
            "TinyEngine/vendor/assimp/bin/Debug/assimp-vc141-mtd.lib"
        }

        postbuildcommands 
		{
			'{COPY} "../TinyEngine/vendor/assimp/bin/Debug/assimp-vc141-mtd.dll" "%{cfg.targetdir}"'
		}

    filter "configurations:Release"
        defines "ENGINE_RELEASE"
        runtime "Release"
        optimize "on"

        links
        {
            "TinyEngine/vendor/assimp/bin/Release/assimp-vc141-mtd.lib"
        }

        postbuildcommands 
		{
			'{COPY} "../TinyEngine/vendor/assimp/bin/Release/assimp-vc141-mtd.dll" "%{cfg.targetdir}"'
		}


    filter "configurations:Dist"
        defines "ENGINE_DIST"
        runtime "Release"
        optimize "on"

-----------------------------------------------
-- Sandbox    
-----------------------------------------------
project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "TinyEngine/src",
        "TinyEngine/vendor/spdlog/include"  , 
        "TinyEngine/vendor",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}"
    }

    links
    {
        "TinyEngine"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "ENGINE_PLATFORM_WINDOWS";
        }

    filter "configurations:Debug"
        defines "ENGINE_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "ENGINE_RELEASE"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        defines "ENGINE_DIST"
        runtime "Release"
        optimize "on"
