#!/usr/bin/env bash
# Install C++ dependencies for the Mandelbrot Viewer dev environment
set -e

echo "==> Updating package list..."
sudo apt-get update -y

echo "==> Installing SFML and build tools..."
sudo apt-get install -y \
    libsfml-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libxrandr-dev \
    libxcursor-dev \
    libudev-dev \
    libfreetype-dev \
    libopenal-dev \
    libflac-dev \
    libvorbis-dev \
    libx11-dev \
    cmake \
    ninja-build \
    git

echo "==> Cloning Dear ImGui..."
IMGUI_DIR="/opt/imgui"
IMGUI_SFML_DIR="/opt/imgui-sfml"

if [ ! -d "$IMGUI_DIR" ]; then
    sudo git clone --depth=1 --branch v1.90.4 https://github.com/ocornut/imgui.git "$IMGUI_DIR"
fi

echo "==> Cloning ImGui-SFML..."
if [ ! -d "$IMGUI_SFML_DIR" ]; then
    sudo git clone --depth=1 --branch v2.6 https://github.com/SFML/imgui-sfml.git "$IMGUI_SFML_DIR"
fi

echo "==> All dependencies installed successfully!"
echo "==> Build the project with:"
echo "    cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j\$(nproc)"
echo "==> Run the project with:"
echo "    ./build/mandelbrot-viewer"
