// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#ifndef QTAPPBASE_INSPECTOR_H
#define QTAPPBASE_INSPECTOR_H

#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVector>
#include <QWidget>

class QLabel;
class QToolButton;

namespace qtinspector
{

/// In-process developer tool for inspecting Qt Widget hierarchies, geometries, layouts, and size policies.
/// Only accessible in debug mode. In release builds, devtools are strictly disabled.
class Inspector final : public QObject
{
  public:
    /// Returns true if the application is running in debug mode with devtools enabled.
    static constexpr bool isAvailable()
    {
#if !defined(NDEBUG) || defined(QT_DEBUG) || defined(QTAPPBASE_DEBUG)
        return true;
#else
        return false;
#endif
    }

    /// Installs the widget inspector on QApplication.
    /// Only accessible in debug mode. In release builds, this is a no-op returning nullptr.
    static Inspector *install();

    /// Returns the active inspector instance if already installed, or nullptr.
    static Inspector *instance();

    /// Generates an inspection report for the given widget without exposing private input content.
    /// In release builds, returns an empty string.
    static QString report(const QWidget *widget);

    /// Toggles or sets widget picking mode.
    void setPicking(bool enabled);
    bool isPicking() const
    {
        return picking_;
    }

    /// Returns the currently selected widget, or nullptr.
    QWidget *selectedWidget() const
    {
        return selected_;
    }

    /// Controls visibility of the floating inspector toolbar.
    void setVisible(bool visible);
    void toggleVisible();
    bool isVisible() const
    {
        return visible_;
    }

  protected:
    bool eventFilter(QObject *object, QEvent *event) override;

  private:
    explicit Inspector(QObject *parent);
    void attach(QWidget *window);
    void highlight(QWidget *widget);
    void copySelection();
    void track(QWidget *widget);
    bool isInternal(const QWidget *widget) const;

    struct Tracking
    {
        QPointer<QWidget> widget;
        bool mouseTracking;
    };
    QVector<Tracking> tracking_;
    QPointer<QWidget> bar_, outline_, selected_, hovered_;
    QPointer<QToolButton> selectButton_;
    QPointer<QLabel> status_;
    QString selectedItem_;
    bool picking_ = false, visible_ = true;
};

} // namespace qtinspector

namespace qtappbase
{
namespace devtools = qtinspector;
using Inspector = qtinspector::Inspector;
} // namespace qtappbase

#endif // QTAPPBASE_INSPECTOR_H
