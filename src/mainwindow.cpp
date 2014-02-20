#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

MainWindow::MainWindow(const QString &dir, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //QFile stylesheet(":/styles/github.css");
    //stylesheet.open(QFile::ReadOnly);
    //ui->textBrowser->document()->setDefaultStyleSheet(QTextStream(&stylesheet).readAll());

    //baseDirectory = "/home/stepo/Dropbox/notes/notes";
    ui->plainTextEdit->setVisible(false);
    loadDirectory(dir);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::browseDirectory(const QString &path, QList<QTreeWidgetItem *> &result)
{
    qDebug() << "entering" << path;
    /*
        QDir dir(path + QDir::separator() + fInfo.baseName());
        if (dir.exists())
        {
            QList<QTreeWidgetItem *> items;
            browseDirectory(dir.absolutePath(), items);
            item->addChildren(items);
        }*/

    QDir dir(path);
    QFileInfoList directories = dir.entryInfoList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    foreach (const QFileInfo &dirInfo, directories)
    {
        QStringList itemNames; itemNames << dirInfo.fileName();
        QTreeWidgetItem *item = new QTreeWidgetItem(itemNames);
        item->setIcon(0, QIcon::fromTheme("folder"));
        item->setData(0, Qt::UserRole, false);

        QList<QTreeWidgetItem *> items;
        browseDirectory(dirInfo.absoluteFilePath(), items);
        item->addChildren(items);
        result << item;
    }

    QStringList nameFilters; nameFilters << "*.md";
    QFileInfoList files = dir.entryInfoList(nameFilters, QDir::Files, QDir::Name);
    foreach (const QFileInfo &fInfo, files)
    {
        QStringList itemNames; itemNames << fInfo.baseName();
        QTreeWidgetItem *item = new QTreeWidgetItem(itemNames);
        item->setIcon(0, QIcon::fromTheme("document"));
        item->setData(0, Qt::UserRole, true);
        result << item;
    }
}

void MainWindow::loadDirectory(const QString &dir)
{
    baseDirectory = dir;
    currentNote = "";
    ui->treeWidget->clear();
    ui->plainTextEdit->clear();

    QList<QTreeWidgetItem *> items;
    browseDirectory(baseDirectory, items);
    ui->treeWidget->addTopLevelItems(items);
    ui->treeWidget->expandAll();

    this->setWindowTitle(baseDirectory);
    handleButtons();
}

void MainWindow::on_plainTextEdit_textChanged()
{
    currentNoteChanged = true;
    QString html = converter.convert(ui->plainTextEdit->toPlainText());
    ui->textBrowser->setHtml(html);
}

QString MainWindow::createPath(QStringList path)
{
    QString result;
    foreach (const QString &i, path)
    {
        result += QDir::separator() + i;
    }
    return result;
}

QStringList MainWindow::getPath(QTreeWidgetItem *item)
{
    QStringList result;
    QTreeWidgetItem *parent = item->parent();
    while (parent != 0)
    {
        result.prepend(parent->text(0));
        parent = parent->parent();
    }
    return result;
}

void MainWindow::loadNote(const QString &path)
{
    QFile f(path);
    f.open(QFile::ReadOnly);
    QTextStream stream(&f);
    ui->plainTextEdit->setPlainText(stream.readAll());
    currentNote = path;
    currentNoteChanged = false;
}

void MainWindow::loadNote(QTreeWidgetItem *item)
{
    bool isNote = item->data(0, Qt::UserRole).toBool();
    if (!isNote)
    {
        qDebug() << "Directory selected";
        ui->plainTextEdit->clear();
        currentNote.clear();
        currentNoteChanged = false;
        return;
    }

    QStringList parentPath = getPath(item);
    QString fullPath = baseDirectory + createPath(parentPath);
    fullPath += QDir::separator() + item->text(0) + ".md";
    loadNote(fullPath);
}

void MainWindow::handleButtons()
{
    bool enabled = ui->treeWidget->selectedItems().count() > 0;
    ui->plainTextEdit->setEnabled(enabled);

    bool isNoteSelected = enabled && ui->treeWidget->selectedItems().front()->data(0, Qt::UserRole).toBool();
    ui->pbProperties->setEnabled(isNoteSelected);
    ui->pbRemove->setEnabled(isNoteSelected);

    //ui->pbRemoveNote->setEnabled(false /*enabled*/);
}

void MainWindow::on_treeWidget_itemSelectionChanged()
{
    handleButtons();
    saveCurrentNote();
    if (ui->treeWidget->selectedItems().count() != 1) return;

    QTreeWidgetItem *item = ui->treeWidget->selectedItems().front();
    loadNote(item);
}


void MainWindow::on_pbLoad_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select notes directory", baseDirectory);
    if (dir.isEmpty()) return;

    QSettings settings("stepan.mracek", "marknotes");
    settings.setValue("dir", dir);
    loadDirectory(dir);
}

void MainWindow::saveCurrentNote()
{
    if (currentNote.isEmpty()) return;
    if (!currentNoteChanged) return;

    qDebug() << "saving" << currentNote;
    QFile f(currentNote);
    f.open(QFile::WriteOnly);
    QTextStream stream(&f);
    stream << ui->plainTextEdit->toPlainText();
}

QStringList MainWindow::getPathOfSelectedItem()
{
    QStringList path;
    QList<QTreeWidgetItem*> selectedItems = ui->treeWidget->selectedItems();
    if (selectedItems.count() > 0)
    {
        path = getPath(selectedItems.front());
    }
    return path;
}

void MainWindow::on_pbNewNote_clicked()
{
    saveCurrentNote();

    bool ok;
    QString name = QInputDialog::getText(this, tr("Add new note"), tr("Enter new note name"), QLineEdit::Normal, QString(), &ok);
    if (!ok) return;

    QString selectedFolder;
    if (ui->treeWidget->selectedItems().count() > 0)
    {
        QTreeWidgetItem * selItem = ui->treeWidget->selectedItems().front();
        bool folder = !selItem ->data(0, Qt::UserRole).toBool();
        if (folder)
        {
            selectedFolder = QDir::separator() + selItem->text(0);
        }
    }

    QStringList path = getPathOfSelectedItem();
    QString newFilePath = baseDirectory + createPath(path) + selectedFolder + QDir::separator() + name + ".md";

    qDebug() << newFilePath;

    QFile f(newFilePath);
    if (f.exists())
    {
        QMessageBox::information(this, tr("Information"), tr("Note already exists"));
        return;
    }

    f.open(QFile::WriteOnly);
    loadDirectory(baseDirectory);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    saveCurrentNote();
}


void MainWindow::on_pbEditNote_toggled(bool checked)
{
    ui->plainTextEdit->setVisible(checked);
    ui->textBrowser->setVisible(!checked);
}

void MainWindow::on_pbNewFolder_clicked()
{
    saveCurrentNote();

    bool ok;
    QString name = QInputDialog::getText(this, tr("Add new note"), tr("Enter new note name"), QLineEdit::Normal, QString(), &ok);
    if (!ok) return;

    QString selectedFolder;
    if (ui->treeWidget->selectedItems().count() > 0)
    {
        QTreeWidgetItem * selItem = ui->treeWidget->selectedItems().front();
        bool folder = !selItem ->data(0, Qt::UserRole).toBool();
        if (folder)
        {
            selectedFolder = QDir::separator() + selItem->text(0);
        }
    }

    QStringList path = getPathOfSelectedItem();
    QString fullPath = baseDirectory + createPath(path) + selectedFolder + QDir::separator() + name;
    QFileInfo info(fullPath);
    if (info.exists())
    {
        QMessageBox::information(this, tr("Information"), tr("File or folder already exists"));
        return;
    }

    bool success = QDir(QDir::rootPath()).mkpath(fullPath);
    qDebug() << "creating dir" << fullPath << success;

    loadDirectory(baseDirectory);
}

void MainWindow::on_pbProperties_clicked()
{
    saveCurrentNote();

    bool ok;
    QString name = QInputDialog::getText(this, tr("Rename note"), tr("Enter new note name"),
                                         QLineEdit::Normal, QString(), &ok);
    if (!ok) return;

    QFileInfo info(currentNote);
    QString sourceName = currentNote;
    QString destName = info.absolutePath() + QDir::separator() + name;
    if (!destName.toLower().endsWith(".md"))
    {
        destName.append(".md");
    }

    if (QFile(destName).exists())
    {
        QMessageBox::information(this, tr("Information"), tr("File or folder already exists"));
        return;
    }

    QFile::rename(sourceName, destName);
    loadDirectory(baseDirectory);
}

void MainWindow::on_pbRemove_clicked()
{
    int button = QMessageBox::question(this, tr("Question"), tr("Delete current note?"),
                                       QMessageBox::Yes | QMessageBox::No);
    if (button != QMessageBox::Yes) return;

    qDebug() << "removing" << currentNote;
    QFile::remove(currentNote);
    loadDirectory(baseDirectory);
}
