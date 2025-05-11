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
    QString Welcomemessage=R"(##############################################
  _____  ____  __  __  _____
 |  __ \|  _ \|  \/  |/ ____|
 | |  | | |_) | \  / | (___
 | |  | |  _ <| |\/| |\___ \
 | |__| | |_) | |  | |____) |
 |_____/|____/|_|  |_|_____/

       -> help:语法帮助 <-

 ##############################################)";
    ui->shell->append(Welcomemessage);
    ui->shell->setFont(QFont("Courier New", 13));


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
        qDebug() << "[!] 目录不存在:" << userPath;
        return;
    }

    model->setRootPath(userPath);
    ui->treeView->setRootIndex(model->index(userPath));
    Utils::print("切换至对应界面成功");
    qDebug() << "[*] 目录切换成功:" << userPath;
}


