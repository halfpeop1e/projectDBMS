#include "mainwindow.h"
#include "function.h"
#include <QApplication>
MainWindow *mainWindow = nullptr;
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    mainWindow = new MainWindow();
    mainWindow->show();
    return a.exec();
}
