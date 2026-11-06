// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#ifndef QTAPPBASE_RESIZEGRIPPAINTER_H
#define QTAPPBASE_RESIZEGRIPPAINTER_H

#include <QObject>

namespace qtappbase
{

/// Custom event filter for QSizeGrip in the status bar.
/// It renders a theme-aware 6-dot grip with crisp device-DPI alignment and RTL support.
class ResizeGripPainter : public QObject
{
    Q_OBJECT
  public:
    explicit ResizeGripPainter(QObject *parent = nullptr);

  protected:
    bool eventFilter(QObject *object, QEvent *event) override;
};

} // namespace qtappbase

#endif // QTAPPBASE_RESIZEGRIPPAINTER_H
