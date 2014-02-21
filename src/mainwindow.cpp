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

    ui->plainTextEdit->setVisible(false);
    loadDirectory(dir);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::browseDirectory(const QString &path, QList<QTreeWidgetItem *> &result)
{
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
    if (!isNoteSelected())
    {
        ui->plainTextEdit->clear();
        ui->plainTextEdit->setEnabled(false);
        currentNote.clear();
        currentNoteChanged = false;
        return;
    }

    ui->plainTextEdit->setEnabled(true);
    QStringList parentPath = getPath(item);
    QString fullPath = baseDirectory + createPath(parentPath);
    fullPath += QDir::separator() + item->text(0) + ".md";
    loadNote(fullPath);
}

void MainWindow::handleButtons()
{
    bool enabled = isSomeItemSelected();
    ui->plainTextEdit->setEnabled(enabled);
    ui->pbProperties->setEnabled(enabled);
    ui->pbRemove->setEnabled(enabled);
}

void MainWindow::on_treeWidget_itemSelectionChanged()
{
    handleButtons();
    saveCurrentNote();
    if (!isSomeItemSelected()) return;
    QTreeWidgetItem *item = ui->treeWidget->selectedItems().front();
    loadNote(item);
}

void MainWindow::saveSettings()
{
    QSettings settings("stepan.mracek", "marknotes");
    settings.setValue("dir", baseDirectory);
}

void MainWindow::on_pbLoad_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select notes directory", baseDirectory);
    if (dir.isEmpty()) return;

    saveCurrentNote();
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
    QString name = QInputDialog::getText(this, tr("Add new note"), tr("Enter new note name"),
                                         QLineEdit::Normal, QString(), &ok);
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

void MainWindow::closeEvent(QCloseEvent *)
{
    saveCurrentNote();
}


void MainWindow::on_pbEditNote_toggled(bool checked)
{
    ui->plainTextEdit->setVisible(checked);
    ui->textBrowser->setVisible(!checked);
}

QTreeWidgetItem * MainWindow::selectedItem()
{
    if (isSomeItemSelected()) return ui->treeWidget->selectedItems().front();
    return 0;
}

void MainWindow::on_pbNewFolder_clicked()
{
    saveCurrentNote();

    bool ok;
    QString name = QInputDialog::getText(this, tr("Add new note"), tr("Enter new note name"),
                                         QLineEdit::Normal, QString(), &ok);
    if (!ok) return;

    QString selectedFolder;
    if (isSomeItemSelected())
    {
        QTreeWidgetItem * selItem = selectedItem();
        bool folder = !isNoteSelected();
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

bool MainWindow::isSomeItemSelected()
{
    return ui->treeWidget->selectedItems().count() > 0;
}

bool MainWindow::isNoteSelected()
{
    QList<QTreeWidgetItem*> selItems = ui->treeWidget->selectedItems();
    return ((selItems.count() > 0) && (selItems.front()->data(0, Qt::UserRole).toBool()));
}

bool MainWindow::renameCurrentNote()
{
    bool ok;
    QString name = QInputDialog::getText(this, tr("Rename note"), tr("Enter new note name"),
                                         QLineEdit::Normal, selectedItem()->text(0), &ok);
    if (!ok) return false;

    QFileInfo info(currentNote);
    QString sourceName = currentNote;
    QString destName = info.absolutePath() + QDir::separator() + name;
    if (!destName.toLower().endsWith(".md"))
    {
        destName.append(".md");
    }

    if (QFile(destName).exists())
    {
        QMessageBox::information(this, tr("Information"), tr("Note already exists"));
        return false;
    }

    return QFile::rename(sourceName, destName);
}

bool MainWindow::renameCurrentFolder()
{
    bool ok;
    QString name = QInputDialog::getText(this, tr("Rename Folder"), tr("Enter new folder name"),
                                         QLineEdit::Normal, selectedItem()->text(0), &ok);
    if (!ok) return false;

    QString commonPath = baseDirectory + createPath(getPathOfSelectedItem()) + QDir::separator();
    QString sourcePath = commonPath + selectedItem()->text(0);
    QString destPath = commonPath + name;
    qDebug() << sourcePath;

    if (QFile(destPath).exists())
    {
        QMessageBox::information(this, tr("Information"), tr("File or folder already exists"));
        return false;
    }

    return QFile::rename(sourcePath, destPath);
}

void MainWindow::on_pbProperties_clicked()
{
    if (!isSomeItemSelected()) return;

    saveCurrentNote();

    bool result = isNoteSelected() ? renameCurrentNote() : renameCurrentFolder();

    if (result)
    {
        loadDirectory(baseDirectory);
    }
}

bool MainWindow::removeDirectory(const QString &path)
{
    bool result = true;
    QDir dir(path);

    if (dir.exists(path))
    {
        foreach(const QFileInfo &info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden |
                                                         QDir::AllDirs | QDir::Files, QDir::DirsFirst))
        {
            if (info.isDir())
            {
                result = removeDirectory(info.absoluteFilePath());
            }
            else
            {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result)
            {
                return result;
            }
        }
        result = dir.rmdir(path);
    }
    return result;
}

void MainWindow::on_pbRemove_clicked()
{
    if (!isSomeItemSelected()) return;

    int button = QMessageBox::question(this, tr("Question"),
                                       isNoteSelected() ? tr("Delete current note?")
                                                        : tr("Delete current folder and all its content?"),
                                       QMessageBox::Yes | QMessageBox::No);
    if (button != QMessageBox::Yes) return;

    bool result;
    if (isNoteSelected())
    {
        qDebug() << "removing" << currentNote;
        result = QFile::remove(currentNote);
    }
    else
    {
        QString path = baseDirectory + createPath(getPathOfSelectedItem()) + QDir::separator() + selectedItem()->text(0);
        qDebug() << "removing" << path;
        result = removeDirectory(path);
    }
    qDebug() << "result:" << result;

    if (result)
    {
        loadDirectory(baseDirectory);
    }
}
