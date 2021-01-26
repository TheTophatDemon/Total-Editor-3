cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
project(TotalEditor3 VERSION 0.1.0 LANGUAGES CXX)

file(GLOB SRC_FILES *.cpp)

include_directories("${CMAKE_SOURCE_DIR}/glew/include" 
        "${CMAKE_SOURCE_DIR}/sdl2/include" 
        "${CMAKE_SOURCE_DIR}/sdl2_image/include"
        "${CMAKE_SOURCE_DIR}/glm")
link_directories("${CMAKE_SOURCE_DIR}/glew/lib" 
        "${CMAKE_SOURCE_DIR}/sdl2/lib" 
        "${CMAKE_SOURCE_DIR}/sdl2_image/lib")

add_subdirectory(imgui)
add_subdirectory(glm)
add_library(glm::glm ALIAS glm)

message(STATUS ${SRC_FILES})
add_executable(TotalEditor3 ${SRC_FILES})

target_link_libraries(TotalEditor3 PRIVATE imgui glm
        "${CMAKE_SOURCE_DIR}/glew/lib/glew32.lib" 
        "${CMAKE_SOURCE_DIR}/sdl2/lib/libSDL2main.a" 
        "${CMAKE_SOURCE_DIR}/sdl2/lib/libSDL2.a" 
        "${CMAKE_SOURCE_DIR}/sdl2_image/lib/libSDL2_image.a" 
        opengl32.lib GlU32.lib winmm gdi32 dxguid ws2_32 ole32 imm32 kernel32 hal user32 hid ntdll oleaut32 version uuid advapi32 setupapi shell32)
#add -O3 or -O2 when releasing
target_compile_options(TotalEditor3 PRIVATE -w -Wl,-subsystem,console,)
target_link_options(TotalEditor3 PRIVATE -mwindows -m64)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
