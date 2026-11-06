// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#include "qtappbase/WindowTitleLabel.h"
#include <QPaintEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

namespace qtappbase
{

WindowTitleLabel::WindowTitleLabel(QWidget *parent) : QLabel(parent)
{
    setObjectName("windowTitleLabel");
    setMinimumWidth(0);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    setTextFormat(Qt::PlainText);
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

WindowTitleLabel::WindowTitleLabel(const QString &text, QWidget *parent) : WindowTitleLabel(parent)
{
    setText(text);
}

QString WindowTitleLabel::displayText() const
{
    return fontMetrics().elidedText(text(), Qt::ElideRight, qMax(0, contentsRect().width() - 2 * margin()));
}

void WindowTitleLabel::paintEvent(QPaintEvent *)
{
    QStyleOption option;
    option.initFrom(this);
    QPainter painter(this);
    painter.setFont(font());
    style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);

    const auto area = contentsRect().adjusted(margin(), 0, -margin(), 0);
    painter.setClipRect(area);
    painter.setPen(palette().color(foregroundRole()));
    painter.drawText(area, QStyle::visualAlignment(layoutDirection(), alignment()) | Qt::TextSingleLine,
                     displayText());
}

} // namespace qtappbase
