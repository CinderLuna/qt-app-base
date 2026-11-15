#include "Inspector.h"
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("Qt Widget Inspector Demo");
    QWidget window;
    window.setObjectName("demoWindow");
    window.setWindowTitle("Qt Widget Inspector — demo");
    window.resize(800, 520);
    auto *layout = new QVBoxLayout(&window);
    layout->addWidget(new QLabel("F8 selects widgets. Click to copy a report. Escape exits selection."));
    auto *name = new QLineEdit;
    name->setObjectName("projectName");
    name->setPlaceholderText("An input to inspect");
    layout->addWidget(name);
    auto *editor = new QTextEdit;
    editor->setObjectName("writingEditor");
    editor->setPlaceholderText("Editor text is excluded from inspector reports.");
    layout->addWidget(editor);
    auto *dialog = new QPushButton("Open a modal dialog");
    dialog->setObjectName("openDialog");
    layout->addWidget(dialog);
    QObject::connect(dialog, &QPushButton::clicked, &window,
                     [&]
                     {
                         QDialog child(&window);
                         child.setObjectName("demoDialog");
                         child.resize(600, 260);
                         auto *content = new QVBoxLayout(&child);
                         content->addWidget(new QLabel("The inspector also attaches to modal dialogs."));
                         auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
                         content->addWidget(buttons);
                         QObject::connect(buttons, &QDialogButtonBox::rejected, &child, &QDialog::reject);
                         child.exec();
                     });
    qtinspector::Inspector::install();
    window.show();
    return app.exec();
}
