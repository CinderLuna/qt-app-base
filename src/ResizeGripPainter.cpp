// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#include "qtappbase/ResizeGripPainter.h"
#include "qtappbase/Theme.h"
#include <QEvent>
#include <QPainter>
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
    painter.fillRect(grip->rect(), colors.chrome);
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
