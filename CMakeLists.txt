cmake_minimum_required(VERSION 3.13)
project(BackingGenerator)

set(CMAKE_OSX_SYSROOT /Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk)
#try using non big sur (<11) osx sdk
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -framework CoreAudio -framework AudioToolbox -framework AudioUnit -framework CoreFoundation -framework CoreServices -framework Carbon")

include_directories(include)
#find_library(cf CoreFoundation)
#message("found CoreFoundation at ${cf}")
link_directories(libs)

add_executable(BackingGenerator src/main.cpp
        src/Viewport.cpp
        src/Viewport.h
        src/Drawable.cpp
        src/Drawable.h
        src/Text.cpp
        src/Text.h
        src/Button.cpp
        src/Button.h
        src/Chord.cpp
        src/Chord.h
        src/Song.cpp
        src/Song.h
        src/AudioHandler.cpp
        src/AudioHandler.h src/VSTHost.cpp src/VSTHost.h)

target_link_libraries(BackingGenerator GLEW glfw3 freetype bz2 z portaudio -m64)