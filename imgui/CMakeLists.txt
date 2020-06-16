add_library(
    imgui
    src/imgui_demo.cpp
    src/imgui_draw.cpp
    src/imgui_impl_opengl3.cpp
    src/imgui_impl_sdl.cpp
    src/imgui_widgets.cpp
    src/imgui.cpp
    headers/imconfig.h
    headers/imgui_impl_opengl3.h
    headers/imgui_impl_sdl.h
    headers/imgui_internal.h
    headers/imgui.h
    headers/imstb_rectpack.h
    headers/imstb_textedit.h
    headers/imstb_truetype.h
)
target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/headers")
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)