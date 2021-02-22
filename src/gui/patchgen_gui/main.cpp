#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Translation
    QString locale = QLocale::system().name();
    QTranslator tra;
    if (locale.compare("ja_JP") == 0) {
        tra.load("patchgen_gui_" + locale + ".qm", qApp->applicationDirPath());
        a.installTranslator(&tra);
    }

    MainWindow w;
    w.show();
    return a.exec();
}
