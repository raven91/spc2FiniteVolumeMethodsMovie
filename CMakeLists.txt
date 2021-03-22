cmake_minimum_required(VERSION 3.10)
project(spc2FiniteVolumeMethodsMovie)

set(CMAKE_CXX_STANDARD 11)

add_executable(spc2FiniteVolumeMethodsMovie
        main.cpp
        ScreenshotHandling.cpp ParameterContainer.cpp
        ParameterContainer.hpp
        Definitions.hpp Definitions.cpp
        SimulationModel.cpp SimulationModel.hpp
        Renderer.cpp Renderer.hpp
        ScreenshotHandler.cpp ScreenshotHandler.hpp)

include_directories(/usr/local/include/ /usr/local/include/freetype2/)
link_directories(/usr/local/lib)
set(CMAKE_OSX_SYSROOT /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk)

file(GLOB LIBRARIES "/usr/local/lib/libGLEW.1.13.0.dylib"
        "/usr/local/lib/libglfw.3.1.dylib"
        "/usr/local/lib/libpng16.16.dylib"
        "/usr/local/lib/libfreetype.a"
        "/usr/local/opt/bzip2/lib/libbz2.a"
        "/usr/local/opt/zlib/lib/libz.a")
message("LIBRARIES = ${LIBRARIES}")
target_link_libraries(spc2FiniteVolumeMethodsMovie ${LIBRARIES})

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-framework OpenGL" COMPILER_SUPPORTS_FRAMEWORK_OPENGL)
if(COMPILER_SUPPORTS_FRAMEWORK_OPENGL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework OpenGL")
endif()

CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()
CHECK_CXX_COMPILER_FLAG("-O3" COMPILER_SUPPORTS_O3)
if(COMPILER_SUPPORTS_O3)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()
CHECK_CXX_COMPILER_FLAG("-flto" COMPILER_SUPPORTS_FLTO)
if(COMPILER_SUPPORTS_FLTO)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
endif()