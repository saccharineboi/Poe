-- Copyright (C) 2022 saccharineboi --

workspace "poe"
    configurations { "debug", "release" }
    location "build"

project "poe"
    kind "ConsoleApp"
    language "C++"
    location "build/poe"
    targetdir "build/%{cfg.buildcfg}"

    files {
        "src/main.cpp",
        "tp/glad/glad.cpp",
        "src/Poe.cpp",
        "src/Demo.cpp"
    }

    includedirs { "include", "source" }

    filter "system:linux"
        links { "m", "glfw", "pthread", "GL", "assimp" }

    filter "configurations:debug"
        defines { "_DEBUG" }
        symbols "On"

    filter "configurations:release"
        defines { "NDEBUG" }
        optimize "On"
        symbols "On"

    filter { "system:linux", "action:gmake2" }
        buildoptions { "-std=c++20",
                       "-O1",
                       "-march=native",
                       "-Wall",
                       "-Wextra",
                       "-Wpedantic",
                       "-Wfloat-equal",
                       "-Wundef",
                       "-Wshadow",
                       "-Wpointer-arith",
                       "-Wcast-align",
                       "-Wwrite-strings",
                       "-Wswitch-enum",
                       "-Wcast-qual",
                       "-Wconversion",
                       "-Wduplicated-cond",
                       "-Wduplicated-branches",
                       "-Wnon-virtual-dtor",
                       "-Woverloaded-virtual",
                       "-Wold-style-cast",
                       "-Wno-unused-parameter",
                       "-fno-rtti",
                       "-fno-exceptions" }

    filter { "system:linux", "action:gmake2", "configurations:Debug" }
        buildoptions { "-Wno-unused-but-set-variable",
                       "-Wno-unused-variable" }
