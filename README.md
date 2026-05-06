# Mandelbrot Viewer

An interactive Mandelbrot set viewer with two front-ends:

| Front-end | Technology | How to run |
|-----------|-----------|------------|
| **Web** | HTML5 + WebGL | Open `index.html` in a browser |
| **Desktop (C++)** | C++ 17, SFML 2, Dear ImGui | See below |

---

## C++ Desktop Application

### Prerequisites — Dev Container (recommended)

1. Install [Docker](https://www.docker.com/) and the
   [VS Code Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers).
2. Open this repository folder in VS Code.
3. When prompted, click **"Reopen in Container"**  
   (or run **Dev Containers: Reopen in Container** from the command palette).

The container automatically runs `.devcontainer/install-deps.sh`, which
installs SFML 2 and Dear ImGui, so you are ready to build straight away.

### Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### Run

```bash
./build/mandelbrot-viewer
```

### Controls

| Action | Effect |
|--------|--------|
| Mouse wheel | Zoom in / out |
| Left-click drag | Pan |
| **Max Iterations** slider | Trade render quality vs. speed |
| **Colour Theme** combo | Switch colour palette |
| **Reset View** button | Return to the default view |
| **? About** button | Show help and info |

### Project layout

```
.
├── .devcontainer/
│   ├── devcontainer.json   # VS Code Dev Container definition
│   └── install-deps.sh     # Installs SFML + Dear ImGui inside the container
├── src/
│   └── main.cpp            # C++ Mandelbrot viewer (SFML + ImGui)
├── CMakeLists.txt          # CMake build definition
├── index.html              # Web front-end (no build step required)
└── README.md
```

### GUI framework

The C++ application uses two libraries:

* **[SFML 2](https://www.sfml-dev.org/)** — Simple and Fast Multimedia
  Library.  Provides the window, hardware-accelerated 2-D drawing, event
  handling, and pixel-buffer management.
* **[Dear ImGui](https://github.com/ocornut/imgui)** (via
  [imgui-sfml](https://github.com/SFML/imgui-sfml)) — Immediate-mode GUI
  toolkit.  Lets you add sliders, combo-boxes, buttons, and text with a
  single line of C++ each, making the UI extremely easy to extend.
