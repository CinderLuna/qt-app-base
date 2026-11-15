# Qt Widget Inspector

A self-contained Qt 6.2+ / C++17 developer component. Copy this entire directory into another Qt Widgets project, or build its demo on its own. It uses Qt Widgets only; there is no service, network connection, injection, or polling timer.

The inspector runs inside the application being inspected. The standalone demo demonstrates the tool; it does not attach to other running processes. Qt Quick/QML, native OS chrome, and platform-native file dialogs are outside its scope.

## Use

The flat bar floats inside the active application's window. Drag its six-dot handle to move it. Each application window or Qt modal dialog gets its own child bar when activated, avoiding a separate tool window being blocked behind a modal dialog.

- **Select / F8:** toggle inspection. Hover outlines a widget. Left click copies its report to the system clipboard. Selection remains enabled for further clicks. Inspection consumes mouse clicks, double clicks, wheel events, and context menus over application widgets, so they do not activate those controls. Toolbar buttons remain usable.
- **Parent:** select the selected widget's parent and copy its report. Repeat to reach containers, layouts' owner widgets, or the application window.
- **Copy:** copy the selected widget's current report again.
- **Escape:** stop inspecting and restore the widgets' previous mouse-tracking settings. Application shortcuts otherwise continue to work.
- **Hide:** hide the bar and stop inspecting. **Ctrl+Alt+F12** shows/hides it again. These shortcuts apply inside this application only.

The clipboard report contains class, objectName, full widget ancestry, a `findChild` hint for named widgets, parent/global logical geometry, minimum/maximum/preferred size, size policy, font, layout type/margins/spacing/direct item index, screen geometry, Qt version, and application name. List/tree view clicks also capture zero-based model row ancestry and column; tab-bar clicks capture the tab index. Item coordinates describe the moment of capture and can change after reordering.

Editor contents, input values, item display contents, tooltips, arbitrary dynamic properties, screenshots, project files, and source files are not collected. Object names and application names are included. Reports go only to your clipboard; paste one into your conversation together with the requested change. Unnamed widgets use class/sibling positions in the path. A named locator is a search hint: names may be duplicated, so verify its ancestry. There is no automatic C++ source-line mapping.

## Integrate in another project

Use the same Qt kit and compiler ABI as the host application:

```cmake
option(MYAPP_DEVTOOLS "Enable widget inspector" OFF)
if(MYAPP_DEVTOOLS)
  add_subdirectory(tools/qt-widget-inspector)
  target_link_libraries(my_app PRIVATE QtWidgetInspector::Inspector)
  target_compile_definitions(my_app PRIVATE MYAPP_DEVTOOLS)
endif()
```

After constructing `QApplication`, before showing/executing your first window:

```cpp
#ifdef MYAPP_DEVTOOLS
#include "Inspector.h"
#endif

// Inside main(), after QApplication app(argc, argv):
#ifdef MYAPP_DEVTOOLS
qtinspector::Inspector::install();
#endif
```

`install()` is idempotent and returns a controller owned by QApplication. Call it on the GUI thread. Widget references are guarded by QPointer, so closing a window or deleting a selected widget invalidates that selection. The tool does not modify story records, object names, application stylesheets, or files. It adds its own overlay children and temporarily enables mouse tracking only during selection.

## Build independently

```powershell
cmake -S tools/qt-widget-inspector -B build/inspector-standalone -G Ninja `
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=F:/Apps/Qt/6.9.3/mingw_64 `
  -DCMAKE_CXX_COMPILER=F:/Apps/Qt/Tools/mingw1310_64/bin/g++.exe
cmake --build build/inspector-standalone
ctest --test-dir build/inspector-standalone --output-on-failure
```

Put the matching Qt/compiler binaries on PATH to run the demo, or deploy it with `windeployqt`. The usual Qt platform plugin must be available.

## Qt App Base integration

In `qt-app-base`, the Widget Inspector is built into `qtappbase` in Debug mode (controlled via `QTAPPBASE_DEBUG`) and toggled via the View menu (`Ctrl+Alt+F12` or `F8`). It can also be built as a standalone demonstration tool with `cmake -B build -DBUILD_DEVTOOLS=ON`.

## Manual feedback

1. Launch the demo application: expect a small flat bar near the bottom of the window. Drag the six dots; it must stay inside the window.
2. Enable Select (or press F8) and hover/click a widget. Expect an outline and a clipboard report, without activating the control.
3. Press Escape to stop selection.
4. Inspect widget rows or components. Expect the viewport's widget path and model row path. Press Parent to identify the owning container.
5. Hide and reopen the bar with Ctrl+Alt+F12. Inspect menus, dialogs, fullscreen, and display scaling. Native title-bar controls and OS file pickers are not QWidget targets.

Automated tests cover report contents, input exclusion, click suppression, clipboard copying, parent navigation, mouse-tracking restoration, destroyed selections, modal dialogs, visibility shortcuts, and model row identification.
