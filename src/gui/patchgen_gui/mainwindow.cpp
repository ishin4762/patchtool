#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::browsePatchFile()
{
    QString defaultFileName = ui->lineEditPatchFile->text();
    QString filename = QFileDialog::getSaveFileName(
        this, tr("Choose patch file location"), defaultFileName);

    if (!filename.isEmpty()) {
        ui->lineEditPatchFile->setText(filename);
    }
}

void MainWindow::browseSrcDirectory()
{
    QString defaultDirectory = ui->lineEditSrcDirectory->text();
    QFileDialog::Options options =
          QFileDialog::ShowDirsOnly | QFileDialog::HideNameFilterDetails;
    QString filename = QFileDialog::getExistingDirectory(
        this, tr("Choose older version's directory location"), defaultDirectory, options);

    if (!filename.isEmpty()) {
        ui->lineEditSrcDirectory->setText(filename);
    }
}

void MainWindow::browseDestDirectory()
{
    QString defaultDirectory = ui->lineEditDestDirectory->text();
    QFileDialog::Options options =
          QFileDialog::ShowDirsOnly | QFileDialog::HideNameFilterDetails;
    QString filename = QFileDialog::getExistingDirectory(
        this, tr("Choose new version's directory location"), defaultDirectory, options);

    if (!filename.isEmpty()) {
        ui->lineEditDestDirectory->setText(filename);
    }
}

void MainWindow::createPatchFile()
{
    if (ui->lineEditPatchFile->text().isEmpty()
            || ui->lineEditSrcDirectory->text().isEmpty()
            || ui->lineEditDestDirectory->text().isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Text boxes must be filled."));
        return;
    }

    if (process != nullptr) {
        delete process;
    }
    process = new QProcess(this);
    connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(processOutput()));
    connect(process, SIGNAL(readyReadStandardError()), this, SLOT(processOutput()));

    QStringList args;
    if (ui->checkBoxExecutable->isChecked()) {
        args << "-e";
    }
    args << ui->lineEditSrcDirectory->text();
    args << ui->lineEditDestDirectory->text();
    args << ui->lineEditPatchFile->text();

    ui->pushButtonCreate->setEnabled(false);
    ui->textEditLog->clear();
    ui->textEditLog->setTextColor(QColor("blue"));
    ui->textEditLog->append("$ ./patchgen " + args.join(" "));
    ui->textEditLog->setTextColor(QColor("black"));

    process->start(qApp->applicationDirPath() + "/patchgen", args);
    ui->pushButtonCreate->setEnabled(true);
}

void MainWindow::processOutput()
{
    QString standardOut = process->readAllStandardOutput();
    QString standartErr = process->readAllStandardError();
    if (!standardOut.isEmpty()) {
        ui->textEditLog->append(standardOut);
    }
    if (!standartErr.isEmpty()) {
        ui->textEditLog->setTextColor(QColor("red"));
        ui->textEditLog->append(standartErr);
        ui->textEditLog->setTextColor(QColor("black"));
    }
}

