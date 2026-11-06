// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#ifndef QTAPPBASE_WINDOWTITLELABEL_H
#define QTAPPBASE_WINDOWTITLELABEL_H

#include <QLabel>

namespace qtappbase
{

/// A label designed for the custom title bar that renders smoothly elided text.
/// It ignores mouse events so drag gestures on empty title bar area pass through to the header.
class WindowTitleLabel : public QLabel
{
    Q_OBJECT
  public:
    explicit WindowTitleLabel(QWidget *parent = nullptr);
    explicit WindowTitleLabel(const QString &text, QWidget *parent = nullptr);

    QString displayText() const;

  protected:
    void paintEvent(QPaintEvent *event) override;
};

} // namespace qtappbase

#endif // QTAPPBASE_WINDOWTITLELABEL_H
