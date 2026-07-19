workspace "Digital Piano"
    configurations { "Debug" }
    platforms { "x64" }

project "Digital Piano"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    targetdir "bin/%{cfg.buildcfg}"
    targetname "Piano"
    objdir "bin-int/%{cfg.buildcfg}"

    files { "src/**.cpp" }
    includedirs { "include", "/usr/include/qt6", "/usr/include/qt6/QtCore", "/usr/include/qt6/QtWidgets", "/usr/include/qt6/QtGui", "/usr/include/pipewire-0.3", "/usr/include/spa-0.2" }
    links { "Qt6Core", "Qt6Widgets", "Qt6Gui", "pipewire-0.3" }

    filter "system:linux"
        buildoptions { "-fPIC" }
        linkoptions { }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"