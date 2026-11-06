# Qt App Base

An open-source C++17 / Qt 6 desktop application template by **Cinderluna**. It provides a Windows 11 custom title bar with Snap Layouts, an integrated menu bar, a custom status bar, and dynamic theming.

The central client area is clean and empty, ready for application-specific widgets.

---

## Quick Start

### Build and Run

Run these commands in PowerShell (adjust paths for your local Qt and compiler installation):

```powershell
$qtPrefix = if ($env:QT_DIR) { $env:QT_DIR } else { 'F:\Apps\Qt\6.9.3\mingw_64' }
$compilerBin = if ($env:MINGW_BIN) { $env:MINGW_BIN } else { 'F:\Apps\Qt\Tools\mingw1310_64\bin' }
$env:PATH = "$compilerBin;$qtPrefix\bin;$env:PATH"
$env:QT_QPA_PLATFORM_PLUGIN_PATH = "$qtPrefix\plugins\platforms"

# Configure, build, and test
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release "-DCMAKE_PREFIX_PATH=$qtPrefix" "-DCMAKE_CXX_COMPILER=$compilerBin/g++.exe"
cmake --build build -j 4
ctest --test-dir build --output-on-failure

# Launch template
.\build\qt_app_base_demo.exe
```

You can also run `run.bat` to launch the application.

---

## Core Features

* **Custom Title Bar**: Windows 11 Snap Layouts on maximize button (`HTMAXBUTTON`), DWM drop shadows, and Aero Snap. See [ARCHITECTURE.md](ARCHITECTURE.md).
* **Integrated Menu Bar**: `QMenuBar` embedded directly inside the custom title bar.
* **Search Input with Shortcut**: Right-aligned `Ctrl+K` shortcut badge using [`SearchLineEdit`](include/qtappbase/SearchLineEdit.h).
* **Elided Window Title**: Right-side text elision with [`WindowTitleLabel`](include/qtappbase/WindowTitleLabel.h).
* **Custom Status Bar**: 6-dot DPI-aware resize grip with status and metric labels.
* **Theme Engine**: 6 color themes (`Slate`, `Olive`, `Forest`, `Parchment`, `Aubergine`, `Rosewood`) with system dark mode tracking.

---

## Usage Example

Inherit from [`BaseWindow`](include/qtappbase/BaseWindow.h):

```cpp
#include "qtappbase/BaseWindow.h"
#include "qtappbase/SearchLineEdit.h"
#include <QLabel>

class MainWindow : public qtappbase::BaseWindow
{
public:
    explicit MainWindow(QWidget *parent = nullptr) : BaseWindow(parent)
    {
        setWindowTitle("Cinderluna Application");

        // 1. Title bar menus
        auto *fileMenu = headerMenuBar()->addMenu("&File");
        fileMenu->addAction("&Exit", QKeySequence::Quit, this, &QWidget::close);

        // 2. Theme menus
        qtappbase::ThemeManager::populateThemeMenu(headerMenuBar()->addMenu("&Theme"));
        qtappbase::ThemeManager::populateAppearanceMenu(headerMenuBar()->addMenu("&Appearance"));

        // 3. Search input
        setHeaderSearchWidget(new qtappbase::SearchLineEdit("Search…", this));

        // 4. Client workspace (add your widgets here)
        setCentralWidget(new QLabel("Workspace Area", this));

        // 5. Status bar
        appStatusBar()->setStatusText("Ready");
    }
};
```

---

## Theme & Branding Notices

* **High Contrast Incompatibility**: The theme engine does not support OS High Contrast accessibility themes. Applications requiring High Contrast mode must provide custom stylesheets.
* **Public Usage & Branding**: Do not use the default Cinderluna brand assets (`Cb` logo and mark) or default color themes for public applications. Customize the branding and color palettes to avoid confusion with official Cinderluna products.

---

## Licenses & Legal Considerations

Both [Folimere](https://github.com/cinderluna/folimere) and this template are open-source projects created by **Cinderluna**.

| Component | License | Notes & Compliance |
| :--- | :--- | :--- |
| **qt-app-base** | **GPL-3.0-or-later** | Open-source project template (see [LICENSE](LICENSE)). Downstream applications derived from this template inherit the GPL-3.0-or-later copyleft terms. |
| **Folimere Code** | **GPL-3.0-or-later** | Title bar and status bar code were extracted from Folimere, an open-source project by Cinderluna. Both share the GPL-3.0-or-later license. |
| **Qt 6 Framework** | **LGPLv3** / **GPLv3** / Commercial | Qt Widgets, Gui, Core, and Svg are licensed under LGPLv3/GPLv3. Standard dynamic linking satisfies LGPLv3 user-relinkability requirements and is 100% license-compatible with GPL-3.0-or-later. |
| **Fluent Icons** | **MIT License** | Permissive open-source license. Compatible with GPL-3.0-or-later. Notice retained in [assets/icons/fluent/LICENSE](assets/icons/fluent/LICENSE). |
