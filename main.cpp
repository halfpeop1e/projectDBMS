#include "mainwindow.h"
#include "interprete.h"
#include"globals.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    qputenv("LANG", "zh_CN.UTF-8");
    qputenv("LC_ALL", "zh_CN.UTF-8");
    QApplication a(argc, argv);
    initializeUserFile();
    mainWindow = new MainWindow();
    mainWindow->show();
    return a.exec();
}
