-- premake-1.lua
workspace "ponga"
configurations { "Debug", "Release" }

project "ponga"
kind "ConsoleApp"
language "C"
targetdir "bin/%{cfg.buildcfg}"

files { "**.h", "**.c" }

filter "configurations:Debug"
defines { "DEBUG" }
symbols "On"

filter "configurations:Release"
defines { "NDEBUG" }
optimize "On"
