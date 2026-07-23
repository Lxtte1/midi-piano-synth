project "midifile"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    targetname "midifile"
    targetdir "bin/"
    objdir "bin-int/"

    files { "midifile/src/**.cpp" }
    includedirs { "midifile/include" }
    links {}