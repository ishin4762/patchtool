#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QTextStream>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QProcess *process = nullptr;

protected slots:
    void browsePatchFile();
    void browseSrcDirectory();
    void browseDestDirectory();
    void createPatchFile();
    void processOutput();
};
#endif // MAINWINDOW_H
