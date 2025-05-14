#include "mainwindow.h"
#include "interprete.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QProgressDialog>
#include "globals.h"
#include "toolfunction.h"
QString fastlink;
QString fastfilename;
QString fastdbname;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)

{
    ui->setupUi(this);
    model = new QFileSystemModel(this);
    model->setRootPath("");
    ui->treeView->setModel(model);
    ui->treeView->setRootIndex(model->index(""));
    ui->inputDBname->hide();
    ui->fastinput->hide();
    ui->comfirm->hide();
    ui->cancel->hide();
    QString Welcomemessage=R"(
##############################################
          _____  ____  __  __  _____
         |  __ \|  _ \|  \/  |/ ____|
         | |  | | |_) | \  / | (___
         | |  | |  _ <| |\/| |\___ \
         | |__| | |_) | |  | |____) |
         |_____/|____/|_|  |_|_____/

               -> help:语法帮助 <-
                   github链接：
 https://github.com/halfpeop1e/projectDBMS.git

 ##############################################)";
    ui->shell->append(Welcomemessage);
    ui->shell->setFont(QFont("Courier New", 13));
    connect(ui->input, &QLineEdit::returnPressed,
            this, &MainWindow::on_send_clicked);
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
    QString userPath = dbRoot + "/" + username;
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
void MainWindow::displayFileContent(const QModelIndex &index){

        QString filePath = model->filePath(index);
    if(QFileInfo(filePath).isDir()&& QFileInfo(filePath).fileName()!="DATATYPE"){
            fastdbname=QFileInfo(filePath).fileName();
            fastfilename="";
    }
        if (QFileInfo(filePath).isFile() && filePath.endsWith(".txt")) {
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                QStringList rawLines;
                QString fileName = QFileInfo(filePath).fileName();
                fastfilename=QFileInfo(filePath).completeBaseName();
                qDebug()<<fastfilename;
                fastdbname="";
                while (!in.atEnd()) {
                    rawLines.append(in.readLine());
                }
                if (rawLines.isEmpty()) {
                    ui->contentdisplay->append("文件为空");
                    return;
                }
                QStringList header = rawLines.first().split(",");
                QList<int> selectedIndexes;
                for (int i = 0; i < header.size(); ++i)
                    selectedIndexes.append(i);

                QStringList formattedResult;
                for (const QString &line : rawLines) {
                    QStringList fields = line.split(",");
                    QStringList out;
                    for (int idx : selectedIndexes)
                        out.append(fields.value(idx));
                    formattedResult.append(out.join(","));
                }
                ui->contentdisplay->append(Utils::formatAsTable(formattedResult));
                file.close();
            }
        }
    }

void MainWindow::on_treeView_clicked(const QModelIndex &index)
{
    ui->contentdisplay->clear();
    displayFileContent(index);
}


void MainWindow::on_show_database_clicked()
{
    if(!ui->inputDBname->isVisible())
    {
        fastlink=dbRoot+"/"+currentUser;
        ui->inputDBname->show();
        ui->fastinput->show();
        ui->comfirm->show();
        ui->cancel->show();
    }
    else{
        fastlink="";
        ui->inputDBname->hide();
        ui->fastinput->hide();
        ui->comfirm->hide();
        ui->cancel->hide();
    }
}

void MainWindow::on_comfirm_clicked()
{
    Utils::setOutputShell(ui->shell);
    Interpreter::interpret("CREATE DATABASE "+ui->fastinput->text());
    ui->fastinput->clear();
}


void MainWindow::on_cancel_clicked()
{
    fastlink="";
    ui->inputDBname->hide();
    ui->fastinput->clear();
}




void MainWindow::on_load_clicked()
{
    if (!Auth::checkPermission(Auth::ADMIN)) {
        Utils::setOutputShell(ui->shell);
        Utils::print("[!]请求无效，需要ADMIN权限.\n");
        return;
    }
    else{
        QStringList filePaths = QFileDialog::getOpenFileNames(
            this,
            tr("选择文本文件"),
            QDir::homePath(),
            tr("文本文件 (*.txt)")
            );

        if(filePaths.isEmpty()) {
            return; // 用户取消了选择
        }
        QString destDir =dbRoot+"/"+currentUser+"/"+usingDatabase;
        QDir dir(destDir);
        if(!dir.exists()) {
            dir.mkpath(".");
        }
        QProgressDialog progress("正在加载...", "取消", 0, filePaths.size(), this);
        progress.setWindowModality(Qt::WindowModal);
        int successCount = 0;
        for(int i = 0; i < filePaths.size(); i++) {
            progress.setValue(i);
            progress.setLabelText(QString("正在处理文件 %1/%2")
                                      .arg(i+1).arg(filePaths.size()));

            if(progress.wasCanceled())
                break;

            QFileInfo fileInfo(filePaths[i]);
            QString destPath = destDir + "/" + fileInfo.fileName();

            // 检查文件是否已存在
            if(QFile::exists(destPath)) {
                QFile::remove(destPath);
            }

            if(QFile::copy(filePaths[i], destPath)) {
                successCount++;
            } else {
                QMessageBox::warning(this, tr("错误"),
                                     tr("无法复制文件: %1").arg(fileInfo.fileName()));
            }
        }

        progress.setValue(filePaths.size());
        QMessageBox::information(this, tr("完成"),
                                 tr("成功复制 %1/%2 个文件到目录:\n%3")
                                     .arg(successCount)
                                     .arg(filePaths.size())
                                     .arg(destDir));
    }
}


void MainWindow::on_clear_clicked()
{

    Utils::setOutputShell(ui->shell);
    if(fastdbname!=""){
    Interpreter::interpret("DROP DATABASE "+fastdbname);
    }
    if(fastfilename!=""){
        Interpreter::interpret("DROP TABLE "+fastfilename);
    }
    ui->fastinput->clear();
}


void MainWindow::on_back_clicked()
{
    QString backlink=currentUser;
    MainWindow::updateDirectoryView(backlink);
}


void MainWindow::on_use_database_clicked()
{
    if(fastdbname!="")
    {
        Utils::setOutputShell(ui->shell);
        Interpreter::interpret("USE "+fastdbname);
    }
    else{
        Utils::print("请选择数据库");
    }

}


void MainWindow::on_currentusingdb_textChanged(const QString &arg1)
{
    QString nowusingdb=arg1;
    ui->currentusingdb->setText(arg1);
}


void MainWindow::on_currentuser_textChanged(const QString &arg1)
{
    QString nowuser=arg1;
    ui->currentuser->setText(arg1);
}

