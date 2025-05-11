#include "mainwindow.h"
#include "function.h"
#include "interprete.h"
#include <QApplication>
MainWindow *mainWindow = nullptr;
int main(int argc, char *argv[])
{
    qputenv("LANG", "zh_CN.UTF-8");
    qputenv("LC_ALL", "zh_CN.UTF-8");
    QApplication a(argc, argv);
    mainWindow = new MainWindow();
    mainWindow->show();
    return a.exec();
}
