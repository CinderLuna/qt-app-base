// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#include "qtappbase/WindowHeader.h"
#include <QContextMenuEvent>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QTimer>
#include <QToolButton>
#include <QWindow>
#include <QtMath>

#if defined(Q_OS_WIN)
#include <qt_windows.h>
#endif

namespace qtappbase
{

bool WindowHeader::supportsExpandedTitleBar()
{
#if defined(Q_OS_WIN) && QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    return QGuiApplication::platformName() == "windows";
#else
    return false;
#endif
}

WindowHeader::WindowHeader(QWidget *parent) : QWidget(parent), row_(new QHBoxLayout(this))
{
    setObjectName("applicationHeader");
    setAttribute(Qt::WA_StyledBackground, true);
    setAccessibleName("Application header");
    row_->setContentsMargins(16, 4, 16, 4);
    row_->setSpacing(0);
    setMinimumHeight(48);
}

void WindowHeader::installCaptionControls()
{
    if (controls_ || !supportsExpandedTitleBar())
        return;

    controls_ = new QWidget(this);
    controls_->setObjectName("captionControls");
    controls_->setStyleSheet("QWidget#captionControls { background:transparent; }");

    auto *layout = new QHBoxLayout(controls_);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    const auto families = QFontDatabase::families();
    QFont glyphs(families.contains("Segoe Fluent Icons") ? "Segoe Fluent Icons" : "Segoe MDL2 Assets");
    glyphs.setPixelSize(10);

    const auto add = [&](const char *name, const char *label, ushort glyph) -> QToolButton *
    {
        auto *button = new QToolButton(controls_);
        button->setObjectName(name);
        button->setAccessibleName(label);
        button->setToolTip(label);
        button->setText(QString(QChar(glyph)));
        button->setFont(glyphs);
        button->setFixedWidth(46);
        button->setMinimumHeight(32);
        button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        button->setStyleSheet(QString("QToolButton { padding:0; border:0; border-radius:0; "
                                      "font-family:'%1'; font-size:10px; }")
                                  .arg(glyphs.family()));
        layout->addWidget(button);
        return button;
    };

    minimize_ = add("captionMinimize", "Minimize", 0xE921);
    maximize_ = add("captionMaximize", "Maximize", 0xE922);
    close_ = add("captionClose", "Close", 0xE8BB);
    close_->setStyleSheet(close_->styleSheet() + "QToolButton:hover { background:#C42B1C; color:white; }");

#if defined(Q_OS_WIN)
    connect(minimize_, &QToolButton::clicked, this,
            [this]
            {
                if (window())
                    PostMessageW(reinterpret_cast<HWND>(window()->winId()), WM_SYSCOMMAND, SC_MINIMIZE, 0);
            });
    connect(close_, &QToolButton::clicked, this,
            [this]
            {
                if (window())
                    PostMessageW(reinterpret_cast<HWND>(window()->winId()), WM_SYSCOMMAND, SC_CLOSE, 0);
            });
#endif
    connect(maximize_, &QToolButton::clicked, this, &WindowHeader::requestMaximizeRestore);

    row_->addSpacing(8);
    row_->addWidget(controls_);
    row_->setContentsMargins(16, 0, 0, 0);
}

void WindowHeader::requestMaximizeRestore()
{
    if (!window())
        return;

#if defined(Q_OS_WIN)
    if (supportsExpandedTitleBar() && !window()->isFullScreen())
    {
        const auto hwnd = reinterpret_cast<HWND>(window()->winId());
        PostMessageW(hwnd, WM_SYSCOMMAND, IsZoomed(hwnd) ? SC_RESTORE : SC_MAXIMIZE, 0);
        return;
    }
#endif
    window()->isMaximized() ? window()->showNormal() : window()->showMaximized();
}

void WindowHeader::showSystemMenu(bool atCursor)
{
    if (!window())
        return;

#if defined(Q_OS_WIN)
    if (!supportsExpandedTitleBar() || window()->isFullScreen())
        return;

    const auto hwnd = reinterpret_cast<HWND>(window()->winId());
    const auto menu = GetSystemMenu(hwnd, FALSE);
    if (!menu)
        return;

    const bool maximized = IsZoomed(hwnd);
    const auto enable = [menu](UINT command, bool enabled)
    { EnableMenuItem(menu, command, MF_BYCOMMAND | (enabled ? MF_ENABLED : MF_GRAYED)); };

    enable(SC_RESTORE, maximized || IsIconic(hwnd));
    enable(SC_MOVE, !maximized);
    enable(SC_SIZE, !maximized);
    enable(SC_MINIMIZE, !IsIconic(hwnd));
    enable(SC_MAXIMIZE, !maximized);
    SetMenuDefaultItem(menu, SC_CLOSE, FALSE);

    POINT position{};
    if (atCursor)
    {
        GetCursorPos(&position);
    }
    else
    {
        position.y = qRound(height() * window()->devicePixelRatioF());
        ClientToScreen(hwnd, &position);
    }

    const UINT alignment = GetSystemMetrics(SM_MENUDROPALIGNMENT) ? TPM_RIGHTALIGN : TPM_LEFTALIGN;
    const UINT command = TrackPopupMenuEx(menu, alignment | TPM_RIGHTBUTTON | TPM_RETURNCMD, position.x,
                                          position.y, hwnd, nullptr);
    if (command)
        PostMessageW(hwnd, WM_SYSCOMMAND, command, 0);
#else
    Q_UNUSED(atCursor);
#endif
}

void WindowHeader::contextMenuEvent(QContextMenuEvent *event)
{
    if (window() && supportsExpandedTitleBar() && !window()->isFullScreen())
    {
        showSystemMenu(event->reason() == QContextMenuEvent::Mouse);
        event->accept();
    }
    else
    {
        QWidget::contextMenuEvent(event);
    }
}

void WindowHeader::updateCaptionSpace()
{
    if (!window())
        return;

    int reserved = 0;
#if defined(Q_OS_WIN) && QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    auto *handle = window()->windowHandle();
    if (supportsExpandedTitleBar() && handle && handle->flags().testFlag(Qt::ExpandedClientAreaHint) &&
        !window()->isFullScreen())
    {
        // QWidget may restore WindowTitleHint when it recreates its native window.
        // App-owned controls replace the Qt helper's caption visuals.
        // Only write a changed flag, avoiding needless native-frame updates.
        if (handle->flags().testFlag(Qt::WindowTitleHint))
            handle->setFlag(Qt::WindowTitleHint, false);
    }
#endif

    if (controls_)
    {
        controls_->setVisible(!window()->isFullScreen());
        reserved = window()->isFullScreen() ? 0 : controls_->sizeHint().width();
        maximize_->setText(QString(QChar(window()->isMaximized() ? 0xE923 : 0xE922)));
        maximize_->setAccessibleName(window()->isMaximized() ? "Restore Down" : "Maximize");
        maximize_->setToolTip(maximize_->accessibleName());
    }

    row_->setContentsMargins(16, controls_ ? 0 : 4, controls_ ? 0 : 4, controls_ ? 0 : 4);
    setProperty("captionReservedWidth", reserved);
}

void WindowHeader::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    updateCaptionSpace();
    // One event-driven retry after native creation; no polling. The context cancels
    // this callback if the header is destroyed before the event loop runs.
    QTimer::singleShot(0, this, &WindowHeader::updateCaptionSpace);
}

bool WindowHeader::event(QEvent *event)
{
    const bool handled = QWidget::event(event);
    if (event->type() == QEvent::DevicePixelRatioChange)
        updateCaptionSpace();
    return handled;
}

void WindowHeader::mousePressEvent(QMouseEvent *event)
{
    if (window() && event->button() == Qt::LeftButton && !window()->isFullScreen() &&
        window()->windowHandle())
    {
        if (window()->windowHandle()->startSystemMove())
        {
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void WindowHeader::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (window() && event->button() == Qt::LeftButton && !window()->isFullScreen())
    {
        requestMaximizeRestore();
        event->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

} // namespace qtappbase
