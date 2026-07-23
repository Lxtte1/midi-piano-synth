workspace "Digital Piano"
    configurations { "Debug" }
    platforms { "x64" }

require "midifile"

project "DigitalPiano"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    targetname "Piano"
    targetdir "bin/"
    objdir "bin-int/"

    files { "src/**.cpp" }
    includedirs { "include", "/usr/include/qt6", "/usr/include/qt6/QtCore", "/usr/include/qt6/QtWidgets", "/usr/include/qt6/QtGui", "/usr/include/pipewire-0.3", "/usr/include/spa-0.2", "midifile/include" }
    links { "Qt6Core", "Qt6Widgets", "Qt6Gui", "pipewire-0.3", "asound", "midifile" }

    filter "system:linux"
        buildoptions { "-fPIC" }
        linkoptions { }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
