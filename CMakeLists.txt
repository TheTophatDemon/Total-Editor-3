cmake_minimum_required(VERSION 3.0.0)
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

add_executable(TotalEditor3 ${SRC_FILES})

target_link_libraries(TotalEditor3 PRIVATE imgui glm
        "${CMAKE_SOURCE_DIR}/glew/lib/glew32.lib" 
        "${CMAKE_SOURCE_DIR}/sdl2/lib/SDL2.lib" 
        "${CMAKE_SOURCE_DIR}/sdl2_image/lib/SDL2_image.lib" 
        opengl32.lib GlU32.lib)
#add -O3 or -O2 when releasing
target_compile_options(TotalEditor3 PRIVATE -w -Wl,-subsystem,console)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
