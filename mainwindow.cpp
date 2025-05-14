#include "mainwindow.h"
#include "interprete.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QProgressDialog>
#include <QToolBar>
#include <QTableWidget>
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
    ui->tableWidget->hide();
    ui->addrow->hide();
    ui->savetable->hide();
    ui->typecombo->hide();
    ui->addcol->hide();
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
    ui->currentusingdb->setText(nowusingdb);
}


void MainWindow::on_currentuser_textChanged(const QString &arg1)
{
    QString nowuser=arg1;
    ui->currentuser->setText(nowuser);
}

void MainWindow::loadTxtAsTable(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot open file");
        return;
    }

    QTextStream in(&file);
    QStringList lines;
    while (!in.atEnd())
        lines << in.readLine();

    int rowCount = lines.size();
    if (rowCount == 0) return;

    QStringList firstRow = lines[0].split(",", Qt::SkipEmptyParts);
    int columnCount = firstRow.size();

    ui->tableWidget->setRowCount(rowCount);
    ui->tableWidget->setColumnCount(columnCount);

    for (int i = 0; i < rowCount; ++i) {
        QStringList values = lines[i].split(",", Qt::SkipEmptyParts);
        for (int j = 0; j < values.size(); ++j) {
            QTableWidgetItem* item = new QTableWidgetItem(values[j].trimmed());
            ui->tableWidget->setItem(i, j, item);
        }
    }

    file.close();
}
void MainWindow::updateTypeFile(const QString &typeFile, const QString &newType) {
    QFile file(typeFile);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qWarning() << "无法打开文件:" << typeFile;
        return;
    }

    QTextStream in(&file);
    QStringList typeLines;

    while (!in.atEnd()) {
        typeLines.append(in.readLine());
    }

    // 处理文件内容，假设类型文件有三行（字段类型、字段状态、默认值）
    if (typeLines.size() < 3) {
        // 如果文件内容不符合预期格式，先填充默认行
        typeLines.append("");  // 第一行（字段类型）
        typeLines.append("");  // 第二行（字段状态）
        typeLines.append("");  // 第三行（默认值）
    }

    // 更新字段类型行
    QStringList types = typeLines[0].split(",");
    types.append(newType);  // 在最后一列添加新字段类型
    typeLines[0] = types.join(",");

    // 更新字段状态行（默认为0）
    QStringList states = typeLines[1].split(",");
    states.append("0");  // 为新列添加默认状态（0）
    typeLines[1] = states.join(",");

    // 更新默认值行（默认为空）
    QStringList defaults = typeLines[2].split(",");
    defaults.append("");  // 为新列添加默认值（空）
    typeLines[2] = defaults.join(",");

    // 将更新后的内容写回文件
    file.resize(0);  // 清空文件内容
    QTextStream out(&file);
    for (const QString &line : typeLines) {
        out << line << "\n";
    }

    file.close();
}
void MainWindow::addColumn() {
    int columnCount = ui->tableWidget->columnCount();
    ui->tableWidget->insertColumn(columnCount);  // 在最后一列插入新的一列

    // 获取当前选择的字段类型
    QString currenttype = ui->typecombo->currentText();

    // 构建类型文件路径
    QString typeFile = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + "DATATYPE" + "/" + fastfilename + "_data.txt";

    // 更新类型文件
    updateTypeFile(typeFile, currenttype);

    // 初始化新列中的每个单元格
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QTableWidgetItem *newItem = nullptr;

        if (row == 0) {
            // 第一行：字段名，默认为空
            newItem = new QTableWidgetItem("");
        }
        else if (row == 1) {
            // 第二行：字段状态（默认为 0）
            newItem = new QTableWidgetItem("0");
        }
        else if (row == 2) {
            // 第三行：默认值，默认为空
            newItem = new QTableWidgetItem("");
        }

        ui->tableWidget->setItem(row, columnCount, newItem);  // 设置新的单元格到新列
    }
}
void MainWindow::addRow() {
    int rowCount = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(rowCount);  // 在最后一行插入新的一行

    // 如果需要，初始化新行中的每个单元格
    for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
        QTableWidgetItem *newItem = new QTableWidgetItem("");  // 创建一个空的单元格
        ui->tableWidget->setItem(rowCount, col, newItem);  // 设置新的单元格到新行
    }
}
void MainWindow::saveFile() {
    if (currentpath.isEmpty()) {
        QMessageBox::warning(this, "Error", "No file loaded.");
        return;
    }

    QFile file(currentpath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot open file for writing.");
        return;
    }

    QTextStream out(&file);

    // 遍历所有行和列，将每个单元格的数据写入文件
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QStringList rowData;
        for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
            QTableWidgetItem* item = ui->tableWidget->item(row, col);
            if (item) {
                rowData << item->text(); // 获取单元格文本
            } else {
                rowData << ""; // 如果没有数据，填充空字符串
            }
        }
        out << rowData.join(",") << "\n"; // 使用逗号分隔每个单元格，换行结束
    }

    file.close();
    QMessageBox::information(this, "Success", "File saved successfully.");
}

void MainWindow::on_opentable_clicked()
{
    QString showfile= dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + fastfilename + ".txt";
    currentpath=showfile;
    ui->tableWidget->show();
    ui->addrow->show();
    ui->addcol->show();
    ui->savetable->show();
    ui->typecombo->show();
    MainWindow::loadTxtAsTable(showfile);
}


void MainWindow::on_closetable_clicked()
{
    ui->tableWidget->hide();
    ui->addrow->hide();
    ui->addcol->hide();
    ui->savetable->hide();
    ui->typecombo->hide();
}


void MainWindow::on_addrow_clicked()
{
    addRow();
}



void MainWindow::on_savetable_clicked()
{
    saveFile();
    ui->tableWidget->hide();
    ui->addrow->hide();
    ui->savetable->hide();
    ui->typecombo->hide();
    ui->addcol->hide();
}


void MainWindow::on_addcol_clicked()
{
    Utils::print("已添加字段，类型"+ui->typecombo->currentText());
    addColumn();
}

