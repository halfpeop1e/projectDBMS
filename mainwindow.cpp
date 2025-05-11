#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "interprete.h"
#include "function.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)

{
    ui->setupUi(this);
    model = new QFileSystemModel(this);
    model->setRootPath("");
    ui->treeView->setModel(model);
    ui->treeView->setRootIndex(model->index(""));

}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_send_clicked()
{
    Utils::setOutputShell(ui->shell);
    ui->shell->append(ui->input->text());
    Interpreter::interpret(ui->input->text());
    ui->input->clear();
}
void MainWindow::updateDirectoryView(const QString &username)
{
    QString userPath = Utils::dbRoot+"/"+username;
    QDir dir(userPath);
    if (!dir.exists()) {
        Utils::print("切换至对应界面失败");
        qDebug() << "[!] 目录不存在:" << userPath;
        return;
    }

    model->setRootPath(userPath);
    ui->treeView->setRootIndex(model->index(userPath));
    Utils::print("切换至对应界面成功");
    qDebug() << "[*] 目录切换成功:" << userPath;
}


