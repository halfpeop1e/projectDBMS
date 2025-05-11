#include <QApplication>
#include "function.h"
#include "mainwindow.h"
MainWindow *mainWindow = nullptr;
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    mainWindow = new MainWindow();
    mainWindow->show();
    return a.exec();
}
