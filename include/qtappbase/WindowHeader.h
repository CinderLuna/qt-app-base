// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#ifndef QTAPPBASE_WINDOWHEADER_H
#define QTAPPBASE_WINDOWHEADER_H

#include <QWidget>

class QHBoxLayout;
class QToolButton;

namespace qtappbase
{

/// WindowHeader is a custom title bar container widget.
/// On Windows with expanded title bar support, it hosts native-styled caption controls
/// (minimize, maximize/restore, close) and handles titlebar dragging, double-click to
/// maximize, and system menu presentation. Unused header space initiates window movement
/// while child widgets (menus, labels, buttons, search) retain their interactive input.
class WindowHeader : public QWidget
{
    Q_OBJECT
  public:
    explicit WindowHeader(QWidget *parent = nullptr);

    /// Checks if the operating system and Qt runtime support the custom expanded title bar.
    static bool supportsExpandedTitleBar();

    /// The horizontal layout for placing custom controls (icon, menu, title, search, actions).
    QHBoxLayout *row() const
    {
        return row_;
    }

    /// Updates caption control visibility and layout margins based on fullscreen/maximized state.
    void updateCaptionSpace();

    /// Installs the caption controls (minimize, maximize, close buttons).
    void installCaptionControls();

    /// Access the caption buttons.
    QToolButton *minimizeButton() const
    {
        return minimize_;
    }

    QToolButton *maximizeButton() const
    {
        return maximize_;
    }

    QToolButton *closeButton() const
    {
        return close_;
    }

    QWidget *captionControlsWidget() const
    {
        return controls_;
    }

    /// Toggles between maximized and restored window states.
    void requestMaximizeRestore();

    /// Shows the standard Windows system menu (Restore, Move, Size, Minimize, Maximize, Close).
    void showSystemMenu(bool atCursor = true);

  protected:
    bool event(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

  private:
    QHBoxLayout *row_ = nullptr;
    QWidget *controls_ = nullptr;
    QToolButton *minimize_ = nullptr;
    QToolButton *maximize_ = nullptr;
    QToolButton *close_ = nullptr;
};

} // namespace qtappbase

#endif // QTAPPBASE_WINDOWHEADER_H
