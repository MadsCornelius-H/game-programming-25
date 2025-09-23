cmake_minimum_required(VERSION 3.10)

project(imgui LANGUAGES CXX)

# Core ImGui sources
set(IMGUI_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
)

# Backends (you said they are in imgui/, not imgui/backends/)
set(IMGUI_BACKENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_sdl3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_sdlrenderer3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_sdlgpu3.cpp
)

add_library(imgui STATIC
    ${IMGUI_SRC}
    ${IMGUI_BACKENDS}
)

# Export include directories
target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
)

# Link against SDL3
target_link_libraries(imgui PUBLIC SDL3::SDL3)
