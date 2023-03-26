workspace "TinyDemoOfPBR"
    architecture "x64"
    startproject "TinyDemoOfPBR"

    configurations
    {
        "Debug",
        "Release"
    }

    flags
    {
        "MultiProcessorCompile"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include_dirs = {}
include_dirs["glfw"] = "%{wks.location}/TinyDemoOfPBR/vender/glfw/include"
include_dirs["glad"] = "%{wks.location}/TinyDemoOfPBR/vender/glad/include"
include_dirs["spdlog"] = "%{wks.location}/TinyDemoOfPBR/vender/spdlog/include"
include_dirs["glm"] = "%{wks.location}/TinyDemoOfPBR/vender/glm"
include_dirs["stb"] = "%{wks.location}/TinyDemoOfPBR/vender/stb"

group "Dependencies"
    include "TinyDemoOfPBR/vender"
group ""

group "Core"
    include "TinyDemoOfPBR"
group ""