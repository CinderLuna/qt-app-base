// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#include "qtappbase/BaseWindow.h"
#include "qtappbase/Inspector.h"
#include <QApplication>
#include <QClipboard>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSignalSpy>
#include <QTextEdit>
#include <QToolButton>
#include <QVBoxLayout>
#include <QtTest>

using namespace qtappbase;

class InspectorTests : public QObject
{
    Q_OBJECT
  private slots:
    void testAccessibilityGating();
    void reportHasLocatorAndLayoutButNoInputContent();
    void clickCopiesWithoutActivatingAndEscapeRestoresNormalInput();
    void modalDialogsAndDestroyedSelectionsAreHandled();
    void hideAndShortcutReopenDoNotLeaveSelectionEnabled();
    void itemViewClicksIdentifyRowWithoutCopyingItsContents();
};

void InspectorTests::testAccessibilityGating()
{
    const bool available = qtinspector::Inspector::isAvailable();
    QCOMPARE(available, BaseWindow::isDevtoolsAvailable());

#if defined(NDEBUG) && !defined(QT_DEBUG) && !defined(QTAPPBASE_DEBUG)
    // In Release mode: devtools must be strictly disabled and inaccessible
    QVERIFY2(!available, "Devtools must not be available in release builds");
    QCOMPARE(qtinspector::Inspector::install(), nullptr);
    QCOMPARE(qtinspector::Inspector::instance(), nullptr);
    QCOMPARE(BaseWindow::inspector(), nullptr);
    QWidget dummy;
    QVERIFY(qtinspector::Inspector::report(&dummy).isEmpty());
#else
    // In Debug mode: devtools must be available and operational
    QVERIFY2(available, "Devtools must be available in debug builds");
    auto *inspector = qtinspector::Inspector::install();
    QVERIFY2(inspector != nullptr, "Inspector::install() should return controller in debug mode");
    QCOMPARE(qtinspector::Inspector::instance(), inspector);
    QCOMPARE(BaseWindow::inspector(), inspector);
#endif
}

void InspectorTests::reportHasLocatorAndLayoutButNoInputContent()
{
    if (!qtinspector::Inspector::isAvailable())
    {
        QSKIP("Devtools are disabled in release mode - skipping interactive inspection test.");
    }

    QWidget root;
    root.setObjectName("testWindow");
    auto *layout = new QVBoxLayout(&root);
    auto *editor = new QTextEdit;
    editor->setObjectName("testEditor");
    editor->setPlainText("PRIVATE_MANUSCRIPT_TEXT");
    editor->setToolTip("PRIVATE_TOOLTIP");
    editor->setProperty("secret", "PRIVATE_PROPERTY");
    layout->addWidget(editor);

    const auto report = qtinspector::Inspector::report(editor);
    QVERIFY(report.contains("QWidget#testWindow / QTextEdit#testEditor"));
    QVERIFY(report.contains("Parent layout: QVBoxLayout"));
    QVERIFY(report.contains("findChild<QTextEdit *>"));
    QVERIFY(!report.contains("PRIVATE_"));

    QLineEdit input("PRIVATE_INPUT");
    QVERIFY(!qtinspector::Inspector::report(&input).contains("PRIVATE_INPUT"));
}

void InspectorTests::clickCopiesWithoutActivatingAndEscapeRestoresNormalInput()
{
    if (!qtinspector::Inspector::isAvailable())
    {
        QSKIP("Devtools are disabled in release mode - skipping interactive inspection test.");
    }

    QWidget root;
    root.resize(800, 500);
    root.setObjectName("inspectionWindow");
    auto *layout = new QVBoxLayout(&root);
    auto *button = new QPushButton("Test button");
    button->setObjectName("testButton");
    layout->addWidget(button);
    layout->addStretch();

    QSignalSpy activated(button, &QPushButton::clicked);
    auto *inspector = qtinspector::Inspector::install();
    QCOMPARE(inspector, qtinspector::Inspector::install());

    root.show();
    QTRY_VERIFY(root.findChild<QWidget *>("qtWidgetInspectorBar"));

    const bool tracking = button->hasMouseTracking();
    inspector->setPicking(true);
    QTest::mouseClick(button, Qt::LeftButton);
    QCOMPARE(activated.count(), 0);
    QCOMPARE(inspector->selectedWidget(), button);
    QVERIFY(QApplication::clipboard()->text().contains("Object name: testButton"));

    root.findChild<QToolButton *>("inspectorParent")->click();
    QCOMPARE(inspector->selectedWidget(), &root);
    QVERIFY(QApplication::clipboard()->text().contains("Object name: inspectionWindow"));

    QTest::keyClick(button, Qt::Key_Escape);
    QVERIFY(!inspector->isPicking());
    QCOMPARE(button->hasMouseTracking(), tracking);

    QTest::mouseClick(button, Qt::LeftButton);
    QCOMPARE(activated.count(), 1);
}

void InspectorTests::modalDialogsAndDestroyedSelectionsAreHandled()
{
    if (!qtinspector::Inspector::isAvailable())
    {
        QSKIP("Devtools are disabled in release mode - skipping interactive inspection test.");
    }

    QWidget root;
    root.resize(800, 500);
    root.show();

    auto *inspector = qtinspector::Inspector::install();
    QDialog dialog(&root);
    dialog.resize(600, 300);
    dialog.setModal(true);
    auto *layout = new QVBoxLayout(&dialog);
    auto *button = new QPushButton("Transient");
    layout->addWidget(button);
    layout->addStretch();
    dialog.show();

    QTRY_VERIFY(dialog.findChild<QWidget *>("qtWidgetInspectorBar"));

    inspector->setPicking(true);
    QTest::mouseClick(button, Qt::LeftButton);
    QCOMPARE(inspector->selectedWidget(), button);

    delete button;
    QVERIFY(!inspector->selectedWidget());

    dialog.findChild<QToolButton *>("inspectorCopy")->click();
    inspector->setPicking(false);
    dialog.close();
    QCoreApplication::processEvents();
}

void InspectorTests::hideAndShortcutReopenDoNotLeaveSelectionEnabled()
{
    if (!qtinspector::Inspector::isAvailable())
    {
        QSKIP("Devtools are disabled in release mode - skipping interactive inspection test.");
    }

    QWidget root;
    root.resize(800, 500);
    root.show();

    auto *inspector = qtinspector::Inspector::install();
    QTRY_VERIFY(root.findChild<QWidget *>("qtWidgetInspectorBar"));

    inspector->setPicking(true);
    root.findChild<QToolButton *>("inspectorHide")->click();
    QVERIFY(!inspector->isPicking());
    QVERIFY(!root.findChild<QWidget *>("qtWidgetInspectorBar")->isVisible());

    QTest::keyClick(&root, Qt::Key_F12, Qt::ControlModifier | Qt::AltModifier);
    QVERIFY(root.findChild<QWidget *>("qtWidgetInspectorBar")->isVisible());

    QTest::keyClick(&root, Qt::Key_F8);
    QVERIFY(inspector->isPicking());

    QTest::keyClick(&root, Qt::Key_Escape);
    QVERIFY(!inspector->isPicking());
}

void InspectorTests::itemViewClicksIdentifyRowWithoutCopyingItsContents()
{
    if (!qtinspector::Inspector::isAvailable())
    {
        QSKIP("Devtools are disabled in release mode - skipping interactive inspection test.");
    }

    QWidget root;
    root.resize(800, 500);
    auto *layout = new QVBoxLayout(&root);
    auto *list = new QListWidget;
    list->setObjectName("projectList");
    list->addItems({"PRIVATE_FIRST_PROJECT", "PRIVATE_SECOND_PROJECT"});
    layout->addWidget(list);
    root.show();

    auto *inspector = qtinspector::Inspector::install();
    QTRY_VERIFY(root.findChild<QWidget *>("qtWidgetInspectorBar"));

    inspector->setPicking(true);
    const auto point = list->visualItemRect(list->item(1)).center();
    QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier, point);

    const auto report = QApplication::clipboard()->text();
    QVERIFY(report.contains("QListWidget#projectList"));
    QVERIFY(report.contains("row path=1; column=0"));
    QVERIFY(!report.contains("PRIVATE_"));

    inspector->setPicking(false);
}

QTEST_MAIN(InspectorTests)
#include "inspector_tests.moc"
