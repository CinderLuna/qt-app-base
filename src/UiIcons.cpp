// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#include "qtappbase/UiIcons.h"
#include "qtappbase/Theme.h"
#include <QBuffer>
#include <QEvent>
#include <QFile>
#include <QImageReader>
#include <QPainter>
#include <QStyleOptionToolButton>
#include <QStylePainter>
#include <QTabBar>
#include <QTimer>
#include <QToolButton>
#include <QVariant>

static void registerToolbarIcons()
{
    Q_INIT_RESOURCE(toolbar_icons);
}

static void initializeToolbarIcons()
{
    static const bool initialized = []
    {
        registerToolbarIcons();
        return true;
    }();
    Q_UNUSED(initialized);
}

namespace qtappbase
{

namespace
{

class TabScrollIcons final : public QObject
{
    Q_OBJECT
    QTabBar *tabs_;
    bool pending_ = false;

    void refresh()
    {
        for (auto *button : tabs_->findChildren<QToolButton *>(QString(), Qt::FindDirectChildrenOnly))
        {
            auto direction = button->arrowType();
            if (direction == Qt::LeftArrow || direction == Qt::RightArrow)
                button->setProperty("tabScrollDirection", int(direction));
            else if (button->property("tabScrollDirection").isValid())
                direction = Qt::ArrowType(button->property("tabScrollDirection").toInt());
            else
                continue;

            const bool left = direction == Qt::LeftArrow;
            button->installEventFilter(this);
            button->setIcon(
                uiIcon(left ? "chevron_left" : "chevron_right", ThemeManager::instance().colors().text));
            button->setIconSize(QSize(16, 16));
            button->setFixedWidth(24);
            const QString style = "QToolButton { padding:0; border:0; border-radius:0; }";
            if (button->styleSheet() != style)
                button->setStyleSheet(style);
            button->setAccessibleName(left ? "Scroll tabs left" : "Scroll tabs right");
            button->setToolTip(button->accessibleName());
        }
    }

    void schedule()
    {
        if (pending_)
            return;
        pending_ = true;
        QTimer::singleShot(0, this,
                           [this]
                           {
                               pending_ = false;
                               refresh();
                           });
    }

    bool eventFilter(QObject *object, QEvent *event) override
    {
        // Qt restores arrow types during layout. Supply an icon-only style
        // option at paint time, leaving native scroll direction/layout intact.
        if (event->type() == QEvent::Paint)
        {
            if (auto *button = qobject_cast<QToolButton *>(object))
            {
                if (button->property("tabScrollDirection").isValid())
                {
                    QStyleOptionToolButton option;
                    option.initFrom(button);
                    option.icon = button->icon();
                    option.iconSize = button->iconSize();
                    option.toolButtonStyle = Qt::ToolButtonIconOnly;
                    option.subControls = QStyle::SC_ToolButton;
                    option.state |= button->isDown() ? QStyle::State_Sunken : QStyle::State_Raised;
                    QStylePainter painter(button);
                    painter.drawComplexControl(QStyle::CC_ToolButton, option);
                    return true;
                }
            }
        }

        if (event->type() == QEvent::Show || event->type() == QEvent::Resize ||
            event->type() == QEvent::StyleChange || event->type() == QEvent::LayoutDirectionChange ||
            event->type() == QEvent::ChildAdded)
        {
            schedule();
        }

        return QObject::eventFilter(object, event);
    }

  public:
    explicit TabScrollIcons(QTabBar *tabs) : QObject(tabs), tabs_(tabs)
    {
        setObjectName("tabScrollIcons");
        tabs->installEventFilter(this);
        connect(&ThemeManager::instance(), &ThemeManager::changed, this, [this] { refresh(); });
        refresh();
    }
};

} // namespace

void styleDocumentTabScrolling(QTabBar *tabs)
{
    if (!tabs->findChild<QObject *>("tabScrollIcons", Qt::FindDirectChildrenOnly))
        new TabScrollIcons(tabs);
}

QIcon uiIcon(const QString &name, const QColor &color)
{
    static const bool initialized = []
    {
        initializeToolbarIcons();
        return true;
    }();
    Q_UNUSED(initialized);

    const QIcon source(":/icons/fluent/" + name + ".svg");
    QIcon result;
    for (const int size : {16, 20, 24, 32, 40, 48, 64})
    {
        auto pixmap = source.pixmap(size, size);
        if (pixmap.isNull())
            continue;
        QPainter painter(&pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(pixmap.rect(), color);
        painter.end();
        result.addPixmap(pixmap);
    }
    return result;
}

QIcon uiColorIcon(const QString &name, const QColor &foreground, const QColor &selected)
{
    initializeToolbarIcons();
    QFile file(":/icons/fluent/" + name + ".svg");
    if (!file.open(QIODevice::ReadOnly))
        return {};

    auto svg = file.readAll();
    svg.replace("#212121", foreground.name().toUtf8());
    svg.replace("#C44545", selected.name().toUtf8());
    svg.replace("#F2DA70", selected.name().toUtf8());
    svg.replace("fill-opacity=\"1\"", "fill-opacity=\"" + QByteArray::number(selected.alphaF()) + "\"");

    QIcon result;
    for (const int size : {16, 20, 24, 32, 40, 48, 64})
    {
        QBuffer buffer(&svg);
        buffer.open(QIODevice::ReadOnly);
        QImageReader reader(&buffer, "svg");
        reader.setScaledSize(QSize(size, size));
        result.addPixmap(QPixmap::fromImage(reader.read()));
    }
    return result;
}

} // namespace qtappbase

#include "UiIcons.moc"
