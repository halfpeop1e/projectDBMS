#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileSystemModel>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QTextBrowser>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void updateDirectoryView(const QString &username);
    ~MainWindow();

private slots:
    void on_send_clicked();

private:
    Ui::MainWindow *ui;
    QFileSystemModel *model;
};

#endif // MAINWINDOW_H
