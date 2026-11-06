// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#ifndef QTAPPBASE_BASEWINDOW_H
#define QTAPPBASE_BASEWINDOW_H

#include "qtappbase/BaseStatusBar.h"
#include "qtappbase/Theme.h"
#include "qtappbase/WindowHeader.h"
#include "qtappbase/WindowTitleLabel.h"
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>

class QFrame;
class QAction;

namespace qtappbase
{

/// BaseWindow is the foundation window class for organization desktop applications.
/// It integrates:
/// - Windows 11 Fluent expanded title bar with Snap Layouts (HTMAXBUTTON)
/// - Native resizing borders, DWM shadows, and Aero Snap via WM_NCHITTEST
/// - Integrated MenuBar inside the custom title bar
/// - Elided title label and brand icon slot
/// - Customizable title bar center/right action slots (e.g. search, action buttons)
/// - Modern BaseStatusBar with custom 6-dot resize grip, status text, info metrics, and zoom controls
/// - Dynamic multi-palette theming (Slate, Olive, Forest, Parchment, Aubergine, Rosewood)
///   with Light / Dark / Follow System appearance and automatic OS theme detection
/// - Fullscreen toggle with F11 / Escape and smooth caption clearance
class BaseWindow : public QMainWindow
{
    Q_OBJECT
  public:
    explicit BaseWindow(QWidget *parent = nullptr);
    ~BaseWindow() override;

    /// Title bar components
    WindowHeader *header() const
    {
        return header_;
    }

    WindowTitleLabel *titleLabel() const
    {
        return titleLabel_;
    }

    QLabel *brandIconLabel() const
    {
        return brandIconLabel_;
    }

    QMenuBar *headerMenuBar() const
    {
        return appMenu_;
    }

    /// Status bar component
    BaseStatusBar *appStatusBar() const
    {
        return statusBar_;
    }

    /// Custom slots in header for application-specific controls (e.g., search or action buttons)
    QWidget *headerCenterArea() const
    {
        return headerCenterArea_;
    }

    QWidget *headerRightArea() const
    {
        return headerRightArea_;
    }

    void setHeaderSearchWidget(QWidget *searchWidget);

    /// Brand icon helpers
    void setBrandIcon(const QIcon &icon, const QSize &size = QSize(24, 24));
    void setBrandIconVisible(bool visible);

    /// Overrides window title to update both QMainWindow and custom title label
    void setWindowTitle(const QString &title);

    /// Fullscreen management
    void toggleFullScreen();
    bool isExtendedHeader() const
    {
        return extendedHeader_;
    }

    QAction *fullScreenAction() const
    {
        return fullScreenAction_;
    }

  protected:
    void showEvent(QShowEvent *event) override;
    void changeEvent(QEvent *event) override;
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

    /// Override in derived classes to perform custom reactions to theme changes
    virtual void onThemeChanged(const ThemeColors &colors);

  private:
    void setupBaseUi();
    void syncWindowsChrome();
    void applyCurrentTheme();

    bool extendedHeader_ = false;
    WindowHeader *header_ = nullptr;
    QLabel *brandIconLabel_ = nullptr;
    QMenuBar *appMenu_ = nullptr;
    QFrame *menuSeparator_ = nullptr;
    WindowTitleLabel *titleLabel_ = nullptr;
    QWidget *headerCenterArea_ = nullptr;
    QWidget *headerRightArea_ = nullptr;
    QWidget *searchWidget_ = nullptr;

    BaseStatusBar *statusBar_ = nullptr;
    QAction *fullScreenAction_ = nullptr;
};

} // namespace qtappbase

#endif // QTAPPBASE_BASEWINDOW_H
