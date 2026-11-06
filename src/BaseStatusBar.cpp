// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#include "qtappbase/BaseStatusBar.h"
#include "qtappbase/ResizeGripPainter.h"
#include <QLabel>
#include <QPushButton>
#include <QShowEvent>
#include <QSizeGrip>

namespace qtappbase
{

BaseStatusBar::BaseStatusBar(QWidget *parent) : QStatusBar(parent)
{
    setObjectName("baseStatusBar");
    setSizeGripEnabled(true);
    setupUi();
}

void BaseStatusBar::setupUi()
{
    // Main status label (left-aligned, stretching)
    statusLabel_ = new QLabel(this);
    statusLabel_->setObjectName("statusBarMainText");
    statusLabel_->setWordWrap(false);
    statusLabel_->setTextFormat(Qt::PlainText);
    statusLabel_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    statusLabel_->setAccessibleName("Application status");
    addWidget(statusLabel_, 1);

    // Info / metric label (permanent, right-aligned)
    infoLabel_ = new QLabel(this);
    infoLabel_->setObjectName("statusBarInfoText");
    infoLabel_->setTextFormat(Qt::PlainText);
    infoLabel_->setAccessibleName("Context metrics");
    addPermanentWidget(infoLabel_);
}

void BaseStatusBar::showEvent(QShowEvent *event)
{
    QStatusBar::showEvent(event);
    if (auto *grip = findChild<QSizeGrip *>())
    {
        grip->setObjectName("statusResizeGrip");
        static bool filterInstalled = false;
        if (!filterInstalled)
        {
            grip->installEventFilter(new ResizeGripPainter(grip));
            filterInstalled = true;
        }
    }
}

void BaseStatusBar::setStatusText(const QString &text)
{
    statusLabel_->setText(text);
    statusLabel_->setToolTip(text);
}

QString BaseStatusBar::statusText() const
{
    return statusLabel_->text();
}

void BaseStatusBar::setInfoText(const QString &text)
{
    infoLabel_->setText(text);
    infoLabel_->setVisible(!text.isEmpty());
}

QString BaseStatusBar::infoText() const
{
    return infoLabel_->text();
}

void BaseStatusBar::setActionButton(QPushButton *button)
{
    if (actionButton_)
    {
        removeWidget(actionButton_);
        actionButton_->deleteLater();
    }
    actionButton_ = button;
    if (actionButton_)
    {
        actionButton_->setObjectName("statusBarAction");
        actionButton_->setAutoDefault(false);
        addPermanentWidget(actionButton_);
    }
}

void BaseStatusBar::setDetailsButton(QPushButton *button)
{
    if (detailsButton_)
    {
        removeWidget(detailsButton_);
        detailsButton_->deleteLater();
    }
    detailsButton_ = button;
    if (detailsButton_)
    {
        detailsButton_->setObjectName("statusBarDetails");
        detailsButton_->setAutoDefault(false);
        addPermanentWidget(detailsButton_);
    }
}

} // namespace qtappbase
