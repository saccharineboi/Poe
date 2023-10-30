-- Build script for Poe
-- Copyright (C) 2023 saccharineboi
--
-- This program is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program.  If not, see <https://www.gnu.org/licenses/>.

local required_programs = {"git", "cmake"}

function is_program_in_path(program)
    local handle = io.popen("command -v " .. program)
    local result = handle:read("*a")
    handle:close()
    return result:find('/') ~= nil
end

for _, program in ipairs(required_programs) do
    if not is_program_in_path(program) then
        print("premake.lua: " .. program .. " not found in system PATH")
        os.exit(1)
    end
end

function install_glfw()
    if os.execute("[ -d submodules/glfw ]") then
        return
    end

    os.execute("git submodule update --init --recursive")
    os.execute("mkdir -p submodules/glfw/build")

    local cmake_success = os.execute("cd submodules/glfw && cmake -B build")
    if not cmake_success then
        print("premake.lua: cmake failed at glfw")
        return
    end

    local make_success = os.execute("cd submodules/glfw/build && make")
    if not make_success then
        print("premake.lua: make failed at glfw")
        return
    end

    os.execute("mkdir -p include")
    os.execute("cp -r submodules/glfw/include/* include/")

    os.execute("mkdir -p lib")
    os.execute("cp submodules/glfw/build/src/libglfw3.a lib/")
end

install_glfw()

--------------------------------------------------
workspace "poe"
    configurations { "debug", "release", "testing" }
    location "build"

    --------------------------------------------------
    project "imgui"
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
            libdirs { "lib/linux64" }
            links { "m", "glfw3", "pthread", "GL" }

        filter "configurations:debug"
            defines { "_DEBUG", "DEBUG" }
            symbols "On"

        filter "configurations:release"
            defines { "NDEBUG" }
            optimize "On"

        filter "configurations:testing"
            defines { "NDEBUG" }
            optimize "On"
            symbols "On"

    --------------------------------------------------
    project "glad"
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

        filter "configurations:testing"
            defines { "NDEBUG" }
            optimize "On"
            symbols "On"

    --------------------------------------------------
    project "poe"
        kind "StaticLib"
        language "C++"
        location "build/poe"
        targetdir "build/%{cfg.buildcfg}"

        files {
            "src/Poe.cpp",
            "src/UI.cpp",
            "src/Cameras.cpp"
        }

        includedirs { "include", "src" }

        filter "system:linux"
            libdirs { "lib/linux64" }
            links { "m", "glfw3", "pthread", "GL", "assimp", "imgui", "glad" }

        filter "configurations:debug"
            defines { "_DEBUG", "DEBUG" }
            symbols "On"

        filter "configurations:testing"
            defines { "NDEBUG" }
            symbols "On"
            optimize "On"

        filter "configurations:release"
            defines { "NDEBUG" }
            optimize "On"

        filter { "system:linux", "action:gmake2" }
            buildoptions { "-std=c++20",
                           "-march=x86-64",
                           "-msse4.2",
                           "-Wall",
                           "-Wextra",
                           "-Werror",
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
                           "-Wformat-nonliteral",
                           "-Wformat-security",
                           "-Wformat-y2k",
                           "-Wformat=2",
                           "-Wno-unused-parameter",
                           "-Wunused",
                           "-Winvalid-pch",
                           "-Wlogical-op",
                           "-Wmissing-declarations",
                           "-Wmissing-field-initializers",
                           "-Wmissing-format-attribute",
                           "-Wmissing-include-dirs",
                           "-Wmissing-noreturn",
                           "-Wpacked",
                           "-Wredundant-decls",
                           "-Wstack-protector",
                           "-Wstrict-null-sentinel",
                           "-Wdisabled-optimization",
                           "-Wsign-conversion",
                           "-Wsign-promo",
                           "-Wstrict-aliasing=2",
                           "-Wstrict-overflow=2",
                           "-fno-rtti",
                           "-fno-exceptions",
                           "-Wno-suggest-attribute=format" }

        filter { "system:linux", "action:gmake2", "configurations:Debug" }
            buildoptions { "-Wno-unused-but-set-variable",
                           "-Wno-unused-variable",
                           "-Wno-unused-function",
                           "-fno-omit-frame-pointer" }

    --------------------------------------------------
    project "csitaly_demo"
        kind "ConsoleApp"
        language "C++"
        location "build/poe"
        targetdir "build/%{cfg.buildcfg}"

        files {
            "examples/CSItalyDemo.cpp"
        }

        includedirs { "include", "src" }

        filter "system:linux"
            libdirs { "lib/linux64" }
            links { "m", "glfw3", "pthread", "GL", "assimp", "imgui", "glad", "poe" }

        filter "configurations:debug"
            defines { "_DEBUG", "DEBUG" }
            symbols "On"

        filter "configurations:testing"
            defines { "NDEBUG" }
            symbols "On"
            optimize "On"

        filter "configurations:release"
            defines { "NDEBUG" }
            optimize "On"

        filter { "system:linux", "action:gmake2" }
            buildoptions { "-std=c++20",
                           "-march=x86-64",
                           "-msse4.2",
                           "-Wall",
                           "-Wextra",
                           "-Werror",
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
                           "-Wformat-nonliteral",
                           "-Wformat-security",
                           "-Wformat-y2k",
                           "-Wformat=2",
                           "-Wno-unused-parameter",
                           "-Wunused",
                           "-Winvalid-pch",
                           "-Wlogical-op",
                           "-Wmissing-declarations",
                           "-Wmissing-field-initializers",
                           "-Wmissing-format-attribute",
                           "-Wmissing-include-dirs",
                           "-Wmissing-noreturn",
                           "-Wpacked",
                           "-Wredundant-decls",
                           "-Wstack-protector",
                           "-Wstrict-null-sentinel",
                           "-Wdisabled-optimization",
                           "-Wsign-conversion",
                           "-Wsign-promo",
                           "-Wstrict-aliasing=2",
                           "-Wstrict-overflow=2",
                           "-fno-rtti",
                           "-fno-exceptions",
                           "-Wno-suggest-attribute=format" }

        filter { "system:linux", "action:gmake2", "configurations:Debug" }
            buildoptions { "-Wno-unused-but-set-variable",
                           "-Wno-unused-variable",
                           "-Wno-unused-function",
                           "-fno-omit-frame-pointer" }
