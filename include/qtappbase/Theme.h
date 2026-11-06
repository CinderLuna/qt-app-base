// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#ifndef QTAPPBASE_THEME_H
#define QTAPPBASE_THEME_H

#include <QColor>
#include <QList>
#include <QObject>
#include <QPair>
#include <QPalette>
#include <QString>

class QMenu;

namespace qtappbase
{

enum class ColorTheme
{
    Slate,
    Olive,
    Forest,
    Parchment,
    Aubergine,
    Rosewood
};

enum class Appearance
{
    System,
    Light,
    Dark
};

struct ThemeColors
{
    QColor background;
    QColor panel;
    QColor chrome;
    QColor text;
    QColor muted;
    QColor border;
    QColor accent;
    QColor selection;
    QColor selectedText;

    QPalette palette() const;
    QString stylesheet() const;
};

ThemeColors themeColors(ColorTheme theme, bool dark);
QList<QPair<QString, ColorTheme>> availableThemes();

class ThemeManager : public QObject
{
    Q_OBJECT
  public:
    static ThemeManager &instance();

    ColorTheme theme() const
    {
        return theme_;
    }

    Appearance appearance() const
    {
        return appearance_;
    }

    bool isDark() const
    {
        return dark_;
    }

    ThemeColors colors() const
    {
        return themeColors(theme_, dark_);
    }

    bool setPreferences(ColorTheme theme, Appearance appearance);
    void reload();

    /// Convenience helpers to build standard Theme and Appearance menus.
    static void populateThemeMenu(QMenu *menu);
    static void populateAppearanceMenu(QMenu *menu);

  signals:
    void changed();

  private:
    explicit ThemeManager(QObject *parent = nullptr);
    void refresh();

    ColorTheme theme_ = ColorTheme::Slate;
    Appearance appearance_ = Appearance::System;
    bool dark_ = false;
    bool refreshing_ = false;
};

} // namespace qtappbase

#endif // QTAPPBASE_THEME_H
