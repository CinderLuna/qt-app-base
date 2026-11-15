# Architecture & Native Window Manager Integration

This document outlines the low-level Win32 integration, styling mechanics, and Qt event processing utilized by `qt-app-base`.

---

## 1. Win32 Frameless Architecture & Windows 11 Snap Layouts

Creating custom title bars in Qt often leads to broken OS integration if implemented naively with `Qt::FramelessWindowHint` (`WS_POPUP`). A borderless popup window loses:
1. **Windows 11 Snap Layouts flyout** when hovering the maximize button.
2. **DWM compositor drop shadows** and rounded corners.
3. **Native resize handles and Aero Snap animations** when dragged to screen edges.
4. **Standard system menus** (`Alt + Space` and title bar right-click).

### The Solution: Native Chrome Integration

`BaseWindow` keeps the native window frame styles while instructing Qt to expand its drawing surface into the non-client title bar:

```cpp
auto flags = windowFlags() | Qt::ExpandedClientAreaHint | Qt::NoTitleBarBackgroundHint |
             Qt::CustomizeWindowHint | Qt::WindowSystemMenuHint;
flags &= ~(Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
flags.setFlag(Qt::WindowTitleHint, false);
setWindowFlags(flags);
setAttribute(Qt::WA_ContentsMarginsRespectsSafeArea, false);
```

In `BaseWindow::syncWindowsChrome()`:
```cpp
const LONG_PTR oldStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);
const LONG_PTR style =
    (oldStyle & ~WS_POPUP) | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME;
if (style != oldStyle) {
    SetWindowLongPtrW(hwnd, GWL_STYLE, style);
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}
```

This retains full DWM shadow composition, native animations, and OS window manager recognition.

---

## 2. Non-Client Hit Testing (`WM_NCHITTEST`)

In `BaseWindowChrome.cpp`, `BaseWindow::nativeEvent()` intercepts `WM_NCHITTEST` and dynamically segments the window into:

1. **Resize Borders (`HTTOP`, `HTLEFT`, `HTBOTTOMRIGHT`, etc.)**:
   Calculated dynamically using per-monitor DPI:
   ```cpp
   const UINT dpi = GetDpiForWindow(msg->hwnd);
   const int border = IsZoomed(msg->hwnd) ? 0
       : GetSystemMetricsForDpi(SM_CXSIZEFRAME, dpi) + GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
   ```

2. **Windows 11 Snap Layouts Trigger (`HTMAXBUTTON`)**:
   When the cursor is over the maximize button:
   ```cpp
   if (button == maximize)
       *result = HTMAXBUTTON;
   ```
   To ensure the Windows 11 Snap flyout displays reliably:
   * `WM_NCMOUSEMOVE` sets `Qt::WA_UnderMouse` and calls `TrackMouseEvent(&tracking)`.
   * `WM_NCLBUTTONDOWN` and `WM_NCLBUTTONUP` track mouse clicks on the non-client button.

3. **Draggable Title Bar Space (`HTCAPTION`)**:
   Empty spaces within `WindowHeader` return `HTCAPTION`, allowing smooth compositor dragging, Aero Snapping, and double-clicking to maximize/restore.

4. **Interactive Controls (`HTCLIENT`)**:
   Child widgets inside the title bar (menus, search edit, buttons) return `HTCLIENT` so they receive normal mouse clicks, focus, and keyboard events without dragging the window.

---

## 3. Qt 6.9+ Blank Caption Animation Suppression

In Qt 6.9, Qt's internal Windows platform plugin spawns an internal helper window named `_q_titlebar` when `Qt::ExpandedClientAreaHint` is used. This blank helper window attempts to run a 200ms timer (timer ID 1) that can cause redundant repaints.

`BaseWindowChrome.cpp` discovers this internal helper window via `EnumWindows`, subclasses it temporarily to cancel Timer 1, and restores original window procedures on teardown.

---

## 4. Linux Custom Title Bar & Native Compositor Integration

On Linux (both X11 and Wayland environments):
1. **Frameless Window Setup**:
   `BaseWindow` applies `Qt::FramelessWindowHint` to suppress native server-side decorations (SSD).
2. **Interactive Window Movement**:
   Unused space in `WindowHeader` initiates native compositor movement via `window()->windowHandle()->startSystemMove()`, enabling full compositor gestures (tiling, edge-snapping, multi-monitor dragging).
3. **Interactive Window Resizing**:
   - The top edge (top 6px of `WindowHeader`) triggers native vertical resizing through `window()->windowHandle()->startSystemResize(Qt::TopEdge)` with `Qt::SizeVerCursor` feedback.
   - All window edges and corners support smooth resizing via event-filter hit testing in `BaseWindow`.
   - The bottom-right corner provides standard interactive resizing via `BaseStatusBar`'s custom-painted `QSizeGrip`.
4. **Native GNOME (Adwaita / Libadwaita) Window Controls**:
   On Linux desktops, window controls automatically style to match native GNOME Adwaita specifications:
   - **Circular Buttons**: 26x26px buttons with `border-radius: 13px` vertically centered in the 48px header.
   - **GNOME Spacing & Margin**: 8px inter-button spacing with 12px right-edge padding.
   - **Adwaita Symbolic Glyphs**: Clean vector icons rendered with rounded joins and caps:
     - Close: 2px rounded-cap cross.
     - Maximize: 9x9 box with 1.75px stroke.
     - Restore: Overlapping boxes matching GNOME symbolic unmaximize.
     - Minimize: 2px thick horizontal bar positioned slightly below center.
   - **Native Hover & Pressed States**:
     - Minimize & Maximize: subtle circular background highlight (`$selection`).
     - Close: GNOME Red (`#e01b24`) on hover and dark red (`#a51d2d`) on press with crisp white cross glyph.
   - **Embedded SVG Assets & Vector Rendering**: Canonical GNOME symbolic assets are packaged under `assets/icons/gnome/` and embedded at `:/icons/gnome/`. `CaptionButton` renders these glyphs procedurally via antialiased `QPainter` vector paths, guaranteeing crisp rendering at fractional DPI scaling factors (1.25x, 1.5x, 2x) with zero DOM parsing overhead and instant stroke recoloring on theme switch or hover.
5. **Native Window Operations Menu**:
   Right-clicking empty header space displays a standard desktop window menu offering Restore, Move, Minimize, Maximize, and Close actions.
6. **Rounded Window Corners (Approach A - Translucent ARGB Surface)**:
   - **Translucent Top-Level ARGB Surface**:
     `BaseWindow` applies `setAttribute(Qt::WA_TranslucentBackground, true)` on Linux. The top-level window surface remains fully transparent outside child surfaces, avoiding square corner artifacts or black rectangular backing boxes.
   - **Coordinated Subsurface Framing**:
     Central contents are hosted inside `centralContainer_` (an internal framing widget accessed via `BaseWindow::setCentralWidget` and `BaseWindow::centralWidget`).
     The window structure forms a continuous 1px outline:
     - `WindowHeader`: `border-top-left-radius: 8px; border-top-right-radius: 8px;` with top/left/right 1px borders.
     - `centralContainer_`: left/right 1px borders (and bottom border when status bar is hidden).
     - `BaseStatusBar`: `border-bottom-left-radius: 8px; border-bottom-right-radius: 8px;` with bottom/left/right 1px borders.
   - **Resize Grip Harmony**:
     `ResizeGripPainter` paints an antialiased 8px circular path on the outer edge in restored state, perfectly blending the bottom-right corner grip into the rounded status bar.
   - **Dynamic Maximized/Fullscreen Squaring**:
     A dynamic QObject property `windowRestored` toggles between `true` (floating/restored) and `false` (maximized/fullscreen). The theme stylesheet dynamically collapses all corner radii to `0px` when `QMainWindow[windowRestored="false"]`, guaranteeing pixel-perfect alignment against screen edges.
7. **Native Modal Dialogs & GNOME Compositor Integration**:
   - On GNOME desktop environments, Mutter enforces `org.gnome.mutter attach-modal-dialogs: true` by default. Under this native GNOME behavior, standard modal dialogs parented to the window (e.g. `QMessageBox::about(this, ...)`) appear as attached sheets pinned to the parent titlebar, and moving the dialog moves the parent window with it.
   - End-users who prefer detached, free-floating modal dialogs across their GNOME desktop can toggle this behavior via GNOME Tweaks (*Windows &rarr; Attach Modal Dialogs*) or terminal:
     ```bash
     gsettings set org.gnome.mutter attach-modal-dialogs false
     ```

---

## 5. Status Bar & `ResizeGripPainter`

Standard `QSizeGrip` painting can look blurry or inconsistent across different DPI scaling factors.
`ResizeGripPainter` installs as an event filter on `QSizeGrip`:
* Fills the grip background with `ThemeColors::chrome`.
* Uses logical DPI coordinates to paint a crisp 6-dot matrix (3 rows: 1 dot, 2 dots, 3 dots).
* Automatically mirrors coordinates when running in Right-to-Left (RTL) mode.

---

## 6. Theme Architecture

`ThemeManager` operates as a central singleton emitting `changed()`:
1. `ThemeColors` calculates semantic tokens:
   * `background`, `panel`, `chrome`, `text`, `muted`, `border`, `accent`, `selection`, `selectedText`.
2. Generates an exhaustive stylesheet that applies consistent padding, border radii, hover states, and focus outlines across all common Qt Widgets.
3. Automatically queries `QGuiApplication::styleHints()` to synchronize with Windows light and dark mode preferences.
4. Stores user overrides in `QSettings` under `ui/theme` and `ui/appearance`.

---

## 7. Developer Tools (Qt Widget Inspector) & Debug Mode Gating

The Qt Widget Inspector provides in-process diagnostic tools to inspect widget ancestry, object names, layout constraints, and geometric metrics.

### Strict Debug-Only Contract
Devtools must never leak into production or release builds. This is enforced along two orthogonal lines of defense:

1. **Compile-time generator expression & preprocessor guards:**
   - In CMake: `target_compile_definitions(qtappbase PUBLIC "$<$<CONFIG:Debug>:QTAPPBASE_DEBUG>")`.
   - In C++ (`Inspector.h` & `Inspector.cpp`): Implementation is guarded by `#if !defined(NDEBUG) || defined(QT_DEBUG) || defined(QTAPPBASE_DEBUG)`.
   - In Release builds (`-DCMAKE_BUILD_TYPE=Release`), `NDEBUG` is defined and `QTAPPBASE_DEBUG` is absent. `Inspector::isAvailable()` evaluates to `false` at compile-time. All public entry points (`install()`, `instance()`, `report()`) compile to inert no-op stubs returning `nullptr` or empty strings.

2. **UI & Event Hook Inaccessibility:**
   - In Release builds, no `QEvent` filter is installed on `QApplication`, no mouse tracking is altered, no keyboard shortcuts (`F8`, `Ctrl+Alt+F12`, `Escape`) are registered, and the `View -> Widget Inspector` menu item is omitted.
   - In Debug builds, `BaseWindow::showEvent()` and `main()` ensure the inspector is installed on `QApplication`. Top-level windows and modal dialogs receive the floating inspector toolbar, and picking mode allows non-destructive inspection of live widgets.

---

## 8. Compiler Strictness & Conformance

The template enforces strict compiler diagnostics and modern Qt API standards via CMake option `QT_APP_BASE_STRICT_MODE` (enabled by default):

### Diagnostics Flags:
* **GCC / Clang**:
  `-Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion -Wold-style-cast -Wnon-virtual-dtor -Woverloaded-virtual`
* **MSVC**:
  `/W4 /WX /permissive- /w14242 /w14254 /w14263 /w14265 /w14287 /w14296 /w14311 /w14545 /w14546 /w14547 /w14549 /w14555 /w14640`

### Qt Strictness Definitions:
* `QT_NO_KEYWORDS`: Enforces modern Qt macro usage (`Q_SLOTS`, `Q_SIGNALS`, `Q_EMIT`), preventing namespace collisions with third-party libraries.
* `QT_DISABLE_DEPRECATED_UP_TO=0x060200`: Disables Qt APIs deprecated up to Qt 6.2, enforcing forward-compatible patterns across all targets.
