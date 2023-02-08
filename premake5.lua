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
IncludeDir["spdlog"] = "TinyEngine/vendor/spdlog/include"
IncludeDir["stb_image"] = "TinyEngine/vendor/stb_image"
IncludeDir["entt"] = "TinyEngine/vendor/entt/include"
IncludeDir["assimp"] = "TinyEngine/vendor/assimp/include"

LibraryDir = {}
LibraryDir["yaml_cpp"] = "vendor/yaml-cpp/bin/Release/yaml-cpp.lib"
LibraryDir["yaml_cppd"] = "vendor/yaml-cpp/bin/Debug/yaml-cppd.lib"

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
    staticruntime "off"

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
        "%{prj.name}/vendor/yaml-cpp/include",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.assimp}"
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

        links
        {
            "%{LibraryDir.yaml_cppd}"
        }

    filter "configurations:Release"
        defines "ENGINE_RELEASE"
        runtime "Release"
        optimize "on"

        links
        {
            "%{LibraryDir.yaml_cpp}"
        }

-----------------------------------------------
-- TinyEngineEditor        
-----------------------------------------------
project "TinyEngineEditor"
    location "TinyEngineEditor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

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
        "TinyEngine/vendor",
        "%{IncludeDir.glm}",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.assimp}"
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

        postbuildcommands 
		{
			'{COPY} "../TinyEngine/vendor/assimp/bin/Debug/assimp-vc141-mtd.dll" "%{cfg.targetdir}"'
		}

        links
        {
            "TinyEngine/vendor/assimp/bin/Debug/assimp-vc141-mtd.lib"
        }

    filter "configurations:Release"
        defines "ENGINE_RELEASE"
        runtime "Release"
        optimize "on"

        postbuildcommands 
		{
			'{COPY} "../TinyEngine/vendor/assimp/bin/Release/assimp-vc141-mt.dll" "%{cfg.targetdir}"'
		}

        links
        {
            "TinyEngine/vendor/assimp/bin/Release/assimp-vc141-mt.lib"
        }

-----------------------------------------------
-- Sandbox    
-----------------------------------------------
project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

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
