#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileSystemModel>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QTextBrowser>
#include <QTableWidget>
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
    void welcome();
    void displayFileContent(const QModelIndex &index);
    void on_currentusingdb_textChanged(const QString &arg1);
    void on_currentuser_textChanged(const QString &arg1);
    void loadTxtAsTable(const QString& filePath);
    void addColumn();
    void addRow();
    void saveFile();
    ~MainWindow();

private slots:
    void on_send_clicked();

    void on_treeView_clicked(const QModelIndex &index);
    void on_show_database_clicked();

    void on_cancel_clicked();

    void on_comfirm_clicked();

    void on_load_clicked();

    void on_clear_clicked();

    void on_back_clicked();

    void on_use_database_clicked();

    void on_opentable_clicked();

    void on_closetable_clicked();

    void on_addrow_clicked();

    void on_savetable_clicked();

private:
    Ui::MainWindow *ui;
    QFileSystemModel *model;
    QTableWidget *table;
    QString currentpath;
    void loadToTable(const QString &filePath);
};

#endif // MAINWINDOW_H
