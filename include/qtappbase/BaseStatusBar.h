// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#ifndef QTAPPBASE_BASESTATUSBAR_H
#define QTAPPBASE_BASESTATUSBAR_H

#include <QStatusBar>

class QLabel;
class QPushButton;

namespace qtappbase
{

/// A modern, fully styled status bar that extends QStatusBar with:
/// - Custom DPI-aware 6-dot resize grip painter
/// - Primary elided/stretchable status label (ideal for notifications and progress)
/// - Secondary permanent info/metrics label (ideal for counts, cursor position, coordinates)
/// - Action and Details buttons slots
class BaseStatusBar : public QStatusBar
{
    Q_OBJECT
  public:
    explicit BaseStatusBar(QWidget *parent = nullptr);

    /// Main stretchable status label
    QLabel *statusLabel() const
    {
        return statusLabel_;
    }

    void setStatusText(const QString &text);
    QString statusText() const;

    /// Permanent information label (right-side)
    QLabel *infoLabel() const
    {
        return infoLabel_;
    }

    void setInfoText(const QString &text);
    QString infoText() const;

    /// Primary action button slot (e.g., "Save", "Commit", "Sync")
    void setActionButton(QPushButton *button);
    QPushButton *actionButton() const
    {
        return actionButton_;
    }

    /// Details button slot (e.g., for error details dialog)
    void setDetailsButton(QPushButton *button);
    QPushButton *detailsButton() const
    {
        return detailsButton_;
    }

    /// Convenience wrapper around QStatusBar::showMessage
    void showTemporaryMessage(const QString &text, int timeoutMs = 5000)
    {
        showMessage(text, timeoutMs);
    }

  protected:
    void showEvent(QShowEvent *event) override;

  private:
    void setupUi();

    QLabel *statusLabel_ = nullptr;
    QPushButton *actionButton_ = nullptr;
    QPushButton *detailsButton_ = nullptr;
    QLabel *infoLabel_ = nullptr;
};

} // namespace qtappbase

#endif // QTAPPBASE_BASESTATUSBAR_H
