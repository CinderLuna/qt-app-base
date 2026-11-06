// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#include "qtappbase/BaseWindow.h"
#include "qtappbase/SearchLineEdit.h"
#include "qtappbase/Theme.h"
#include "qtappbase/UiIcons.h"
#include <QApplication>
#include <QFileDialog>
#include <QIcon>
#include <QLineEdit>
#include <QMessageBox>
#include <QShortcut>
#include <QWidget>

using namespace qtappbase;

class TemplateMainWindow : public BaseWindow
{
    Q_OBJECT
  public:
    explicit TemplateMainWindow(QWidget *parent = nullptr) : BaseWindow(parent)
    {
        setWindowTitle("Cinderluna Application");
        setupMenus();
        setupHeaderSearch();
        setupEmptyClientArea();
        setupStatusBar();
    }

  private:
    void setupMenus()
    {
        auto *menuBar = headerMenuBar();

        // --- File Menu ---
        auto *fileMenu = menuBar->addMenu("&File");
        fileMenu->addAction(uiIcon("arrow_redo", ThemeManager::instance().colors().text), "&New",
                            QKeySequence::New, this,
                            [this] { appStatusBar()->showTemporaryMessage("New created", 3000); });
        fileMenu->addAction("&Open…", QKeySequence::Open, this,
                            [this]
                            {
                                const auto file = QFileDialog::getOpenFileName(this, "Open File");
                                if (!file.isEmpty())
                                    appStatusBar()->showTemporaryMessage("Opened: " + file, 4000);
                            });
        fileMenu->addSeparator();
        fileMenu->addAction("&Save", QKeySequence::Save, this,
                            [this] { appStatusBar()->setStatusText("Saved."); });
        fileMenu->addAction("Save &As…", QKeySequence::SaveAs, this, [this]
                            { appStatusBar()->showTemporaryMessage("Save As dialog requested.", 3000); });
        fileMenu->addSeparator();
        fileMenu->addAction("E&xit", QKeySequence::Quit, this, &QWidget::close);

        // --- Edit Menu ---
        auto *editMenu = menuBar->addMenu("&Edit");
        editMenu->addAction(uiIcon("arrow_undo", ThemeManager::instance().colors().text), "&Undo",
                            QKeySequence::Undo, this,
                            [this] { appStatusBar()->showTemporaryMessage("Undo", 2000); });
        editMenu->addAction(uiIcon("arrow_redo", ThemeManager::instance().colors().text), "&Redo",
                            QKeySequence::Redo, this,
                            [this] { appStatusBar()->showTemporaryMessage("Redo", 2000); });

        // --- View Menu ---
        auto *viewMenu = menuBar->addMenu("&View");
        viewMenu->addAction(fullScreenAction());

        // --- Theme & Appearance Menus ---
        auto *themeMenu = menuBar->addMenu("&Theme");
        ThemeManager::populateThemeMenu(themeMenu);

        auto *appearanceMenu = menuBar->addMenu("&Appearance");
        ThemeManager::populateAppearanceMenu(appearanceMenu);

        // --- Help Menu ---
        auto *helpMenu = menuBar->addMenu("&Help");
        helpMenu->addAction(
            "&About Cinderluna App Base…", this,
            [this]
            {
                QMessageBox::about(
                    this, "About Cinderluna Application",
                    "<h3>Cinderluna Qt Application Base</h3>"
                    "<p>The official starting template for Cinderluna desktop applications.</p>"
                    "<ul>"
                    "<li>Native Windows 11 custom title bar with Snap Layouts</li>"
                    "<li>Integrated menu bar and elided window title</li>"
                    "<li>Custom status bar with 6-dot resize grip</li>"
                    "<li>Multi-theme styling engine with automatic dark/light tracking</li>"
                    "</ul>");
            });
        helpMenu->addAction("About &Qt…", qApp, &QApplication::aboutQt);
    }

    void setupHeaderSearch()
    {
        searchEdit_ = new SearchLineEdit("Search…", this);
        searchEdit_->setShortcutText("Ctrl+K");
        searchEdit_->setMinimumWidth(180);
        searchEdit_->setMaximumWidth(280);
        searchEdit_->setToolTip("Project search · Ctrl+K");
        setHeaderSearchWidget(searchEdit_);

        auto *searchShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_K), this);
        connect(searchShortcut, &QShortcut::activated, searchEdit_,
                [this]
                {
                    searchEdit_->setFocus();
                    searchEdit_->selectAll();
                });
    }

    void setupEmptyClientArea()
    {
        // Client area stays completely empty as the clean starting template
        auto *emptyWorkspace = new QWidget(this);
        emptyWorkspace->setObjectName("emptyClientArea");
        setCentralWidget(emptyWorkspace);
    }

    void setupStatusBar()
    {
        appStatusBar()->setStatusText("Ready");
        appStatusBar()->setInfoText("");
    }

    SearchLineEdit *searchEdit_ = nullptr;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("QtAppBase");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("Cinderluna");
    QApplication::setStyle("Fusion");

    // Set application icon from resource
    QApplication::setWindowIcon(QIcon(":/brand/app-icon.svg"));

    TemplateMainWindow window;
    window.show();

    return app.exec();
}

#include "main.moc"
