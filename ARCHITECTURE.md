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

## 4. Status Bar & `ResizeGripPainter`

Standard `QSizeGrip` painting can look blurry or inconsistent across different DPI scaling factors.
`ResizeGripPainter` installs as an event filter on `QSizeGrip`:
* Fills the grip background with `ThemeColors::chrome`.
* Uses logical DPI coordinates to paint a crisp 6-dot matrix (3 rows: 1 dot, 2 dots, 3 dots).
* Automatically mirrors coordinates when running in Right-to-Left (RTL) mode.

---

## 5. Theme Architecture

`ThemeManager` operates as a central singleton emitting `changed()`:
1. `ThemeColors` calculates semantic tokens:
   * `background`, `panel`, `chrome`, `text`, `muted`, `border`, `accent`, `selection`, `selectedText`.
2. Generates an exhaustive stylesheet that applies consistent padding, border radii, hover states, and focus outlines across all common Qt Widgets.
3. Automatically queries `QGuiApplication::styleHints()` to synchronize with Windows light and dark mode preferences.
4. Stores user overrides in `QSettings` under `ui/theme` and `ui/appearance`.
