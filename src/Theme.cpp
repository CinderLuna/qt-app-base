// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#include "qtappbase/Theme.h"
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QMenu>
#include <QScopedValueRollback>
#include <QSettings>
#include <QStyleHints>
#include <utility>

namespace qtappbase
{

QList<QPair<QString, ColorTheme>> availableThemes()
{
    return {{"Slate", ColorTheme::Slate},         {"Olive", ColorTheme::Olive},
            {"Forest", ColorTheme::Forest},       {"Parchment", ColorTheme::Parchment},
            {"Aubergine", ColorTheme::Aubergine}, {"Rosewood", ColorTheme::Rosewood}};
}

ThemeColors themeColors(ColorTheme theme, bool dark)
{
    ThemeColors c;
    // Default Fallback (Slate)
    c.background = dark ? QColor("#222830") : QColor("#F8FAFD");
    c.panel = dark ? QColor("#1E232B") : QColor("#EDF1F7");
    c.chrome = dark ? QColor("#191E25") : QColor("#E5EBF3");
    c.text = dark ? QColor("#E0E7EF") : QColor("#222E40");
    c.muted = dark ? QColor("#ACB9C9") : QColor("#54657B");
    c.border = dark ? QColor("#435060") : QColor("#BFCCDD");
    c.accent = dark ? QColor("#9BBDE8") : QColor("#315D91");
    c.selection = dark ? QColor("#30435D") : QColor("#D9E5F5");
    c.selectedText = dark ? QColor("#EDF4FF") : QColor("#25496F");

    switch (theme)
    {
    case ColorTheme::Olive:
        c.background = dark ? QColor("#242526") : QColor("#FCFBF8");
        c.panel = dark ? QColor("#202122") : QColor("#F3F1EB");
        c.chrome = dark ? QColor("#1B1C1D") : QColor("#F4F2ED");
        c.text = dark ? QColor("#E5E4DE") : QColor("#202822");
        c.muted = dark ? QColor("#B0B0A7") : QColor("#60645B");
        c.border = dark ? QColor("#444642") : QColor("#C9CCC2");
        c.accent = dark ? QColor("#B4BF84") : QColor("#52652C");
        c.selection = dark ? QColor("#3D4630") : QColor("#E0E6CC");
        c.selectedText = dark ? QColor("#F1F1E8") : c.accent;
        break;

    case ColorTheme::Forest:
        c.background = dark ? QColor("#212624") : QColor("#F7FAF8");
        c.panel = dark ? QColor("#1C211F") : QColor("#ECF2EE");
        c.chrome = dark ? QColor("#171C1A") : QColor("#E3ECE6");
        c.text = dark ? QColor("#DFEAE3") : QColor("#1D2B22");
        c.muted = dark ? QColor("#A5B7AC") : QColor("#566A5D");
        c.border = dark ? QColor("#3E4D43") : QColor("#BCD0C3");
        c.accent = dark ? QColor("#9DC4AD") : QColor("#174D40");
        c.selection = dark ? QColor("#304A3B") : QColor("#DFE7DE");
        c.selectedText = dark ? QColor("#F0F8F3") : c.accent;
        break;

    case ColorTheme::Parchment:
        c = dark ? ThemeColors{"#29251F", "#24211C", "#1E1B17", "#EDE3D1", "#BEB09A",
                               "#514A3E", "#D6B775", "#4B402C", "#F7ECD8"}
                 : ThemeColors{"#FBF4E6", "#F1E8D7", "#EDE2CE", "#332B20", "#71604A",
                               "#CDBE9F", "#76551D", "#E8D9B7", "#503B16"};
        break;

    case ColorTheme::Slate:
        // already default
        break;

    case ColorTheme::Aubergine:
        c = dark ? ThemeColors{"#2B242E", "#251F28", "#201A23", "#ECE1EE", "#C0ADC4",
                               "#55435D", "#CEABDF", "#4A3555", "#F6EAFB"}
                 : ThemeColors{"#FCF9FD", "#F1EBF5", "#EAE1F0", "#34243E", "#715D7C",
                               "#D2BFD9", "#78508E", "#E9DAF0", "#5C356F"};
        break;

    case ColorTheme::Rosewood:
        c = dark ? ThemeColors{"#2D2525", "#271F20", "#211A1B", "#EFE2DF", "#C5AFAC",
                               "#5B4445", "#DCA7AA", "#513638", "#FBECE8"}
                 : ThemeColors{"#FCF8F5", "#F3EBE7", "#EEE2DE", "#3D292B", "#795F61",
                               "#D5BFBB", "#914F58", "#EEDADD", "#703C44"};
        break;
    }

    return c;
}

QPalette ThemeColors::palette() const
{
    QPalette p;
    p.setColor(QPalette::Window, panel);
    p.setColor(QPalette::WindowText, text);
    p.setColor(QPalette::Base, background);
    p.setColor(QPalette::AlternateBase, panel);
    p.setColor(QPalette::Text, text);
    p.setColor(QPalette::Button, panel);
    p.setColor(QPalette::ButtonText, text);
    p.setColor(QPalette::Highlight, selection);
    p.setColor(QPalette::HighlightedText, selectedText);
    p.setColor(QPalette::PlaceholderText, muted);
    p.setColor(QPalette::Link, accent);
    p.setColor(QPalette::ToolTipBase, panel);
    p.setColor(QPalette::ToolTipText, text);
    p.setColor(QPalette::Disabled, QPalette::Text, muted);
    p.setColor(QPalette::Disabled, QPalette::ButtonText, muted);
    return p;
}

QString ThemeColors::stylesheet() const
{
    QString css = R"(
      QMainWindow, QWidget { background:$background; color:$text; font-family:'Segoe UI', sans-serif; font-size:14px; }
      
      /* --- Custom Title Bar (Header) --- */
      QWidget#applicationHeader { background:$chrome; }
      QWidget#captionControls { background:transparent; }
      QWidget#captionControls QToolButton {
          background:transparent; color:$text; border:0; border-radius:0;
          padding:0; font-family:'Segoe Fluent Icons','Segoe MDL2 Assets'; font-size:10px;
      }
      QWidget#captionControls QToolButton:hover { background:$selection; }
      QWidget#captionControls QToolButton#captionClose:hover { background:#C42B1C; color:white; }
      
      QLabel#brandIcon { background:transparent; }
      QLabel#windowTitleLabel { font-size:14px; color:$muted; background:transparent; font-weight:500; }
      QFrame#titleMenuSeparator { background:$border; border:0; }

      /* --- Menu Bar & Menus --- */
      QMenuBar { background:$chrome; border:0; }
      QMenuBar::item { padding:6px 10px; background:transparent; border-radius:3px; }
      QMenuBar::item:selected { background:$selection; color:$selectedText; }
      QMenu { background:$chrome; border:1px solid $border; padding:4px; }
      QMenu::item { padding:6px 20px 6px 12px; border-radius:3px; }
      QMenu::item:selected { background:$selection; color:$selectedText; }
      QMenu::separator { height:1px; background:$border; margin:4px 6px; }

      /* --- Status Bar --- */
      QStatusBar { padding:2px 8px; background:$chrome; border-top:1px solid $border; }
      QStatusBar::item { border:0; }
      QStatusBar QWidget { background:transparent; }
      QStatusBar QLabel { color:$muted; font-size:12px; }
      QStatusBar QLabel#statusBarMainText { color:$text; font-size:13px; }
      QStatusBar QLabel#statusBarInfoText { color:$muted; font-size:12px; padding:0 6px; }
      QStatusBar QPushButton {
          background:transparent; color:$text; border:1px solid transparent;
          border-radius:3px; padding:2px 8px; margin:0 2px; font-size:12px;
      }
      QStatusBar QPushButton:hover { background:$selection; }
      QStatusBar QPushButton:focus { border:1px solid $accent; }
      QSizeGrip#statusResizeGrip { background:$chrome; width:16px; height:16px; }

      /* --- Standard Controls & Forms --- */
      QPushButton {
          background:$panel; color:$text; border:1px solid $border;
          padding:6px 14px; border-radius:4px; font-weight:500;
      }
      QPushButton:hover { background:$selection; }
      QPushButton:focus { border:1px solid $accent; }
      QPushButton#primaryButton {
          background:$selection; color:$selectedText; border:1px solid $accent;
      }

      QToolButton {
          background:transparent; color:$text; padding:5px 8px;
          border:1px solid transparent; border-radius:3px;
      }
      QToolButton:hover { background:$selection; }
      QToolButton:checked { background:$selection; border:1px solid $accent; color:$selectedText; }
      QToolButton:focus { border:1px solid $accent; }
      QToolButton:disabled, QMenu::item:disabled { color:$muted; }

      QLineEdit, QPlainTextEdit, QTextEdit, QComboBox, QSpinBox {
          background:$background; color:$text; border:1px solid $border;
          border-radius:3px; padding:6px 8px;
          selection-background-color:$selection; selection-color:$selectedText;
      }
      QLineEdit:focus, QPlainTextEdit:focus, QTextEdit:focus, QComboBox:focus, QSpinBox:focus {
          border:1px solid $accent;
      }
      QLineEdit#headerSearchEdit {
          background:$panel; border-radius:4px; padding:5px 10px;
      }
      QLabel#searchShortcut {
          background:transparent; color:$muted; font-size:11px; font-weight:500;
          padding:1px 4px; border:1px solid $border; border-radius:3px;
      }

      /* --- Navigation, Tabs & Views --- */
      QTabBar::tab {
          padding:8px 16px; background:$chrome; border-radius:3px;
          margin-right:2px; border-bottom:2px solid transparent;
      }
      QTabBar::tab:selected { color:$accent; background:$background; border-bottom:2px solid $accent; }
      QTabWidget::pane { border:1px solid $border; background:$background; }

      QTreeWidget, QListWidget, QTableView {
          background:$background; border:1px solid $border; outline:0;
      }
      QTreeWidget::item, QListWidget::item { padding:6px 8px; }
      QTreeWidget::item:selected, QListWidget::item:selected {
          background:$selection; color:$selectedText;
      }
      QHeaderView::section {
          background:$chrome; padding:6px 10px; border:0; border-bottom:1px solid $border;
      }

      QSplitter::handle { background:$border; width:1px; height:1px; }
      QToolTip { background:$panel; color:$text; border:1px solid $border; padding:4px 8px; }
    )";

    const QList<QPair<QString, QColor>> values{{"selectedText", selectedText},
                                               {"background", background},
                                               {"selection", selection},
                                               {"chrome", chrome},
                                               {"panel", panel},
                                               {"border", border},
                                               {"accent", accent},
                                               {"muted", muted},
                                               {"text", text}};

    for (const auto &value : std::as_const(values))
        css.replace("$" + value.first, value.second.name());

    return css;
}

ThemeManager &ThemeManager::instance()
{
    static auto *manager = new ThemeManager(qApp);
    return *manager;
}

ThemeManager::ThemeManager(QObject *parent) : QObject(parent)
{
    if (QGuiApplication::styleHints())
    {
        connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this,
                [this]
                {
                    if (appearance_ == Appearance::System && !refreshing_)
                        refresh();
                });
    }
    reload();
}

void ThemeManager::reload()
{
    QSettings settings;
    theme_ = ColorTheme::Slate;
    const auto savedTheme = settings.value("ui/theme", "slate").toString();
    for (const auto &entry : availableThemes())
    {
        if (entry.first.compare(savedTheme, Qt::CaseInsensitive) == 0)
            theme_ = entry.second;
    }

    const auto appearance = settings.value("ui/appearance", "system").toString();
    appearance_ = appearance == "dark"    ? Appearance::Dark
                  : appearance == "light" ? Appearance::Light
                                          : Appearance::System;
    refresh();
}

bool ThemeManager::setPreferences(ColorTheme theme, Appearance appearance)
{
    theme_ = theme;
    appearance_ = appearance;
    QSettings settings;

    for (const auto &entry : availableThemes())
    {
        if (entry.second == theme)
            settings.setValue("ui/theme", entry.first.toLower());
    }

    settings.setValue("ui/appearance", appearance == Appearance::Dark    ? "dark"
                                       : appearance == Appearance::Light ? "light"
                                                                         : "system");
    settings.sync();
    refresh();
    return settings.status() == QSettings::NoError;
}

void ThemeManager::refresh()
{
    if (refreshing_)
        return;

    const QScopedValueRollback<bool> guard(refreshing_, true);
    auto *hints = QGuiApplication::styleHints();
    if (hints)
    {
        if (appearance_ == Appearance::System)
            hints->unsetColorScheme();
        else
            hints->setColorScheme(appearance_ == Appearance::Dark ? Qt::ColorScheme::Dark
                                                                  : Qt::ColorScheme::Light);

        dark_ = appearance_ == Appearance::Dark ||
                (appearance_ == Appearance::System && hints->colorScheme() == Qt::ColorScheme::Dark);
    }
    else
    {
        dark_ = (appearance_ == Appearance::Dark);
    }

    QApplication::setPalette(colors().palette());
    emit changed();
}

void ThemeManager::populateThemeMenu(QMenu *menu)
{
    if (!menu)
        return;

    auto *group = new QActionGroup(menu);
    auto &manager = ThemeManager::instance();

    for (const auto &entry : availableThemes())
    {
        auto *action = menu->addAction(entry.first);
        action->setObjectName("themeAction_" + entry.first);
        action->setCheckable(true);
        action->setChecked(manager.theme() == entry.second);
        group->addAction(action);

        QObject::connect(action, &QAction::triggered,
                         [theme = entry.second]
                         {
                             auto &mgr = ThemeManager::instance();
                             mgr.setPreferences(theme, mgr.appearance());
                         });
    }

    QObject::connect(&manager, &ThemeManager::changed, menu,
                     [group]
                     {
                         const auto currentTheme = ThemeManager::instance().theme();
                         for (auto *action : group->actions())
                         {
                             for (const auto &entry : availableThemes())
                             {
                                 if (action->objectName() == ("themeAction_" + entry.first))
                                     action->setChecked(entry.second == currentTheme);
                             }
                         }
                     });
}

void ThemeManager::populateAppearanceMenu(QMenu *menu)
{
    if (!menu)
        return;

    auto *group = new QActionGroup(menu);
    auto &manager = ThemeManager::instance();

    const QList<QPair<QString, Appearance>> appearances = {
        {"Follow System", Appearance::System}, {"Light", Appearance::Light}, {"Dark", Appearance::Dark}};

    for (const auto &entry : std::as_const(appearances))
    {
        auto *action = menu->addAction(entry.first);
        action->setObjectName("appearanceAction_" + entry.first);
        action->setCheckable(true);
        action->setChecked(manager.appearance() == entry.second);
        group->addAction(action);

        QObject::connect(action, &QAction::triggered,
                         [appearance = entry.second]
                         {
                             auto &mgr = ThemeManager::instance();
                             mgr.setPreferences(mgr.theme(), appearance);
                         });
    }

    QObject::connect(&manager, &ThemeManager::changed, menu,
                     [group, appearances]
                     {
                         const auto currentApp = ThemeManager::instance().appearance();
                         for (auto *action : group->actions())
                         {
                             for (const auto &entry : appearances)
                             {
                                 if (action->objectName() == ("appearanceAction_" + entry.first))
                                     action->setChecked(entry.second == currentApp);
                             }
                         }
                     });
}

} // namespace qtappbase
