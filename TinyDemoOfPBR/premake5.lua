project "TinyDemoOfPBR"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin_intermediate/" .. outputdir .. "/%{prj.name}")

    files
    {
        "src/**.h",
        "src/**.cpp",
        "vender/glm/glm/**.hpp",
        "vender/glm/glm/**.inl"
    }

    includedirs
    {
        "%{wks.location}/TinyDemoOfPBR/src",
        "%{include_dirs.spdlog}",
        "%{include_dirs.glfw}",
        "%{include_dirs.glad}",
        "%{include_dirs.glm}",
        "%{include_dirs.stb}"
    }

    links
    {
        "glfw",
        "Glad",
        "opengl32.lib"
    }

    filter "system:windows"
        cppdialect "C++17"
        systemversion "latest"

        defines
        {
            "GQY_PLATFORM_WINDOWS"
        }

    filter "configurations:Debug"
        defines "TINY_PBR_DEMO_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "TINY_PBR_DEMO_RELEASE"
        runtime "Release"
        optimize "on"
