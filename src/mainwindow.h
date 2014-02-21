#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>

#include "discountconverter.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QString baseDirectory;
    QString currentNote;
    bool currentNoteChanged;
    DiscountConverter converter;

public:
    explicit MainWindow(const QString &dir, QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_plainTextEdit_textChanged();

    void on_treeWidget_itemSelectionChanged();

    void on_pbLoad_clicked();

    void on_pbNewNote_clicked();

    void on_pbEditNote_toggled(bool checked);

    void on_pbNewFolder_clicked();

    void on_pbProperties_clicked();

    void on_pbRemove_clicked();

private:
    Ui::MainWindow *ui;

    void browseDirectory(const QString &path, QList<QTreeWidgetItem *> &result);
    void loadDirectory(const QString &dir);
    QStringList getPath(QTreeWidgetItem *item);
    QString createPath(QStringList path);
    void loadNote(const QString &path);
    void loadNote(QTreeWidgetItem *item);
    void saveCurrentNote();
    QStringList getPathOfSelectedItem();
    void handleButtons();
    void closeEvent(QCloseEvent *);
    void saveSettings();
    bool isSomeItemSelected();
    bool isNoteSelected();
    QTreeWidgetItem *selectedItem();
    bool renameCurrentNote();
    bool renameCurrentFolder();
    bool removeDirectory(const QString &path);
};

#endif // MAINWINDOW_H
