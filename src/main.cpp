#include "mainwindow.h"
#include <QApplication>
#include <QSettings>
#include <QDir>
#include <QFileDialog>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSettings settings("stepan.mracek", "marknotes");
    if (!settings.contains("dir"))
    {
        QString dir = QFileDialog::getExistingDirectory(0, "Select notes directory", QDir::homePath());
        if (dir.isEmpty()) return 1;

        settings.setValue("dir", dir);
    }

    QString dir = settings.value("dir").toString();
    MainWindow w(dir);
    w.show();

    return a.exec();
}
