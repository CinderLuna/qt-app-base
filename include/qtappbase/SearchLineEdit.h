// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#ifndef QTAPPBASE_SEARCHLINEEDIT_H
#define QTAPPBASE_SEARCHLINEEDIT_H

#include <QLineEdit>

class QLabel;

namespace qtappbase
{

/// A customized search line edit with a shortcut hint label (e.g. "Ctrl+K")
/// anchored to the right side of the search bar.
class SearchLineEdit : public QLineEdit
{
    Q_OBJECT
  public:
    explicit SearchLineEdit(QWidget *parent = nullptr);
    explicit SearchLineEdit(const QString &placeholder, QWidget *parent = nullptr);

    void setShortcutText(const QString &text);
    QString shortcutText() const;

    QLabel *shortcutLabel() const
    {
        return shortcutLabel_;
    }

  protected:
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;

  private:
    void setupUi();
    void updateShortcutPosition();

    QLabel *shortcutLabel_ = nullptr;
};

} // namespace qtappbase

#endif // QTAPPBASE_SEARCHLINEEDIT_H
