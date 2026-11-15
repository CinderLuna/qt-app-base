// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#include "qtappbase/BaseWindow.h"
#include "qtappbase/Inspector.h"
#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QShortcut>
#include <QShowEvent>
#include <QStyle>
#include <QWindow>

namespace qtappbase
{

static bool s_autoInstallDevtools = true;

bool BaseWindow::isDevtoolsAvailable()
{
    return qtinspector::Inspector::isAvailable();
}

qtinspector::Inspector *BaseWindow::inspector()
{
    return qtinspector::Inspector::instance();
}

void BaseWindow::setAutoInstallDevtools(bool enabled)
{
    s_autoInstallDevtools = enabled;
}

bool BaseWindow::autoInstallDevtools()
{
    return s_autoInstallDevtools;
}

BaseWindow::BaseWindow(QWidget *parent) : QMainWindow(parent)
{
    extendedHeader_ = WindowHeader::supportsExpandedTitleBar();
#if defined(Q_OS_WIN) && QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    if (extendedHeader_)
    {
        auto flags = windowFlags() | Qt::ExpandedClientAreaHint | Qt::NoTitleBarBackgroundHint |
                     Qt::CustomizeWindowHint | Qt::WindowSystemMenuHint;
        flags &= ~(Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
        flags.setFlag(Qt::WindowTitleHint, false);
        setWindowFlags(flags);

        // The header handles native caption clearance explicitly; do not inset
        // the whole application below the old title-bar safe area as well.
        setAttribute(Qt::WA_ContentsMarginsRespectsSafeArea, false);
    }
#elif (defined(Q_OS_LINUX) || defined(Q_OS_UNIX)) && !defined(Q_OS_DARWIN)
    if (extendedHeader_)
    {
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground, true);
        setMouseTracking(true);
        if (qApp)
            qApp->installEventFilter(this);
    }
#endif

    resize(1200, 800);
    setMinimumSize(640, 480);

    setupBaseUi();

    // Hook up ThemeManager updates
    connect(&ThemeManager::instance(), &ThemeManager::changed, this, &BaseWindow::applyCurrentTheme);
    applyCurrentTheme();
}

BaseWindow::~BaseWindow() = default;

void BaseWindow::setupBaseUi()
{
    // Custom header / title bar
    header_ = new WindowHeader(this);
    auto *top = header_->row();

    // 1. Brand Icon
    brandIconLabel_ = new QLabel(header_);
    brandIconLabel_->setObjectName("brandIcon");
    brandIconLabel_->setFixedSize(24, 24);
    brandIconLabel_->setAttribute(Qt::WA_TransparentForMouseEvents);
    if (!QApplication::windowIcon().isNull())
        brandIconLabel_->setPixmap(QApplication::windowIcon().pixmap(24, 24));
    top->addWidget(brandIconLabel_);
    top->addSpacing(4);

    // 2. Menu Bar
    appMenu_ = new QMenuBar(header_);
    appMenu_->setObjectName("appMenuBar");
    appMenu_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    top->addWidget(appMenu_, 0, Qt::AlignVCenter);

    // 3. Separator
    menuSeparator_ = new QFrame(header_);
    menuSeparator_->setObjectName("titleMenuSeparator");
    menuSeparator_->setFixedSize(1, 20);
    menuSeparator_->setAttribute(Qt::WA_TransparentForMouseEvents);
    top->addSpacing(8);
    top->addWidget(menuSeparator_, 0, Qt::AlignVCenter);
    top->addSpacing(8);

    // 4. Window Title Label
    titleLabel_ = new WindowTitleLabel(header_);
    titleLabel_->setText(windowTitle());
    top->addWidget(titleLabel_, 1);

    // 5. Center/Right Customizable Slots
    headerCenterArea_ = new QWidget(header_);
    headerCenterArea_->setObjectName("headerCenterArea");
    headerCenterArea_->setStyleSheet("background:transparent;");
    auto *centerLayout = new QHBoxLayout(headerCenterArea_);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(4);
    top->addWidget(headerCenterArea_, 0, Qt::AlignVCenter);

    top->addSpacing(12);

    headerRightArea_ = new QWidget(header_);
    headerRightArea_->setObjectName("headerRightArea");
    headerRightArea_->setStyleSheet("background:transparent;");
    auto *rightLayout = new QHBoxLayout(headerRightArea_);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(4);
    top->addWidget(headerRightArea_, 0, Qt::AlignVCenter);

    // 6. Caption Controls (Minimize, Maximize, Close)
    if (extendedHeader_)
        header_->installCaptionControls();

    // Mount header as the window's menu widget
    setMenuWidget(header_);

    // Central workspace container for Linux rounded corner and border framing
    centralContainer_ = new QWidget(this);
    centralContainer_->setObjectName("centralContainer");
    auto *centralLayout = new QVBoxLayout(centralContainer_);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);
    QMainWindow::setCentralWidget(centralContainer_);

    // Custom Status Bar
    statusBar_ = new BaseStatusBar(this);
    setStatusBar(statusBar_);

    // Fullscreen action & shortcut
    fullScreenAction_ = new QAction("Full Screen", this);
    fullScreenAction_->setObjectName("fullScreenAction");
    fullScreenAction_->setCheckable(true);
    fullScreenAction_->setShortcut(QKeySequence(Qt::Key_F11));
    connect(fullScreenAction_, &QAction::triggered, this, &BaseWindow::toggleFullScreen);
    addAction(fullScreenAction_);

    auto *escapeShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escapeShortcut, &QShortcut::activated, this,
            [this]
            {
                if (isFullScreen())
                    toggleFullScreen();
            });

    auto *systemMenuShortcut = new QShortcut(QKeySequence(Qt::ALT | Qt::Key_Space), this);
    connect(systemMenuShortcut, &QShortcut::activated, this,
            [this]
            {
                if (header_)
                    header_->showSystemMenu(false);
            });
}

void BaseWindow::setHeaderSearchWidget(QWidget *searchWidget)
{
    if (searchWidget_ && searchWidget_ != searchWidget)
    {
        headerCenterArea_->layout()->removeWidget(searchWidget_);
        searchWidget_->deleteLater();
    }
    searchWidget_ = searchWidget;
    if (searchWidget_)
    {
        searchWidget_->setObjectName("headerSearchEdit");
        headerCenterArea_->layout()->addWidget(searchWidget_);
    }
}

void BaseWindow::setBrandIcon(const QIcon &icon, const QSize &size)
{
    brandIconLabel_->setFixedSize(size);
    brandIconLabel_->setPixmap(icon.pixmap(size));
    brandIconLabel_->setVisible(!icon.isNull());
}

void BaseWindow::setBrandIconVisible(bool visible)
{
    brandIconLabel_->setVisible(visible);
}

void BaseWindow::setWindowTitle(const QString &title)
{
    QMainWindow::setWindowTitle(title);
    if (titleLabel_)
        titleLabel_->setText(title);
}

void BaseWindow::setCentralWidget(QWidget *widget)
{
    if (!centralContainer_)
    {
        QMainWindow::setCentralWidget(widget);
        return;
    }

    if (auto *layout = centralContainer_->layout())
    {
        while (auto *item = layout->takeAt(0))
        {
            if (item->widget())
                item->widget()->deleteLater();
            delete item;
        }
        if (widget)
            layout->addWidget(widget);
    }
}

QWidget *BaseWindow::centralWidget() const
{
    if (centralContainer_ && centralContainer_->layout() && centralContainer_->layout()->count() > 0)
    {
        auto *item = centralContainer_->layout()->itemAt(0);
        if (item)
            return item->widget();
    }
    return nullptr;
}

void BaseWindow::updateCornerRadii()
{
#if (defined(Q_OS_LINUX) || defined(Q_OS_UNIX)) && !defined(Q_OS_DARWIN)
    const bool restored = extendedHeader_ && !isMaximized() && !isFullScreen();
    if (!property("windowRestored").isValid() || property("windowRestored").toBool() != restored)
    {
        setProperty("windowRestored", restored);
        style()->unpolish(this);
        style()->polish(this);
        if (header_)
        {
            header_->style()->unpolish(header_);
            header_->style()->polish(header_);
        }
        if (statusBar_)
        {
            statusBar_->style()->unpolish(statusBar_);
            statusBar_->style()->polish(statusBar_);
        }
        if (centralContainer_)
        {
            centralContainer_->setProperty("hasStatusBar", statusBar_ && statusBar_->isVisible());
            centralContainer_->style()->unpolish(centralContainer_);
            centralContainer_->style()->polish(centralContainer_);
        }
    }
#endif
}

void BaseWindow::toggleFullScreen()
{
    if (isFullScreen())
    {
        if (savedStateBeforeFullScreen_.testFlag(Qt::WindowMaximized))
            showMaximized();
        else
            showNormal();

        if (fullScreenAction_)
            fullScreenAction_->setChecked(false);
    }
    else
    {
        savedStateBeforeFullScreen_ = windowState();
        showFullScreen();
        if (fullScreenAction_)
            fullScreenAction_->setChecked(true);
    }
    header_->updateCaptionSpace();
}

Qt::Edges BaseWindow::calculateEdges(const QPoint &pos) const
{
    if (isMaximized() || isFullScreen())
        return {};

    const int margin = 6;
    Qt::Edges edges;
    if (pos.x() <= margin)
        edges |= Qt::LeftEdge;
    else if (pos.x() >= width() - margin)
        edges |= Qt::RightEdge;

    if (pos.y() <= margin)
        edges |= Qt::TopEdge;
    else if (pos.y() >= height() - margin)
        edges |= Qt::BottomEdge;

    return edges;
}

void BaseWindow::updateBorderCursor(Qt::Edges edges)
{
    if ((edges.testFlag(Qt::TopEdge) && edges.testFlag(Qt::LeftEdge)) ||
        (edges.testFlag(Qt::BottomEdge) && edges.testFlag(Qt::RightEdge)))
    {
        setCursor(Qt::SizeFDiagCursor);
    }
    else if ((edges.testFlag(Qt::TopEdge) && edges.testFlag(Qt::RightEdge)) ||
             (edges.testFlag(Qt::BottomEdge) && edges.testFlag(Qt::LeftEdge)))
    {
        setCursor(Qt::SizeBDiagCursor);
    }
    else if (edges.testFlag(Qt::TopEdge) || edges.testFlag(Qt::BottomEdge))
    {
        setCursor(Qt::SizeVerCursor);
    }
    else if (edges.testFlag(Qt::LeftEdge) || edges.testFlag(Qt::RightEdge))
    {
        setCursor(Qt::SizeHorCursor);
    }
    else
    {
        unsetCursor();
    }
}

bool BaseWindow::eventFilter(QObject *watched, QEvent *event)
{
#if (defined(Q_OS_LINUX) || defined(Q_OS_UNIX)) && !defined(Q_OS_DARWIN)
    if (extendedHeader_ && !isMaximized() && !isFullScreen())
    {
        auto *widget = qobject_cast<QWidget *>(watched);
        if (widget && widget->window() == this)
        {
            if (event->type() == QEvent::MouseMove)
            {
                auto *mouseEvent = static_cast<QMouseEvent *>(event);
                const QPoint windowPos = mapFromGlobal(mouseEvent->globalPosition().toPoint());
                if (rect().adjusted(-6, -6, 6, 6).contains(windowPos))
                {
                    const auto edges = calculateEdges(windowPos);
                    if (edges != Qt::Edges{})
                        updateBorderCursor(edges);
                    else if (cursor().shape() != Qt::ArrowCursor)
                        unsetCursor();
                }
                else if (cursor().shape() != Qt::ArrowCursor)
                {
                    unsetCursor();
                }
            }
            else if (event->type() == QEvent::MouseButtonPress)
            {
                auto *mouseEvent = static_cast<QMouseEvent *>(event);
                if (mouseEvent->button() == Qt::LeftButton)
                {
                    const QPoint windowPos = mapFromGlobal(mouseEvent->globalPosition().toPoint());
                    const auto edges = calculateEdges(windowPos);
                    if (edges != Qt::Edges{} && windowHandle())
                    {
                        if (windowHandle()->startSystemResize(edges))
                            return true;
                    }
                }
            }
            else if (event->type() == QEvent::Leave && watched == this)
            {
                unsetCursor();
            }
        }
    }
#endif
    return QMainWindow::eventFilter(watched, event);
}

void BaseWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    syncWindowsChrome();
    header_->updateCaptionSpace();
    updateCornerRadii();
#if !defined(NDEBUG) || defined(QT_DEBUG) || defined(QTAPPBASE_DEBUG)
    if (s_autoInstallDevtools)
    {
        qtinspector::Inspector::install();
    }
#endif
}

void BaseWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange)
    {
        header_->updateCaptionSpace();
        updateCornerRadii();
        if (fullScreenAction_)
            fullScreenAction_->setChecked(isFullScreen());
    }
}

void BaseWindow::applyCurrentTheme()
{
    const auto colors = ThemeManager::instance().colors();
    setPalette(colors.palette());
    setStyleSheet(colors.stylesheet());
    updateCornerRadii();
    onThemeChanged(colors);
}

void BaseWindow::onThemeChanged(const ThemeColors &)
{
    // Default implementation does nothing; subclasses can override
}

} // namespace qtappbase
