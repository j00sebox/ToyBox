workspace "gl_sandbox"
	startproject "gl_sandbox"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "gl_sandbox/external/glad"

project "gl_sandbox"
	location "gl_sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"

	pchheader "pch.h"
	pchsource "gl_sandbox/src/pch.cpp"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/external/stb/**.h",
		"%{prj.name}/external/stb/**.cpp",
		"%{prj.name}/external/json/json.hpp",
		"%{prj.name}/external/imgui/**.h",
		"%{prj.name}/external/imgui/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/src/renderer",
		"%{prj.name}/src/geometry",
		"%{prj.name}/src/events",
		"%{prj.name}/src/entities",
		"%{prj.name}/external",
		"%{prj.name}/external/stb",
		"%{prj.name}/external/glfw/include",
		"%{prj.name}/external/glad/include",
		"%{prj.name}/external/imgui"
	}

	libdirs { "%{prj.name}/external/glfw/lib" }

	links
	{
		"glfw3.lib",
		"GLAD"
	}

	filter "system:windows"
		
		systemversion "latest"

		defines { "PLATFORM_WINDOWS" }


	filter "system:linux"
		systemversion "latest"

		defines { "PLATFORM_LINUX" }
    
    filter "configurations:Debug"
		defines "DEBUG"
		runtime "Debug"
		symbols "On"
    
    filter "configurations:Release"
		defines "RELEASE"
		runtime "Release"
		optimize "On"

