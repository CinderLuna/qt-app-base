// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#include "qtappbase/WindowHeader.h"
#include "qtappbase/Theme.h"
#include <QApplication>
#include <QContextMenuEvent>
#include <QEnterEvent>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QMenu>
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

namespace
{

enum class CaptionType
{
    Minimize,
    Maximize,
    Restore,
    Close
};

class CaptionButton : public QToolButton
{
  public:
    explicit CaptionButton(CaptionType type, QWidget *parent = nullptr)
        : QToolButton(parent), type_(type)
    {
        setAttribute(Qt::WA_Hover, true);
    }

    void setCaptionType(CaptionType type)
    {
        if (type_ != type)
        {
            type_ = type;
            update();
        }
    }

    CaptionType captionType() const
    {
        return type_;
    }

  protected:
    void enterEvent(QEnterEvent *event) override
    {
        QToolButton::enterEvent(event);
        update();
    }

    void leaveEvent(QEvent *event) override
    {
        QToolButton::leaveEvent(event);
        update();
    }

    void paintEvent(QPaintEvent *event) override
    {
        QToolButton::paintEvent(event);

        // If the font glyph is rendered (e.g. Segoe Fluent Icons on Windows),
        // let the default text painting handle it.
        if (!text().isEmpty())
            return;

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        const QPointF c = rect().center();

        QColor color;
        if (type_ == CaptionType::Close && (underMouse() || isDown()))
        {
            color = Qt::white;
        }
        else
        {
            color = ThemeManager::instance().colors().text;
        }

#if (defined(Q_OS_LINUX) || defined(Q_OS_UNIX)) && !defined(Q_OS_DARWIN)
        // Authentic GNOME Adwaita vector glyphs
        QPen pen(color);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);

        switch (type_)
        {
        case CaptionType::Minimize: {
            // GNOME Adwaita minimize: 2px thick horizontal bar, positioned slightly below center
            pen.setWidthF(2.0);
            painter.setPen(pen);
            painter.drawLine(QPointF(c.x() - 4.5, c.y() + 2.5), QPointF(c.x() + 4.5, c.y() + 2.5));
            break;
        }

        case CaptionType::Maximize: {
            // GNOME Adwaita maximize: 9x9 box with 1.75px stroke
            pen.setWidthF(1.75);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(QRectF(c.x() - 4.5, c.y() - 4.5, 9.0, 9.0), 0.75, 0.75);
            break;
        }

        case CaptionType::Restore: {
            // GNOME Adwaita restore: overlapping boxes
            pen.setWidthF(1.5);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);
            // Back rectangle (upper-right)
            painter.drawLine(QPointF(c.x() - 1.5, c.y() - 4.5), QPointF(c.x() + 4.5, c.y() - 4.5));
            painter.drawLine(QPointF(c.x() + 4.5, c.y() - 4.5), QPointF(c.x() + 4.5, c.y() + 1.5));
            painter.drawLine(QPointF(c.x() + 1.5, c.y() + 1.5), QPointF(c.x() + 4.5, c.y() + 1.5));
            painter.drawLine(QPointF(c.x() - 1.5, c.y() - 4.5), QPointF(c.x() - 1.5, c.y() - 1.5));

            // Front rectangle (lower-left)
            painter.drawRoundedRect(QRectF(c.x() - 4.5, c.y() - 1.5, 6.0, 6.0), 0.5, 0.5);
            break;
        }

        case CaptionType::Close: {
            // GNOME Adwaita close: 2px rounded cross
            pen.setWidthF(2.0);
            painter.setPen(pen);
            painter.drawLine(QPointF(c.x() - 4.0, c.y() - 4.0), QPointF(c.x() + 4.0, c.y() + 4.0));
            painter.drawLine(QPointF(c.x() - 4.0, c.y() + 4.0), QPointF(c.x() + 4.0, c.y() - 4.0));
            break;
        }
        }
#else
        QPen pen(color);
        pen.setWidthF(1.25);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);

        switch (type_)
        {
        case CaptionType::Minimize:
            painter.drawLine(QPointF(c.x() - 5.0, c.y()), QPointF(c.x() + 5.0, c.y()));
            break;

        case CaptionType::Maximize:
            painter.drawRect(QRectF(c.x() - 5.0, c.y() - 5.0, 10.0, 10.0));
            break;

        case CaptionType::Restore:
            // Back rectangle (upper-right)
            painter.drawLine(QPointF(c.x() - 2.0, c.y() - 5.0), QPointF(c.x() + 5.0, c.y() - 5.0));
            painter.drawLine(QPointF(c.x() + 5.0, c.y() - 5.0), QPointF(c.x() + 5.0, c.y() + 2.0));
            painter.drawLine(QPointF(c.x() + 2.0, c.y() + 2.0), QPointF(c.x() + 5.0, c.y() + 2.0));
            painter.drawLine(QPointF(c.x() - 2.0, c.y() - 5.0), QPointF(c.x() - 2.0, c.y() - 2.0));

            // Front rectangle (lower-left)
            painter.drawRect(QRectF(c.x() - 5.0, c.y() - 2.0, 7.0, 7.0));
            break;

        case CaptionType::Close:
            painter.drawLine(QPointF(c.x() - 4.5, c.y() - 4.5), QPointF(c.x() + 4.5, c.y() + 4.5));
            painter.drawLine(QPointF(c.x() - 4.5, c.y() + 4.5), QPointF(c.x() + 4.5, c.y() - 4.5));
            break;
        }
#endif
    }

  private:
    CaptionType type_;
};

} // namespace

bool WindowHeader::supportsExpandedTitleBar()
{
#if defined(Q_OS_WIN) && QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    return QGuiApplication::platformName() == "windows";
#elif (defined(Q_OS_LINUX) || defined(Q_OS_UNIX)) && !defined(Q_OS_DARWIN)
    return true;
#else
    return false;
#endif
}

WindowHeader::WindowHeader(QWidget *parent) : QWidget(parent), row_(new QHBoxLayout(this))
{
    setObjectName("applicationHeader");
    setAttribute(Qt::WA_StyledBackground, true);
    setAccessibleName("Application header");
    setMouseTracking(true);
    row_->setContentsMargins(16, 4, 16, 4);
    row_->setSpacing(0);
    setMinimumHeight(48);

    connect(&ThemeManager::instance(), &ThemeManager::changed, this,
            [this]
            {
                if (controls_)
                    controls_->update();
            });
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
#if (defined(Q_OS_LINUX) || defined(Q_OS_UNIX)) && !defined(Q_OS_DARWIN)
    layout->setSpacing(8);
    layout->setAlignment(Qt::AlignCenter);
#else
    layout->setSpacing(0);
#endif

#if defined(Q_OS_WIN)
    const auto families = QFontDatabase::families();
    const bool hasGlyphFont = families.contains("Segoe Fluent Icons") || families.contains("Segoe MDL2 Assets");
    QFont glyphs(hasGlyphFont ? (families.contains("Segoe Fluent Icons") ? "Segoe Fluent Icons" : "Segoe MDL2 Assets") : QString());
    glyphs.setPixelSize(10);
#endif

    const auto add = [&](const char *name, const char *label, ushort glyph, CaptionType type) -> QToolButton *
    {
        Q_UNUSED(glyph);
        auto *button = new CaptionButton(type, controls_);
        button->setObjectName(name);
        button->setAccessibleName(label);
        button->setToolTip(label);
#if defined(Q_OS_WIN)
        if (hasGlyphFont)
        {
            button->setText(QString(QChar(glyph)));
            button->setFont(glyphs);
            button->setStyleSheet(QString("QToolButton { padding:0; border:0; border-radius:0; "
                                          "font-family:'%1'; font-size:10px; }")
                                      .arg(glyphs.family()));
        }
        else
        {
            button->setStyleSheet("QToolButton { padding:0; border:0; border-radius:0; }");
        }
        button->setFixedWidth(46);
        button->setMinimumHeight(32);
        button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
#elif (defined(Q_OS_LINUX) || defined(Q_OS_UNIX)) && !defined(Q_OS_DARWIN)
        button->setFixedSize(26, 26);
        button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
#else
        button->setFixedWidth(46);
        button->setMinimumHeight(32);
        button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
#endif
        layout->addWidget(button);
        return button;
    };

    minimize_ = add("captionMinimize", "Minimize", 0xE921, CaptionType::Minimize);
    maximize_ = add("captionMaximize", "Maximize", 0xE922, CaptionType::Maximize);
    close_ = add("captionClose", "Close", 0xE8BB, CaptionType::Close);
#if defined(Q_OS_WIN)
    close_->setStyleSheet(close_->styleSheet() + "QToolButton:hover { background:#C42B1C; color:white; }");
#endif

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
#else
    connect(minimize_, &QToolButton::clicked, this,
            [this]
            {
                if (window())
                    window()->showMinimized();
            });
    connect(close_, &QToolButton::clicked, this,
            [this]
            {
                if (window())
                    window()->close();
            });
#endif
    connect(maximize_, &QToolButton::clicked, this, &WindowHeader::requestMaximizeRestore);

    row_->addSpacing(8);
    row_->addWidget(controls_);
#if (defined(Q_OS_LINUX) || defined(Q_OS_UNIX)) && !defined(Q_OS_DARWIN)
    row_->setContentsMargins(16, 4, 12, 4);
#else
    row_->setContentsMargins(16, 0, 0, 0);
#endif
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
    if (window()->isFullScreen())
        return;

    QMenu menu(this);
    const bool isMax = window()->isMaximized();

    auto *restoreAct = menu.addAction("&Restore", this, &WindowHeader::requestMaximizeRestore);
    restoreAct->setEnabled(isMax);

    auto *moveAct = menu.addAction("&Move", this, [this] {
        if (window() && window()->windowHandle())
            window()->windowHandle()->startSystemMove();
    });
    moveAct->setEnabled(!isMax);

    menu.addAction("Mi&nimize", this, [this] { if (window()) window()->showMinimized(); });
    auto *maxAct = menu.addAction("Ma&ximize", this, &WindowHeader::requestMaximizeRestore);
    maxAct->setEnabled(!isMax);

    menu.addSeparator();
    auto *closeAct = menu.addAction("&Close", this, [this] { if (window()) window()->close(); });
    closeAct->setShortcut(QKeySequence(Qt::ALT | Qt::Key_F4));

    const QPoint pos = atCursor ? QCursor::pos() : mapToGlobal(QPoint(0, height()));
    menu.exec(pos);
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
        const bool maximized = window()->isMaximized();

        if (auto *cap = static_cast<CaptionButton *>(maximize_))
            cap->setCaptionType(maximized ? CaptionType::Restore : CaptionType::Maximize);

        if (!maximize_->text().isEmpty())
            maximize_->setText(QString(QChar(maximized ? 0xE923 : 0xE922)));

        maximize_->setAccessibleName(maximized ? "Restore Down" : "Maximize");
        maximize_->setToolTip(maximize_->accessibleName());
        maximize_->update();
    }

    const bool hasVisibleControls = controls_ && controls_->isVisible();
#if (defined(Q_OS_LINUX) || defined(Q_OS_UNIX)) && !defined(Q_OS_DARWIN)
    row_->setContentsMargins(16, 4, hasVisibleControls ? 12 : 16, 4);
#else
    row_->setContentsMargins(16, hasVisibleControls ? 0 : 4, hasVisibleControls ? 0 : 16, hasVisibleControls ? 0 : 4);
#endif
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
    if (window() && event->button() == Qt::LeftButton && !window()->isFullScreen())
    {
#if (defined(Q_OS_LINUX) || defined(Q_OS_UNIX)) && !defined(Q_OS_DARWIN)
        if (!window()->isMaximized() && event->position().y() <= 6 && window()->windowHandle())
        {
            if (window()->windowHandle()->startSystemResize(Qt::TopEdge))
            {
                event->accept();
                return;
            }
        }
#endif
        dragStartPos_ = event->position().toPoint();
        dragPending_ = true;
    }
    QWidget::mousePressEvent(event);
}

void WindowHeader::mouseMoveEvent(QMouseEvent *event)
{
#if (defined(Q_OS_LINUX) || defined(Q_OS_UNIX)) && !defined(Q_OS_DARWIN)
    if (window() && !window()->isMaximized() && !window()->isFullScreen())
    {
        if (event->position().y() <= 6)
            setCursor(Qt::SizeVerCursor);
        else if (cursor().shape() == Qt::SizeVerCursor)
            unsetCursor();
    }
#endif
    if (dragPending_ && (event->buttons() & Qt::LeftButton) && window() && !window()->isFullScreen())
    {
        if ((event->position().toPoint() - dragStartPos_).manhattanLength() >= QApplication::startDragDistance())
        {
            dragPending_ = false;
#if (defined(Q_OS_LINUX) || defined(Q_OS_UNIX)) && !defined(Q_OS_DARWIN)
            if (window()->isMaximized())
            {
                const double ratio = double(event->position().x()) / qMax(1, width());
                window()->showNormal();
                const int restoredWidth = window()->width();
                const int newX = event->globalPosition().toPoint().x() - qRound(restoredWidth * ratio);
                const int newY = event->globalPosition().toPoint().y() - event->position().toPoint().y();
                window()->move(newX, newY);
            }
#endif
            if (window()->windowHandle() && window()->windowHandle()->startSystemMove())
            {
                event->accept();
                return;
            }
        }
    }
    QWidget::mouseMoveEvent(event);
}

void WindowHeader::mouseReleaseEvent(QMouseEvent *event)
{
    dragPending_ = false;
    QWidget::mouseReleaseEvent(event);
}

void WindowHeader::leaveEvent(QEvent *event)
{
#if (defined(Q_OS_LINUX) || defined(Q_OS_UNIX)) && !defined(Q_OS_DARWIN)
    if (cursor().shape() == Qt::SizeVerCursor)
        unsetCursor();
#endif
    QWidget::leaveEvent(event);
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
