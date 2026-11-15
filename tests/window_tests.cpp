// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#include "qtappbase/BaseStatusBar.h"
#include "qtappbase/BaseWindow.h"
#include "qtappbase/Inspector.h"
#include "qtappbase/SearchLineEdit.h"
#include "qtappbase/Theme.h"
#include "qtappbase/WindowHeader.h"
#include "qtappbase/WindowTitleLabel.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>
#include <QToolButton>

using namespace qtappbase;

class WindowTests : public QObject
{
    Q_OBJECT
  private slots:
    void testWindowTitleLabel();
    void testWindowHeader();
    void testBaseStatusBar();
    void testThemeManager();
    void testBaseWindow();
    void testSearchLineEdit();
};

void WindowTests::testWindowTitleLabel()
{
    WindowTitleLabel label;
    label.setText("Test Project Title");
    QCOMPARE(label.text(), QString("Test Project Title"));
    QVERIFY2(!label.displayText().isEmpty(), "Display text should not be empty");
    QVERIFY2(label.testAttribute(Qt::WA_TransparentForMouseEvents),
             "Label must be transparent for mouse events");
}

void WindowTests::testWindowHeader()
{
    WindowHeader header;
    QVERIFY2(header.row() != nullptr, "Header row layout must not be null");
    QCOMPARE(header.objectName(), QString("applicationHeader"));
    QVERIFY2(header.minimumHeight() >= 40, "Header minimum height should be at least 40px");

    header.installCaptionControls();
    if (WindowHeader::supportsExpandedTitleBar())
    {
        QVERIFY2(header.minimizeButton() != nullptr,
                 "Minimize button must be created on supported platforms");
        QVERIFY2(header.maximizeButton() != nullptr,
                 "Maximize button must be created on supported platforms");
        QVERIFY2(header.closeButton() != nullptr, "Close button must be created on supported platforms");
        QCOMPARE(header.minimizeButton()->objectName(), QString("captionMinimize"));
        QCOMPARE(header.maximizeButton()->objectName(), QString("captionMaximize"));
        QCOMPARE(header.closeButton()->objectName(), QString("captionClose"));

#if (defined(Q_OS_LINUX) || defined(Q_OS_UNIX)) && !defined(Q_OS_DARWIN)
        QCOMPARE(header.minimizeButton()->size(), QSize(26, 26));
        QCOMPARE(header.maximizeButton()->size(), QSize(26, 26));
        QCOMPARE(header.closeButton()->size(), QSize(26, 26));
        QCOMPARE(header.captionControlsWidget()->layout()->spacing(), 8);
        QCOMPARE(header.row()->contentsMargins().right(), 12);
#endif
    }
}

void WindowTests::testBaseStatusBar()
{
    BaseStatusBar statusBar;
    QCOMPARE(statusBar.objectName(), QString("baseStatusBar"));
    QVERIFY2(statusBar.statusLabel() != nullptr, "Status label must not be null");
    QVERIFY2(statusBar.infoLabel() != nullptr, "Info label must not be null");

    statusBar.setStatusText("Operational");
    QCOMPARE(statusBar.statusText(), QString("Operational"));

    statusBar.setInfoText("100 items");
    QCOMPARE(statusBar.infoText(), QString("100 items"));

    auto *actionBtn = new QPushButton("Save");
    statusBar.setActionButton(actionBtn);
    QCOMPARE(statusBar.actionButton(), actionBtn);
}

void WindowTests::testThemeManager()
{
    auto &manager = ThemeManager::instance();
    QVERIFY2(!availableThemes().isEmpty(), "Available themes list must not be empty");

    QSignalSpy spy(&manager, &ThemeManager::changed);
    manager.setPreferences(ColorTheme::Forest, Appearance::Dark);
    QCOMPARE(manager.theme(), ColorTheme::Forest);
    QCOMPARE(manager.appearance(), Appearance::Dark);
    QVERIFY2(manager.isDark(), "Theme manager must report dark mode");
    QVERIFY2(spy.count() >= 1, "Theme manager should emit changed signal on preference update");

    const auto colors = manager.colors();
    QVERIFY2(colors.background.isValid(), "Background color must be valid");
    QVERIFY2(colors.panel.isValid(), "Panel color must be valid");
    QVERIFY2(colors.chrome.isValid(), "Chrome color must be valid");
    QVERIFY2(colors.text.isValid(), "Text color must be valid");
    QVERIFY2(colors.accent.isValid(), "Accent color must be valid");

    const auto css = colors.stylesheet();
    QVERIFY2(!css.isEmpty(), "Generated stylesheet must not be empty");
    QVERIFY2(css.contains("QWidget#applicationHeader"), "Stylesheet must style applicationHeader");
    QVERIFY2(css.contains("QStatusBar"), "Stylesheet must style QStatusBar");
#if (defined(Q_OS_LINUX) || defined(Q_OS_UNIX)) && !defined(Q_OS_DARWIN)
    QVERIFY2(css.contains("#e01b24"), "Stylesheet must include GNOME close button hover color");
    QVERIFY2(css.contains("border-radius:13px"), "Stylesheet must style GNOME circular caption buttons");
    QVERIFY2(css.contains("border-top-left-radius:12px"), "Stylesheet must style top rounded corners");
    QVERIFY2(css.contains("border-bottom-left-radius:12px"), "Stylesheet must style bottom rounded corners");
    QVERIFY2(css.contains("QMainWindow[windowRestored=\"false\"]"), "Stylesheet must square corners when maximized");
#endif
}

void WindowTests::testBaseWindow()
{
    BaseWindow window;
    window.setWindowTitle("Test Application");
    QCOMPARE(window.windowTitle(), QString("Test Application"));
    QCOMPARE(window.titleLabel()->text(), QString("Test Application"));
    QVERIFY2(window.header() != nullptr, "Window header must not be null");
    QVERIFY2(window.headerMenuBar() != nullptr, "Window menu bar must not be null");
    QVERIFY2(window.appStatusBar() != nullptr, "Window status bar must not be null");
    QVERIFY2(window.brandIconLabel() != nullptr, "Window brand icon label must not be null");
    QCOMPARE(window.isDevtoolsAvailable(), qtinspector::Inspector::isAvailable());
    QVERIFY2(BaseWindow::autoInstallDevtools(), "Devtools auto-install should default to true");

#if (defined(Q_OS_LINUX) || defined(Q_OS_UNIX)) && !defined(Q_OS_DARWIN)
    QVERIFY2(window.windowFlags().testFlag(Qt::FramelessWindowHint),
             "BaseWindow should set Qt::FramelessWindowHint on Linux");
    QVERIFY2(window.testAttribute(Qt::WA_TranslucentBackground),
             "BaseWindow should set Qt::WA_TranslucentBackground on Linux");
    QCOMPARE(window.property("windowRestored").toBool(), true);
#endif

    auto *sampleWidget = new QLabel("Test Content", &window);
    window.setCentralWidget(sampleWidget);
    QCOMPARE(window.centralWidget(), sampleWidget);

    window.showMaximized();
    QVERIFY(window.isMaximized());
#if (defined(Q_OS_LINUX) || defined(Q_OS_UNIX)) && !defined(Q_OS_DARWIN)
    QCOMPARE(window.property("windowRestored").toBool(), false);
#endif
    window.toggleFullScreen();
    QVERIFY(window.isFullScreen());
#if (defined(Q_OS_LINUX) || defined(Q_OS_UNIX)) && !defined(Q_OS_DARWIN)
    QCOMPARE(window.property("windowRestored").toBool(), false);
#endif
    window.toggleFullScreen();
    QVERIFY(!window.isFullScreen());
    QVERIFY2(window.isMaximized(), "Exiting fullscreen should restore maximized state");
#if (defined(Q_OS_LINUX) || defined(Q_OS_UNIX)) && !defined(Q_OS_DARWIN)
    QCOMPARE(window.property("windowRestored").toBool(), false);
#endif

    window.showNormal();
#if (defined(Q_OS_LINUX) || defined(Q_OS_UNIX)) && !defined(Q_OS_DARWIN)
    QCOMPARE(window.property("windowRestored").toBool(), true);
#endif
}

void WindowTests::testSearchLineEdit()
{
    SearchLineEdit search("Search…");
    QCOMPARE(search.placeholderText(), QString("Search…"));
    QVERIFY2(search.shortcutLabel() != nullptr, "Shortcut label must not be null");
    QCOMPARE(search.shortcutText(), QString("Ctrl+K"));
    QVERIFY2(!search.shortcutLabel()->isHidden(), "Shortcut label should be visible when input is empty");

    search.setText("Hello");
    QVERIFY2(search.shortcutLabel()->isHidden(), "Shortcut label should be hidden when text is present");

    search.clear();
    QVERIFY2(!search.shortcutLabel()->isHidden(), "Shortcut label should be visible when input is cleared");
}

QTEST_MAIN(WindowTests)
#include "window_tests.moc"
