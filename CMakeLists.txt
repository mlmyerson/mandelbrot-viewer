cmake_minimum_required(VERSION 3.20)
project(mandelbrot-viewer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Find SFML ──────────────────────────────────────────────────────────────────
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# ── Dear ImGui source files (vendored via install-deps.sh) ─────────────────────
set(IMGUI_DIR "/opt/imgui" CACHE PATH "Path to Dear ImGui source")
set(IMGUI_SFML_DIR "/opt/imgui-sfml" CACHE PATH "Path to imgui-sfml source")

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_SFML_DIR}/imgui-SFML.cpp
)

# ── Executable ─────────────────────────────────────────────────────────────────
add_executable(${PROJECT_NAME}
    src/main.cpp
    ${IMGUI_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_SFML_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    GL
)

# Optimise release builds
if(CMAKE_BUILD_TYPE MATCHES "^[Rr]elease$")
    target_compile_options(${PROJECT_NAME} PRIVATE -O3 -march=native)
endif()

message(STATUS "IMGUI_DIR     = ${IMGUI_DIR}")
message(STATUS "IMGUI_SFML_DIR = ${IMGUI_SFML_DIR}")
