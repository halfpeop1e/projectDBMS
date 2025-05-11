#include "mainwindow.h"
#include "function.h"
#include "interprete.h"
#include "ui_mainwindow.h"

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
    QString userPath = Utils::dbRoot + "/" + username;
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
        if (QFileInfo(filePath).isFile() && filePath.endsWith(".txt")) {
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                QStringList rawLines;

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

