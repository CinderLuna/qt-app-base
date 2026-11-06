// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#ifndef QTAPPBASE_UIICONS_H
#define QTAPPBASE_UIICONS_H

#include <QColor>
#include <QIcon>

class QTabBar;

namespace qtappbase
{

/// Loads and recolors monochrome Fluent SVG icons with the specified semantic color.
QIcon uiIcon(const QString &name, const QColor &color);

/// Loads and recolors duo-tone SVG icons with foreground and highlight colors.
QIcon uiColorIcon(const QString &name, const QColor &foreground, const QColor &selected);

/// Installs custom themed scroll buttons for tab bars with overflowing tabs.
void styleDocumentTabScrolling(QTabBar *tabs);

} // namespace qtappbase

#endif // QTAPPBASE_UIICONS_H
