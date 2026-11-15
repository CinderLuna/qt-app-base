// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#include "qtappbase/Inspector.h"

#if !defined(NDEBUG) || defined(QT_DEBUG) || defined(QTAPPBASE_DEBUG)

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QApplication>
#include <QClipboard>
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QSignalBlocker>
#include <QTabBar>
#include <QTimer>
#include <QToolButton>
#include <QVariant>
#include <QWindow>

namespace qtinspector
{
namespace
{

constexpr auto Internal = "_qtWidgetInspector";

QString identity(const QObject *object)
{
    QString result = QString::fromLatin1(object->metaObject()->className());
    if (!object->objectName().isEmpty())
    {
        result += "#" + object->objectName();
    }
    else if (object->parent())
    {
        int sibling = 0;
        for (auto *other : object->parent()->children())
        {
            if (other == object)
                break;
            if (other->metaObject() == object->metaObject())
                ++sibling;
        }
        result += QString("[%1]").arg(sibling);
    }
    return result;
}

QString rectText(const QRect &rect)
{
    return QString("x=%1 y=%2 width=%3 height=%4")
        .arg(rect.x())
        .arg(rect.y())
        .arg(rect.width())
        .arg(rect.height());
}

class Outline final : public QWidget
{
  public:
    explicit Outline(QWidget *parent) : QWidget(parent)
    {
        setProperty(Internal, true);
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setFocusPolicy(Qt::NoFocus);
    }

    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setPen(QPen(QColor("#83b7ff"), 2));
        painter.setBrush(QColor(90, 160, 255, 24));
        painter.drawRect(rect().adjusted(1, 1, -1, -1));
    }
};

class Handle final : public QWidget
{
    QPoint offset_;
    bool dragging_ = false;

  public:
    explicit Handle(QWidget *parent) : QWidget(parent)
    {
        setObjectName("inspectorDragHandle");
        setFixedWidth(24);
        setCursor(Qt::SizeAllCursor);
        setToolTip("Drag inspector toolbar");
        setAccessibleName("Drag inspector toolbar");
    }

    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#a6b5c8"));
        for (int row = 0; row < 3; ++row)
            for (int col = 0; col < 2; ++col)
                p.drawEllipse(QPointF(8 + col * 6, height() / 2.0 - 6 + row * 6), 1.4, 1.4);
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton && parentWidget())
        {
            dragging_ = true;
            offset_ = event->globalPosition().toPoint() - parentWidget()->mapToGlobal(QPoint(0, 0));
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        auto *bar = parentWidget();
        if (!bar)
            return;
        auto *host = bar->parentWidget();
        if (!dragging_ || !host)
            return;
        auto position = host->mapFromGlobal(event->globalPosition().toPoint() - offset_);
        position.setX(qBound(0, position.x(), qMax(0, host->width() - bar->width())));
        position.setY(qBound(0, position.y(), qMax(0, host->height() - bar->height())));
        bar->move(position);
    }

    void mouseReleaseEvent(QMouseEvent *) override
    {
        dragging_ = false;
    }
};

} // namespace

Inspector *Inspector::instance()
{
    if (!qApp)
        return nullptr;
    for (auto *child : qApp->children())
    {
        if (child->objectName() == "qtWidgetInspectorController")
            return static_cast<Inspector *>(child);
    }
    return nullptr;
}

Inspector *Inspector::install()
{
    Q_ASSERT(qApp);
    if (auto *existing = instance())
        return existing;
    return new Inspector(qApp);
}

Inspector::Inspector(QObject *parent) : QObject(parent)
{
    setObjectName("qtWidgetInspectorController");
    qApp->installEventFilter(this);
    QTimer::singleShot(0, this,
                       [this]
                       {
                           if (auto *active = QApplication::activeWindow())
                               attach(active);
                           else
                               for (auto *topLevel : QApplication::topLevelWidgets())
                                   if (topLevel->isVisible())
                                   {
                                       attach(topLevel);
                                       break;
                                   }
                       });
}

bool Inspector::isInternal(const QWidget *widget) const
{
    for (auto *current = widget; current; current = current->parentWidget())
    {
        if (current->property(Internal).toBool())
            return true;
    }
    return false;
}

void Inspector::attach(QWidget *window)
{
    if (!window || !window->isWindow() || isInternal(window) || !window->isVisible() ||
        (window->windowType() != Qt::Window && window->windowType() != Qt::Dialog))
    {
        return;
    }
    if (bar_ && bar_->parentWidget() == window)
        return;
    if (bar_)
        bar_->hide();

    auto *bar = window->findChild<QWidget *>("qtWidgetInspectorBar", Qt::FindDirectChildrenOnly);
    if (!bar)
    {
        bar = new QWidget(window);
        bar->setObjectName("qtWidgetInspectorBar");
        bar->setProperty(Internal, true);
        bar->setAttribute(Qt::WA_StyledBackground);
        bar->setStyleSheet(
            "QWidget#qtWidgetInspectorBar { background:#202833; border:1px solid #65758b; border-radius:3px; "
            "}"
            "QWidget#qtWidgetInspectorBar QToolButton { background:transparent; color:#e6edf7; border:0; "
            "padding:6px 9px; }"
            "QWidget#qtWidgetInspectorBar QToolButton:checked { background:#41628c; }"
            "QWidget#qtWidgetInspectorBar QToolButton:hover { background:#35475e; }"
            "QWidget#qtWidgetInspectorBar QLabel { background:transparent; color:#b8c9df; border:0; }");
        auto *layout = new QHBoxLayout(bar);
        layout->setContentsMargins(3, 2, 5, 2);
        layout->setSpacing(2);
        layout->addWidget(new Handle(bar));

        auto button = [bar, layout](const QString &text, const QString &name, const QString &tip)
        {
            auto *item = new QToolButton(bar);
            item->setText(text);
            item->setObjectName(name);
            item->setToolTip(tip);
            item->setAccessibleName(tip);
            layout->addWidget(item);
            return item;
        };

        auto *select = button("Select", "inspectorSelect",
                              "Select widgets and copy on click (F8). Escape exits selection.");
        select->setCheckable(true);
        connect(select, &QToolButton::toggled, this, &Inspector::setPicking);

        auto *parent = button("Parent", "inspectorParent", "Select the parent widget and copy its report");
        connect(parent, &QToolButton::clicked, this,
                [this]
                {
                    if (selected_ && selected_->parentWidget())
                    {
                        selected_ = selected_->parentWidget();
                        selectedItem_.clear();
                        highlight(selected_);
                        copySelection();
                    }
                });

        auto *copy = button("Copy", "inspectorCopy", "Copy the selected widget report again");
        connect(copy, &QToolButton::clicked, this, &Inspector::copySelection);

        auto *status = new QLabel("Ready", bar);
        status->setObjectName("inspectorStatus");
        status->setFixedWidth(140);
        layout->addWidget(status);

        auto *close = button("Hide", "inspectorHide", "Hide inspector (Ctrl+Alt+F12 to show)");
        connect(close, &QToolButton::clicked, this, &Inspector::toggleVisible);

        bar->adjustSize();
        bar->move(qMax(0, (window->width() - bar->width()) / 2),
                  qMax(0, window->height() - bar->height() - 12));
    }

    bar_ = bar;
    selectButton_ = bar->findChild<QToolButton *>("inspectorSelect");
    status_ = bar->findChild<QLabel *>("inspectorStatus");
    if (selectButton_)
    {
        QSignalBlocker blocker(selectButton_);
        selectButton_->setChecked(picking_);
    }
    bar->setVisible(visible_);
    bar->raise();
}

void Inspector::track(QWidget *widget)
{
    if (!widget || isInternal(widget))
        return;
    for (const auto &item : tracking_)
    {
        if (item.widget == widget)
            return;
    }
    tracking_.push_back({widget, widget->hasMouseTracking()});
    widget->setMouseTracking(true);
}

void Inspector::setPicking(bool enabled)
{
    if (enabled == picking_)
        return;
    picking_ = enabled;
    if (enabled)
    {
        for (auto *widget : QApplication::allWidgets())
            track(widget);
    }
    else
    {
        for (const auto &item : tracking_)
        {
            if (item.widget)
                item.widget->setMouseTracking(item.mouseTracking);
        }
        tracking_.clear();
        if (outline_)
            outline_->hide();
        hovered_.clear();
    }
    if (selectButton_)
    {
        QSignalBlocker blocker(selectButton_);
        selectButton_->setChecked(enabled);
    }
    if (status_)
        status_->setText(enabled ? "Click a widget" : "Ready");
}

void Inspector::highlight(QWidget *widget)
{
    if (outline_)
        delete outline_.data();
    hovered_ = widget;
    if (!widget || isInternal(widget))
        return;
    auto *outline = new Outline(widget);
    outline_ = outline;
    outline->setGeometry(widget->rect());
    outline->show();
    outline->raise();
    if (bar_)
        bar_->raise();
}

void Inspector::copySelection()
{
    if (!selected_)
    {
        if (status_)
            status_->setText("Select a widget first");
        return;
    }
    if (auto *clipboard = QApplication::clipboard())
    {
        clipboard->setText(
            report(selected_) +
            (selectedItem_.isEmpty() ? QString() : "\nClicked item (at capture): " + selectedItem_));
    }
    if (status_)
    {
        status_->setText("Copied " + QString::fromLatin1(selected_->metaObject()->className()));
        status_->setToolTip(identity(selected_));
    }
}

void Inspector::setVisible(bool visible)
{
    if (visible_ == visible)
        return;
    visible_ = visible;
    if (!visible_)
        setPicking(false);
    if (bar_)
        bar_->setVisible(visible_);
}

void Inspector::toggleVisible()
{
    setVisible(!visible_);
}

bool Inspector::eventFilter(QObject *object, QEvent *event)
{
    auto *widget = qobject_cast<QWidget *>(object);
    if (!widget)
        return false;

    if ((event->type() == QEvent::Show || event->type() == QEvent::WindowActivate) && !isInternal(widget))
    {
        if (picking_)
            track(widget);
        if (widget->isWindow())
        {
            QTimer::singleShot(0, this,
                               [this, window = QPointer<QWidget>(widget)]
                               {
                                   if (window)
                                       attach(window);
                               });
        }
    }

    if (event->type() == QEvent::Resize && bar_ && widget == bar_->parentWidget())
    {
        bar_->move(qBound(0, bar_->x(), qMax(0, widget->width() - bar_->width())),
                   qBound(0, bar_->y(), qMax(0, widget->height() - bar_->height())));
    }

    if (event->type() == QEvent::KeyPress || event->type() == QEvent::ShortcutOverride)
    {
        auto *key = static_cast<QKeyEvent *>(event);
        const bool toggle =
            key->key() == Qt::Key_F12 && key->modifiers() == (Qt::ControlModifier | Qt::AltModifier);
        const bool pick = visible_ && key->key() == Qt::Key_F8 && key->modifiers() == Qt::NoModifier;
        const bool escape = picking_ && key->key() == Qt::Key_Escape;
        if (toggle || pick || escape)
        {
            event->accept();
            if (event->type() == QEvent::KeyPress && !key->isAutoRepeat())
            {
                if (toggle)
                {
                    attach(widget->window());
                    toggleVisible();
                }
                else
                {
                    setPicking(pick ? !picking_ : false);
                }
            }
            return true;
        }
    }

    if (!picking_ || isInternal(widget))
        return false;

    if (event->type() == QEvent::MouseMove)
    {
        auto *mouse = static_cast<QMouseEvent *>(event);
        auto *target = QApplication::widgetAt(mouse->globalPosition().toPoint());
        if (target && !isInternal(target) && hovered_ != target)
            highlight(target);
        return true;
    }

    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick)
    {
        auto *mouse = static_cast<QMouseEvent *>(event);
        if (mouse->button() == Qt::LeftButton)
        {
            selected_ = widget;
            selectedItem_.clear();
            if (auto *tabs = qobject_cast<QTabBar *>(widget))
            {
                selectedItem_ =
                    QString("tab index=%1 (zero-based)").arg(tabs->tabAt(mouse->position().toPoint()));
            }
            for (auto *ancestor = widget; ancestor; ancestor = ancestor->parentWidget())
            {
                if (auto *view = qobject_cast<QAbstractItemView *>(ancestor))
                {
                    const auto point = view->viewport()->mapFromGlobal(mouse->globalPosition().toPoint());
                    auto index = view->indexAt(point);
                    if (index.isValid())
                    {
                        QStringList rows;
                        const auto column = index.column();
                        for (; index.isValid(); index = index.parent())
                            rows.prepend(QString::number(index.row()));
                        selectedItem_ =
                            QString("model=%1; row path=%2; column=%3 (zero-based; item contents excluded)")
                                .arg(QString::fromLatin1(view->model()->metaObject()->className()),
                                     rows.join('/'))
                                .arg(column);
                    }
                    break;
                }
            }
            highlight(widget);
            copySelection();
        }
        return true;
    }

    return event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::Wheel ||
           event->type() == QEvent::ContextMenu;
}

QString Inspector::report(const QWidget *widget)
{
    if (!widget)
        return {};
    QStringList lines{"Qt Widgets inspector report v1",
                      "Application: " + QCoreApplication::applicationName(),
                      "Qt: " + QString::fromLatin1(qVersion()),
                      "Class: " + QString::fromLatin1(widget->metaObject()->className()),
                      "Object name: " +
                          (widget->objectName().isEmpty() ? "(unnamed)" : widget->objectName())};
    QStringList path;
    for (auto *current = widget; current; current = current->parentWidget())
        path.prepend(identity(current));
    lines << "Widget path: " + path.join(" / ");
    if (!widget->objectName().isEmpty())
    {
        lines << "Locator: findChild<" + QString::fromLatin1(widget->metaObject()->className()) + " *>(" +
                     QString::fromUtf8("\"") + widget->objectName() +
                     "\") (verify uniqueness in this window)";
    }
    lines << "Geometry in parent (logical pixels): " + rectText(widget->geometry());
    lines << "Global geometry (logical pixels): " +
                 rectText(QRect(widget->mapToGlobal(QPoint(0, 0)), widget->size()));
    lines << QString("Visible: %1; enabled: %2; device pixel ratio: %3")
                 .arg(widget->isVisible())
                 .arg(widget->isEnabled())
                 .arg(widget->devicePixelRatioF());
    lines << "Minimum size: " + rectText(QRect({}, widget->minimumSize()))
          << "Maximum size: " + rectText(QRect({}, widget->maximumSize()))
          << "Size hint: " + rectText(QRect({}, widget->sizeHint()));
    const auto policy = widget->sizePolicy();
    lines << QString("Font: %1; points=%2; pixels=%3")
                 .arg(widget->font().family())
                 .arg(widget->font().pointSizeF())
                 .arg(widget->font().pixelSize());
    lines << QString("Size policy: horizontal=%1 vertical=%2; stretch=%3,%4")
                 .arg(int(policy.horizontalPolicy()))
                 .arg(int(policy.verticalPolicy()))
                 .arg(policy.horizontalStretch())
                 .arg(policy.verticalStretch());
    if (auto *layout = widget->layout())
    {
        const auto margins = layout->contentsMargins();
        lines << QString("Owned layout: %1; spacing=%2; margins=%3,%4,%5,%6; items=%7")
                     .arg(QString::fromLatin1(layout->metaObject()->className()))
                     .arg(layout->spacing())
                     .arg(margins.left())
                     .arg(margins.top())
                     .arg(margins.right())
                     .arg(margins.bottom())
                     .arg(layout->count());
    }
    if (auto *parent = widget->parentWidget(); parent && parent->layout())
    {
        lines << QString("Parent layout: %1; direct item index=%2")
                     .arg(QString::fromLatin1(parent->layout()->metaObject()->className()))
                     .arg(parent->layout()->indexOf(const_cast<QWidget *>(widget)));
    }
    if (auto *screen = widget->screen())
        lines << "Screen available geometry: " + rectText(screen->availableGeometry());
    lines << "Content: editor text, input values, tooltips and dynamic properties intentionally excluded.";
    return lines.join('\n');
}

} // namespace qtinspector

#else // Release mode: devtools are strictly disabled and inaccessible

#include <QWidget>

namespace qtinspector
{

Inspector *Inspector::instance()
{
    return nullptr;
}

Inspector *Inspector::install()
{
    return nullptr;
}

QString Inspector::report(const QWidget *)
{
    return {};
}

void Inspector::setPicking(bool)
{
}

void Inspector::setVisible(bool)
{
}

void Inspector::toggleVisible()
{
}

bool Inspector::eventFilter(QObject *, QEvent *)
{
    return false;
}

Inspector::Inspector(QObject *parent) : QObject(parent)
{
}

} // namespace qtinspector

#endif // !defined(NDEBUG) || defined(QT_DEBUG) || defined(QTAPPBASE_DEBUG)
