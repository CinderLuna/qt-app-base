// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#include "qtappbase/SearchLineEdit.h"
#include <QEvent>
#include <QLabel>
#include <QResizeEvent>
#include <QStyle>

namespace qtappbase
{

SearchLineEdit::SearchLineEdit(QWidget *parent) : QLineEdit(parent)
{
    setupUi();
}

SearchLineEdit::SearchLineEdit(const QString &placeholder, QWidget *parent) : QLineEdit(parent)
{
    setPlaceholderText(placeholder);
    setupUi();
}

void SearchLineEdit::setupUi()
{
    setObjectName("headerSearchEdit");
    setClearButtonEnabled(true);

    shortcutLabel_ = new QLabel("Ctrl+K", this);
    shortcutLabel_->setObjectName("searchShortcut");
    shortcutLabel_->setAttribute(Qt::WA_TransparentForMouseEvents);
    shortcutLabel_->setAccessibleName("Shortcut Ctrl+K");

    connect(this, &QLineEdit::textChanged, this,
            [this](const QString &text) { shortcutLabel_->setVisible(text.isEmpty()); });

    updateShortcutPosition();
}

void SearchLineEdit::setShortcutText(const QString &text)
{
    shortcutLabel_->setText(text);
    shortcutLabel_->setAccessibleName("Shortcut " + text);
    updateShortcutPosition();
}

QString SearchLineEdit::shortcutText() const
{
    return shortcutLabel_->text();
}

void SearchLineEdit::updateShortcutPosition()
{
    if (!shortcutLabel_)
        return;

    shortcutLabel_->adjustSize();
    const int rightPadding = 10;
    const int x = width() - shortcutLabel_->width() - rightPadding;
    const int y = (height() - shortcutLabel_->height()) / 2;
    shortcutLabel_->move(x, y);

    setTextMargins(0, 0, shortcutLabel_->width() + rightPadding + 6, 0);
}

void SearchLineEdit::resizeEvent(QResizeEvent *event)
{
    QLineEdit::resizeEvent(event);
    updateShortcutPosition();
}

void SearchLineEdit::changeEvent(QEvent *event)
{
    QLineEdit::changeEvent(event);
    if (shortcutLabel_ && (event->type() == QEvent::FontChange || event->type() == QEvent::StyleChange))
    {
        updateShortcutPosition();
    }
}

} // namespace qtappbase
