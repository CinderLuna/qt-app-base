// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#include "qtappbase/ResizeGripPainter.h"
#include "qtappbase/Theme.h"
#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QSizeGrip>

namespace qtappbase
{

ResizeGripPainter::ResizeGripPainter(QObject *parent) : QObject(parent) {}

bool ResizeGripPainter::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() != QEvent::Paint)
        return QObject::eventFilter(object, event);

    auto *grip = qobject_cast<QSizeGrip *>(object);
    if (!grip)
        return false;

    QPainter painter(grip);
    const auto colors = ThemeManager::instance().colors();
#if (defined(Q_OS_LINUX) || defined(Q_OS_UNIX)) && !defined(Q_OS_DARWIN)
    const bool isMax = grip->window() && (grip->window()->isMaximized() || grip->window()->isFullScreen());
    if (!isMax)
    {
        painter.setRenderHint(QPainter::Antialiasing, true);
        const bool rtl = grip->layoutDirection() == Qt::RightToLeft;
        const qreal w = grip->width();
        const qreal h = grip->height();
        QPainterPath p;
        if (rtl)
        {
            p.moveTo(0, 0);
            p.lineTo(w, 0);
            p.lineTo(w, h);
            p.lineTo(12.0, h);
            p.quadTo(0, h, 0, h - 12.0);
            p.closeSubpath();
        }
        else
        {
            p.moveTo(0, 0);
            p.lineTo(w, 0);
            p.lineTo(w, h - 12.0);
            p.quadTo(w, h, w - 12.0, h);
            p.lineTo(0, h);
            p.closeSubpath();
        }
        painter.fillPath(p, colors.chrome);
    }
    else
    {
        painter.fillRect(grip->rect(), colors.chrome);
    }
#else
    painter.fillRect(grip->rect(), colors.chrome);
#endif
    painter.setPen(Qt::NoPen);
    painter.setBrush(colors.muted);

    // Logical coordinates keep the six dots sharp at the current device DPI.
    const bool rtl = grip->layoutDirection() == Qt::RightToLeft;
    for (int row = 0; row < 3; ++row)
    {
        for (int column = 0; column <= row; ++column)
        {
            const int x = rtl ? 3 + column * 4 : grip->width() - 5 - column * 4;
            painter.drawRect(x, grip->height() - 13 + row * 4, 2, 2);
        }
    }
    return true;
}

} // namespace qtappbase
