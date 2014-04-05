solution "Templet"
    configurations {"Release", "Debug"}
    
    project "test_all"
        kind "ConsoleApp"
        language "C++"
        location "build"
        files {
            "test_all.cpp",
            "../*.cpp"
        }
        includedirs {"../", "../gtest/include"}
        libdirs {"../gtest/build"}
        links {"libgtest"}
        
    configuration "Release"
        targetdir "build/release"
        buildoptions {"-O3", "-Wall", "-Wextra", "-Wpedantic"}
        
    configuration "Debug"
        targetdir "build/debug"
        flags {"Symbols"}
        
        