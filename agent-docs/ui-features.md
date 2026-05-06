# Mandelbrot Viewer – UI & UX Feature Design Report

## Executive Summary

This report surveys best-in-class UI/UX patterns for interactive Mandelbrot set viewers and fractal explorers, then maps them against the current implementation (`index.html`) to produce a prioritised, actionable feature list. Features are grouped by concern, rated by impact and implementation effort, and accompanied by design rationale.

---

## 1. Current State Audit

The existing viewer is a single-file WebGL application with the following capabilities:

| Feature | Status |
|---|---|
| Full-screen WebGL canvas render | ✅ Implemented |
| Click-drag to pan | ✅ Implemented |
| Mouse-wheel zoom | ✅ Implemented |
| Arrow-key zoom | ✅ Implemented |
| Max-iterations slider (50–2000) | ✅ Implemented |
| Two colour themes (Classic, Fire) | ✅ Implemented |
| Info modal (what is this?) | ✅ Implemented |
| Settings modal | ✅ Implemented |
| Touch / mobile support | ❌ Missing |
| Smooth/animated zoom | ❌ Missing |
| Coordinate HUD | ❌ Missing |
| Bookmark / history system | ❌ Missing |
| Deep-link / shareable URL | ❌ Missing |
| Export / screenshot | ❌ Missing |
| Julia set companion view | ❌ Missing |
| Additional colour palettes | ❌ Missing |
| Keyboard shortcut reference | ❌ Missing |
| Progressive / adaptive rendering | ❌ Missing |
| Accessibility (ARIA, high-contrast) | ❌ Missing |

---

## 2. Feature Catalogue

Features are rated **Impact** (H/M/L) and **Effort** (H/M/L).

---

### 2.1 Navigation & Exploration

#### 2.1.1 Touch / Pinch-to-Zoom Support
**Impact: H | Effort: M**

Mobile and tablet users cannot currently interact with the viewer. Adding touch event handlers (`touchstart`, `touchmove`, `touchend`) with two-finger pinch gesture detection for zoom and single-finger drag for pan is essential for any public-facing viewer.

*Design notes:*
- Use `Touch.identifier` to track two simultaneous touches.
- Derive zoom delta from the ratio of current to previous touch-point distance.
- Disable native page scroll/zoom while the canvas has focus (`touch-action: none`).

#### 2.1.2 Smooth Animated Zoom
**Impact: H | Effort: M**

Discrete zoom steps (current: multiplicative factor per wheel tick) create a jarring experience. Smooth zoom uses `requestAnimationFrame` to interpolate zoom level toward a target, giving a fluid, "Google Maps"-style feel.

*Design notes:*
- Maintain a `targetZoom` and lerp `zoom → targetZoom` each frame.
- Zoom should be anchored to the cursor position, not just the screen centre.
- Allow cancellation (start zooming the other direction) mid-animation.

#### 2.1.3 Coordinate HUD (Heads-Up Display)
**Impact: H | Effort: L**

Displaying the current mathematical centre coordinates and zoom level gives the user spatial awareness and enables manual navigation. Fractal explorers that omit this force users to rely entirely on muscle memory.

*Design notes:*
- Show `Re`, `Im` (real and imaginary parts of centre), and a human-readable zoom scale (e.g. `×1,000,000`).
- Position in the bottom-right corner to avoid obscuring the UI buttons.
- Use a monospace font for readability; include a subtle dark background chip.
- Update coordinates live during pan/zoom (debounce display updates to ≤ 30 fps to avoid visual noise).

#### 2.1.4 Cursor Coordinate Tooltip
**Impact: M | Effort: L**

Show the complex coordinates under the cursor in a small floating tooltip. This helps users identify interesting points before committing to a zoom.

#### 2.1.5 Reset / Home Button
**Impact: H | Effort: L**

A single-click "Reset" control returns the view to the canonical starting position (`center = (−0.5, 0)`, `zoom = 3.0`). Essential for users who become lost during deep exploration.

*Design notes:*
- Place it alongside the existing toolbar buttons.
- Optionally animate the transition back to home using smooth zoom/pan interpolation.

#### 2.1.6 Bookmark System
**Impact: H | Effort: M**

Allow users to save named bookmarks (stored in `localStorage`) and revisit them from a dropdown or sidebar. Many iconic Mandelbrot locations (Seahorse Valley, Elephant Valley, Mini-brots) deserve pre-loaded default bookmarks.

*Design notes:*
- Each bookmark stores: `name`, `centerX`, `centerY`, `zoom`, `maxIter`, `colorTheme`.
- Provide a handful of curated default bookmarks covering famous locations.
- Allow user-defined bookmarks with an editable name field.
- Render a small thumbnail of each saved bookmark for quick identification.

#### 2.1.7 Navigation History (Back / Forward)
**Impact: M | Effort: M**

Maintain a history stack of recent view states so users can undo accidental zoom moves. Mirror the browser back/forward metaphor with arrow buttons or keyboard shortcuts (`Alt+←` / `Alt+→`).

#### 2.1.8 Coordinate Jump (Go To Location)
**Impact: M | Effort: L**

A form where users type in specific real/imaginary coordinates and a zoom level, then jump directly to that location. Supports community sharing of coordinates via text.

---

### 2.2 Colour & Visual Quality

#### 2.2.1 Extended Colour Palette Library
**Impact: H | Effort: M**

The current two themes (Classic, Fire) are the bare minimum. A well-regarded fractal viewer should offer at least 8–12 distinct palettes. Suggested additions:

| Palette Name | Character |
|---|---|
| Ocean | Blues and teals with white crests |
| Neon | High-contrast cyan/magenta on black |
| Grayscale | Neutral for accessibility |
| Sunset | Warm orange/pink/purple gradients |
| Ice | Cool whites and pale blues |
| Earth | Browns, greens, beige |
| Psychedelic | Ultra-saturated cycling hues |
| Monochrome Red | Single-hue red gradient |

*Design notes:*
- Implement each palette as a GLSL function selected by a `u_colorTheme` uniform.
- Show colour swatch previews in the settings dropdown.

#### 2.2.2 Smooth Colouring (Escape-Time Normalisation)
**Impact: H | Effort: M**

Integer iteration counts produce visible "banding" – abrupt colour boundaries that make the fractal look blocky. Smooth colouring uses the fractional escape time formula:

```
t_smooth = iter - log2(log2(|z|))
```

This produces continuous gradients across iteration bands, dramatically improving visual quality at all zoom levels.

#### 2.2.3 Colour Cycling / Animation
**Impact: M | Effort: M**

Animate the colour palette offset over time to create a mesmerising "flowing" effect without changing the underlying fractal geometry. Provide a play/pause toggle and a speed slider.

#### 2.2.4 Custom Gradient Editor
**Impact: M | Effort: H**

An advanced feature: allow users to define their own colour gradients by dragging colour stop handles along a gradient bar. Store custom gradients as `localStorage` items.

#### 2.2.5 Render Quality Presets
**Impact: M | Effort: L**

Complement the raw iteration slider with human-readable presets:

| Preset | Iterations | Use Case |
|---|---|---|
| Draft | 50 | Fast exploration |
| Normal | 200 | Default |
| High | 500 | Detailed views |
| Ultra | 2000 | Export / print |

#### 2.2.6 Anti-Aliasing Toggle
**Impact: M | Effort: M**

Supersampling (rendering at 2× resolution and downscaling) or multi-sample averaging within the shader improves edge sharpness, especially at lower zoom levels. This should be optional because it quadruples the render cost.

---

### 2.3 Sharing & Export

#### 2.3.1 Deep-Link / Shareable URL
**Impact: H | Effort: L**

Encode the current view state (center, zoom, maxIter, colorTheme) into the URL hash or query string, e.g.:

```
https://example.com/#cx=-0.7269&cy=0.1889&z=50000&i=500&c=2
```

When the page loads with these parameters, it restores the saved view automatically. Users can copy the browser URL and share it directly with others.

*Design notes:*
- Use `history.replaceState()` to update the URL without a page reload.
- Provide a "Copy Link" button that copies the current URL to the clipboard and shows a brief confirmation toast.

#### 2.3.2 Screenshot / Export
**Impact: H | Effort: M**

Add a "Save Image" button that exports the current canvas as a PNG. Users frequently want to save and share striking fractal renders.

*Design notes:*
- Use `canvas.toDataURL('image/png')` and trigger a synthetic `<a download>` click.
- Optionally support higher-resolution export by temporarily re-rendering at a user-specified resolution (e.g. 4K).
- Provide metadata overlay option (coordinates, iteration count) as an image watermark.

#### 2.3.3 Copy Coordinates Button
**Impact: L | Effort: L**

A small clipboard icon beside the coordinate HUD that copies the current centre coordinates as a formatted string.

---

### 2.4 Educational & Analytical Tools

#### 2.4.1 Julia Set Companion Panel
**Impact: H | Effort: H**

The Julia set for a complex parameter `c` is intimately related to the Mandelbrot set: hovering the cursor over a point `c` in the Mandelbrot view live-updates a small inset panel showing the corresponding Julia set. This is one of the most powerful pedagogical features in any fractal viewer.

*Design notes:*
- Render the Julia set in a resizable inset canvas (e.g. bottom-right, 300×300 px).
- Use the cursor's complex coordinate as the Julia `c` parameter.
- Allow the panel to be detached into a second full-screen view for comparison.
- Debounce shader re-renders to limit GPU load during fast mouse movement.

#### 2.4.2 Orbit Trace Overlay
**Impact: M | Effort: H**

For a clicked point, draw the iterative orbit `z₀ → z₁ → z₂ → …` as a series of arrows or dots overlaid on the canvas using a 2D canvas layer. This helps users understand the mathematics of why a point is inside or outside the set.

#### 2.4.3 Mathematical Annotations Panel
**Impact: M | Effort: L**

An expandable side panel explaining key mathematical concepts contextualised by the current view:
- Current formula: `f_c(z) = z² + c`
- Period of the visible bulb/bud (if detectable)
- Approximate symmetry axis

#### 2.4.4 Iteration Heat-Map Toggle
**Impact: M | Effort: M**

An overlay mode that renders iteration count as a raw heat-map (cool colours = few iterations, hot = many), separate from the chosen aesthetic palette. Useful for understanding the structure before choosing a colour scheme.

---

### 2.5 Performance & Progressive Rendering

#### 2.5.1 Progressive Rendering
**Impact: H | Effort: H**

After every pan or zoom, render a coarse version immediately (low iterations, possibly sub-sampled resolution) and progressively refine it. This eliminates the blank-screen delay on slow GPUs.

*Design notes:*
- Render at 25% resolution first, then 50%, then full.
- Cancel in-progress refinement on the next user interaction.
- Show a subtle "Rendering…" spinner in the corner during refinement.

#### 2.5.2 Adaptive Iteration Ceiling
**Impact: M | Effort: M**

Automatically increase `maxIter` as the zoom level increases (deeper zooms require more iterations to distinguish set membership). A good heuristic: `maxIter = max(200, 100 × log10(1/zoom))`.

#### 2.5.3 WebGL2 / WebGPU Upgrade Path
**Impact: M | Effort: H**

The current app uses WebGL 1. Migrating to WebGL2 enables double-precision workarounds (via two-float arithmetic) to render deeper without the floating-point precision limit (~10⁻⁷). For even deeper exploration, a WebGPU backend is the long-term target.

#### 2.5.4 Worker-Based CPU Fallback
**Impact: L | Effort: H**

For browsers without WebGL support, a Web Worker-based CPU renderer using `OffscreenCanvas` provides a fallback, albeit slower. Uses the same coordinate math as the shader.

---

### 2.6 Accessibility

#### 2.6.1 Full Keyboard Navigation
**Impact: H | Effort: M**

Extend beyond the current arrow-key zoom to include:

| Key | Action |
|---|---|
| `W` / `A` / `S` / `D` or arrow keys | Pan up / left / down / right |
| `+` / `=` | Zoom in |
| `-` | Zoom out |
| `R` | Reset to home |
| `B` | Open bookmarks |
| `E` | Export image |
| `?` | Show keyboard shortcut reference |
| `Esc` | Close any open modal |

#### 2.6.2 Keyboard Shortcut Reference Modal
**Impact: M | Effort: L**

A `?` button or `?` key that opens a clean modal listing all keyboard shortcuts, styled as a cheat-sheet grid.

#### 2.6.3 ARIA Labels & Roles
**Impact: M | Effort: L**

Add `aria-label`, `role`, and `aria-live` attributes to all interactive controls so screen-reader users can understand the interface. The canvas should have `role="img"` with a dynamic `aria-label` reporting the current coordinates.

#### 2.6.4 High-Contrast / Accessible Colour Mode
**Impact: M | Effort: M**

A toggle in Settings that switches to a high-contrast palette (black-and-white or high-contrast greyscale) for users with colour vision deficiency. Also provide explicit greyscale and deuteranopia-safe palettes in the colour library.

#### 2.6.5 Reduced-Motion Mode
**Impact: L | Effort: L**

Respect the `prefers-reduced-motion` media query: disable smooth animated zoom and colour cycling when the user has set this OS preference.

---

### 2.7 UI Shell & Layout

#### 2.7.1 Collapsible / Auto-Hiding Toolbar
**Impact: M | Effort: M**

The current toolbar is always visible in the top-left. An auto-hide behaviour (fade out after 3 seconds of inactivity, reappear on mouse movement) maximises the canvas viewing area.

*Design notes:*
- Show a small semi-transparent pill or hamburger icon that always remains visible as an affordance.
- Never auto-hide while a modal is open.

#### 2.7.2 Settings Panel (Side Drawer vs. Modal)
**Impact: M | Effort: M**

Replace the centred modal with a side drawer (slide in from the left or right). A drawer lets users adjust settings while keeping the fractal visible, enabling live iteration-count tweaking with immediate visual feedback.

#### 2.7.3 Zoom Level Indicator
**Impact: M | Effort: L**

A vertical or horizontal zoom bar (similar to a map scale bar) showing the current magnification factor and how deep the exploration is relative to the full set extent.

#### 2.7.4 Minimap / Overview Inset
**Impact: L | Effort: M**

A small inset in the corner showing the full Mandelbrot set with a box indicating the currently visible viewport. Helps users maintain orientation during deep zoom.

#### 2.7.5 Loading / Render Progress Indicator
**Impact: M | Effort: L**

A subtle spinner or progress bar shown while the GPU is computing a high-iteration render. Even a short delay during settings changes benefits from a visual cue.

#### 2.7.6 Responsive Layout
**Impact: H | Effort: M**

The current CSS uses fixed pixel padding for the toolbar. On small screens the buttons can overlap or be unreachable. The UI should be fully responsive:
- Stack toolbar buttons vertically on narrow viewports.
- Modals / drawers should be full-width on mobile.
- Touch targets must be at least 44×44 px (WCAG 2.5.5).

#### 2.7.7 Dark / Light Theme Toggle
**Impact: L | Effort: L**

The current UI chrome (modals, toolbar) uses a dark background fixed to the black canvas. Providing a light-mode option for the UI shell (not the canvas) improves readability in bright environments.

---

### 2.8 Onboarding & Discovery

#### 2.8.1 First-Run Tutorial / Guided Tour
**Impact: M | Effort: M**

A dismissable overlay or tooltip sequence that walks first-time users through:
1. How to pan and zoom.
2. What the colours represent.
3. Where to find famous locations.

Use `localStorage` to show the tour only once.

#### 2.8.2 "Interesting Places" Gallery
**Impact: H | Effort: M**

A curated gallery of famous Mandelbrot locations presented as clickable thumbnail cards. Clicking a card flies the viewer to that location with a smooth zoom animation.

Suggested locations to include:

| Name | Center (Re, Im) | Zoom |
|---|---|---|
| Full Set | −0.5, 0 | 3.0 |
| Seahorse Valley | −0.7436, 0.1319 | 0.004 |
| Elephant Valley | 0.3, 0 | 0.08 |
| Triple Spiral | −0.0886, 0.6543 | 0.005 |
| Mini Mandelbrot | −1.7497, 0 | 0.005 |
| Feigenbaum Point | −1.4012, 0 | 0.0001 |

#### 2.8.3 "Random Explore" Button
**Impact: M | Effort: L**

A button that picks a random interesting region from a curated seed list and flies the viewer there. Provides serendipitous discovery for casual users.

---

## 3. Prioritised Implementation Roadmap

### Phase 1 — Quick Wins (Low effort, High impact)

| # | Feature | Effort |
|---|---|---|
| 1 | Reset / Home button | L |
| 2 | Coordinate HUD | L |
| 3 | Deep-link / shareable URL | L |
| 4 | Keyboard shortcut reference | L |
| 5 | Render quality presets | L |
| 6 | Full keyboard navigation | M |
| 7 | Additional colour palettes | M |
| 8 | ARIA labels | L |

### Phase 2 — Core UX Improvements (Medium effort, High impact)

| # | Feature | Effort |
|---|---|---|
| 9 | Touch / pinch-to-zoom | M |
| 10 | Smooth animated zoom (cursor-anchored) | M |
| 11 | Smooth colouring (escape-time normalisation) | M |
| 12 | Screenshot / export | M |
| 13 | Bookmark system with defaults | M |
| 14 | "Interesting Places" gallery | M |
| 15 | Settings side drawer | M |
| 16 | Responsive layout | M |
| 17 | Auto-hiding toolbar | M |
| 18 | Progressive rendering | H |

### Phase 3 — Advanced Features (High effort, High impact)

| # | Feature | Effort |
|---|---|---|
| 19 | Julia set companion panel | H |
| 20 | Adaptive iteration ceiling | M |
| 21 | Colour cycling / animation | M |
| 22 | Navigation history (back/forward) | M |
| 23 | WebGL2 double-precision | H |
| 24 | Orbit trace overlay | H |
| 25 | Custom gradient editor | H |

---

## 4. UX Principles to Guide Design

1. **Canvas first.** The fractal is the product. Every UI element should minimise its footprint, use transparency, and get out of the way when not needed.

2. **Progressive disclosure.** Show only the most common controls (reset, settings, share) by default; reveal advanced controls (colour editor, orbit trace, adaptive quality) inside a dedicated panel.

3. **Immediate feedback.** Every interaction—pan, zoom, settings change—must produce visible change in under 16 ms (one frame). Defer expensive re-renders but never defer visual acknowledgement.

4. **Discoverability without clutter.** A keyboard shortcut cheat-sheet and first-run tour address discoverability without adding persistent UI chrome.

5. **Mobile parity.** Touch and mouse interactions must feel equally natural. Never require a hover to access essential functionality.

6. **Accessible by default.** High-contrast palettes, keyboard navigation, and ARIA labels should be built in from the start, not retrofitted.

7. **Shareability as a feature.** Every interesting view a user finds should be trivially shareable—one button press to copy a URL or download a screenshot.

---

## 5. References & Research Sources

- Nielsen Norman Group – *Touch Gesture Reference Guide*
- Google Material Design – *Navigation patterns*, *Motion principles*
- WCAG 2.2 – Success Criteria 1.4.3 (Contrast), 2.1.1 (Keyboard), 2.5.5 (Target Size)
- *The Fractal Geometry of Nature* – Benoît Mandelbrot (1982)
- Fractal explorer implementations surveyed:
  - [Mandelbrot Expedition](https://mandelbrot.site/) – deep-link support, progressive rendering
  - [Frakt.xyz](https://frakt.xyz/) – Julia companion view, smooth colouring
  - [Kalles Fraktaler](https://mathr.co.uk/kf/kf.html) – extreme deep zoom, double-precision
  - [XaoS](https://xaos-project.github.io/) – real-time smooth zoom, animation
