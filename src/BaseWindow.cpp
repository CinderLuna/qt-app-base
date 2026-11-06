// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#include "qtappbase/BaseWindow.h"
#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QShortcut>
#include <QShowEvent>

namespace qtappbase
{

BaseWindow::BaseWindow(QWidget *parent) : QMainWindow(parent)
{
#if defined(Q_OS_WIN) && QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    extendedHeader_ = WindowHeader::supportsExpandedTitleBar();
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

void BaseWindow::toggleFullScreen()
{
    if (isFullScreen())
    {
        showNormal();
        if (fullScreenAction_)
            fullScreenAction_->setChecked(false);
    }
    else
    {
        showFullScreen();
        if (fullScreenAction_)
            fullScreenAction_->setChecked(true);
    }
    header_->updateCaptionSpace();
}

void BaseWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    syncWindowsChrome();
    header_->updateCaptionSpace();
}

void BaseWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange)
    {
        header_->updateCaptionSpace();
        if (fullScreenAction_)
            fullScreenAction_->setChecked(isFullScreen());
    }
}

void BaseWindow::applyCurrentTheme()
{
    const auto colors = ThemeManager::instance().colors();
    setPalette(colors.palette());
    setStyleSheet(colors.stylesheet());
    onThemeChanged(colors);
}

void BaseWindow::onThemeChanged(const ThemeColors &)
{
    // Default implementation does nothing; subclasses can override
}

} // namespace qtappbase
