/**
 * Mandelbrot Viewer — C++ / SFML + Dear ImGui
 *
 * Controls:
 *   Mouse wheel      – zoom in / out
 *   Left-click drag  – pan
 *   ImGui panel      – adjust max iterations, colour theme, and reset view
 */

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

// ── Mandelbrot maths ──────────────────────────────────────────────────────────

static int mandelbrot(double cx, double cy, int maxIter)
{
    double zx = 0.0, zy = 0.0;
    for (int i = 0; i < maxIter; ++i) {
        double zx2 = zx * zx, zy2 = zy * zy;
        if (zx2 + zy2 > 4.0)
            return i;
        zy = 2.0 * zx * zy + cy;
        zx = zx2 - zy2 + cx;
    }
    return maxIter;
}

// ── Colour themes ─────────────────────────────────────────────────────────────

enum class ColorTheme { Classic = 0, Fire, Ocean, Grayscale, COUNT };
static const char* themeNames[] = { "Classic", "Fire", "Ocean", "Grayscale" };

static sf::Color colorize(int iter, int maxIter, ColorTheme theme)
{
    if (iter == maxIter)
        return sf::Color::Black;

    float t = static_cast<float>(iter) / static_cast<float>(maxIter);

    switch (theme) {
    case ColorTheme::Classic: {
        auto channel = [](float v) {
            return static_cast<sf::Uint8>(std::clamp(std::sin(v) * 127.5f + 127.5f, 0.f, 255.f));
        };
        return { channel(t * 10.f), channel(t * 15.f), channel(t * 20.f) };
    }
    case ColorTheme::Fire: {
        return {
            static_cast<sf::Uint8>(std::clamp(t * 3.f * 255.f, 0.f, 255.f)),
            static_cast<sf::Uint8>(std::clamp(t * 1.5f * 255.f, 0.f, 255.f)),
            static_cast<sf::Uint8>(std::clamp(t * 0.5f * 255.f, 0.f, 255.f))
        };
    }
    case ColorTheme::Ocean: {
        return {
            static_cast<sf::Uint8>(std::clamp(t * 0.2f * 255.f, 0.f, 255.f)),
            static_cast<sf::Uint8>(std::clamp(t * 1.0f * 255.f, 0.f, 255.f)),
            static_cast<sf::Uint8>(std::clamp(t * 2.5f * 255.f, 0.f, 255.f))
        };
    }
    case ColorTheme::Grayscale:
    default: {
        sf::Uint8 v = static_cast<sf::Uint8>(t * 255.f);
        return { v, v, v };
    }
    }
}

// ── Renderer ──────────────────────────────────────────────────────────────────

struct Viewport {
    double centerX = -0.5;
    double centerY = 0.0;
    double zoom    = 3.0;   // world-space width
};

static void renderMandelbrot(sf::Image& img, const Viewport& vp,
                              int maxIter, ColorTheme theme)
{
    const unsigned W = img.getSize().x;
    const unsigned H = img.getSize().y;

    for (unsigned py = 0; py < H; ++py) {
        for (unsigned px = 0; px < W; ++px) {
            // Map pixel → complex plane
            double cx = vp.centerX + (static_cast<double>(px) / W - 0.5) * vp.zoom;
            double cy = vp.centerY - (static_cast<double>(py) / H - 0.5) * vp.zoom * H / W;

            int iter = mandelbrot(cx, cy, maxIter);
            img.setPixel(px, py, colorize(iter, maxIter, theme));
        }
    }
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main()
{
    constexpr unsigned WIN_W = 1280;
    constexpr unsigned WIN_H = 720;

    sf::RenderWindow window(
        sf::VideoMode(WIN_W, WIN_H),
        "Mandelbrot Viewer  —  C++ / SFML + ImGui",
        sf::Style::Default
    );
    window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(window))
        return 1;

    // Fractal pixel buffer
    sf::Image   fractalImage;
    sf::Texture fractalTexture;
    sf::Sprite  fractalSprite;

    fractalImage.create(WIN_W, WIN_H, sf::Color::Black);
    fractalTexture.create(WIN_W, WIN_H);

    Viewport    vp;
    int         maxIter   = 200;
    ColorTheme  theme     = ColorTheme::Classic;
    bool        dirty     = true;   // re-render on next frame

    // Pan state
    bool   panning = false;
    sf::Vector2i lastMousePos;

    // Render-quality warning (rendering is single-threaded / CPU)
    bool showInfo = false;

    sf::Clock deltaClock;

    while (window.isOpen()) {
        // ── Events ────────────────────────────────────────────────────────────
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);
            const bool imguiWantsMouse = ImGui::GetIO().WantCaptureMouse;

            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::Resized) {
                sf::FloatRect view(0, 0,
                    static_cast<float>(event.size.width),
                    static_cast<float>(event.size.height));
                window.setView(sf::View(view));
                fractalImage.create(event.size.width, event.size.height, sf::Color::Black);
                fractalTexture.create(event.size.width, event.size.height);
                dirty = true;
            }

            // Zoom with mouse wheel
            if (!imguiWantsMouse && event.type == sf::Event::MouseWheelScrolled) {
                float delta = event.mouseWheelScroll.delta;
                vp.zoom *= (delta > 0) ? 0.85 : 1.0 / 0.85;
                dirty = true;
            }

            // Pan with left-click drag
            if (!imguiWantsMouse && event.type == sf::Event::MouseButtonPressed
                && event.mouseButton.button == sf::Mouse::Left) {
                panning      = true;
                lastMousePos = { event.mouseButton.x, event.mouseButton.y };
            }
            if (event.type == sf::Event::MouseButtonReleased
                && event.mouseButton.button == sf::Mouse::Left) {
                panning = false;
            }
            if (!imguiWantsMouse && panning && event.type == sf::Event::MouseMoved) {
                unsigned W = fractalImage.getSize().x;
                unsigned H = fractalImage.getSize().y;
                int dx = event.mouseMove.x - lastMousePos.x;
                int dy = event.mouseMove.y - lastMousePos.y;
                vp.centerX -= static_cast<double>(dx) / W * vp.zoom;
                vp.centerY += static_cast<double>(dy) / H * vp.zoom * H / W;
                lastMousePos = { event.mouseMove.x, event.mouseMove.y };
                dirty = true;
            }
        }

        // ── ImGui update ──────────────────────────────────────────────────────
        ImGui::SFML::Update(window, deltaClock.restart());

        // Control panel
        ImGui::SetNextWindowPos({ 10.f, 10.f }, ImGuiCond_Once);
        ImGui::SetNextWindowSize({ 280.f, 0.f }, ImGuiCond_Once);
        ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoResize);

        ImGui::Text("Mandelbrot Viewer");
        ImGui::Separator();

        // Max iterations slider
        int prevIter = maxIter;
        ImGui::SliderInt("Max Iterations", &maxIter, 50, 1000);
        if (maxIter != prevIter) dirty = true;

        ImGui::Spacing();

        // Colour theme
        int themeInt = static_cast<int>(theme);
        if (ImGui::Combo("Colour Theme", &themeInt, themeNames,
                         static_cast<int>(ColorTheme::COUNT))) {
            theme = static_cast<ColorTheme>(themeInt);
            dirty = true;
        }

        ImGui::Spacing();
        ImGui::Separator();

        // View info
        ImGui::Text("Center: (%.6f, %.6f)", vp.centerX, vp.centerY);
        ImGui::Text("Zoom:   %.2e", vp.zoom);

        ImGui::Spacing();

        if (ImGui::Button("Reset View", { -1.f, 0.f })) {
            vp = Viewport{};
            dirty = true;
        }

        ImGui::Spacing();
        ImGui::Separator();
        if (ImGui::SmallButton("?  About")) showInfo = !showInfo;

        ImGui::End();

        // Info window
        if (showInfo) {
            ImGui::SetNextWindowSize({ 380.f, 0.f }, ImGuiCond_Once);
            ImGui::Begin("About the Mandelbrot Set", &showInfo);
            ImGui::TextWrapped(
                "The Mandelbrot set is the set of complex numbers c for "
                "which the function f(z) = z\u00b2 + c does not diverge "
                "when iterated from z = 0.");
            ImGui::Spacing();
            ImGui::TextWrapped("Controls:");
            ImGui::BulletText("Mouse wheel  –  zoom in / out");
            ImGui::BulletText("Left-click drag  –  pan");
            ImGui::BulletText("Adjust sliders in the Controls panel");
            ImGui::End();
        }

        // ── Render Mandelbrot if dirty ─────────────────────────────────────────
        if (dirty) {
            renderMandelbrot(fractalImage, vp, maxIter, theme);
            fractalTexture.update(fractalImage);
            fractalSprite.setTexture(fractalTexture, /*resetRect=*/true);
            dirty = false;
        }

        // ── Draw ──────────────────────────────────────────────────────────────
        window.clear();
        window.draw(fractalSprite);
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}
