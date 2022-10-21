-- Copyright (C) 2022 saccharineboi --

--------------------------------------------------
workspace "poe"
    configurations { "debug", "release" }
    location "build"

    --------------------------------------------------
    project "poe_imgui"
        kind "StaticLib"
        language "C++"
        location "build/imgui"
        targetdir "build/%{cfg.buildcfg}"

        files {
            "tp/imgui/imgui.cpp",
            "tp/imgui/imgui_demo.cpp",
            "tp/imgui/imgui_draw.cpp",
            "tp/imgui/imgui_impl_glfw.cpp",
            "tp/imgui/imgui_impl_opengl3.cpp",
            "tp/imgui/imgui_tables.cpp",
            "tp/imgui/imgui_widgets.cpp",
        }

        includedirs { "include", "include/imgui" }

        filter "system:linux"
            links { "m", "glfw", "pthread", "GL" }

        filter "configurations:debug"
            defines { "_DEBUG", "DEBUG" }
            symbols "On"

        filter "configurations:release"
            defines { "NDEBUG" }
            optimize "On"

        filter{}

    --------------------------------------------------
    project "poe_glad"
        kind "StaticLib"
        language "C"
        location "build/glad"
        targetdir "build/%{cfg.buildcfg}"

        files { "tp/glad/glad.cpp" }

        includedirs { "include" }

        filter "system:linux"
            links { "m", "GL" }

        filter "configurations:debug"
            defines { "_DEBUG", "DEBUG" }
            symbols "On"

        filter "configurations:release"
            defines { "NDEBUG" }
            optimize "On"
        filter{}

    --------------------------------------------------
    project "poe"
        kind "WindowedApp"
        language "C++"
        location "build/poe"
        targetdir "build/%{cfg.buildcfg}"

        files {
            "src/Poe.cpp",
            "src/UI.cpp",
            "src/Cameras.cpp",
            "src/Demo_cs_italy.cpp",
            "main.cpp"
        }

        includedirs { "include", "src" }

        filter "system:linux"
            links { "m", "glfw", "pthread", "GL", "assimp", "poe_imgui", "poe_glad" }

        filter "configurations:debug"
            defines { "_DEBUG", "DEBUG" }
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
        filter{}
