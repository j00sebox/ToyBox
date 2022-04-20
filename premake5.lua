workspace "gl_sandbox"
	startproject "gl_sandbox"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "gl_sandbox"
	location "gl_sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/glfw/include"
	}

	libdirs { "%{prj.name}/vendor/glfw/lib" }

	links
	{
		"glfw3.lib"
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

