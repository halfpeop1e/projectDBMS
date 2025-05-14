#include "serverwindow.h"
#include "ui_serverwindow.h"
#include <QMessageBox>
#include <QDateTime>

ServerWindow::ServerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ServerWindow),
    m_server(new DatabaseServer(this))
{
    ui->setupUi(this);

    // 设置默认端口
    ui->portSpinBox->setValue(3310);

    // 连接信号
    connect(m_server, &DatabaseServer::logMessage, this, &ServerWindow::displayLogMessage);
    connect(m_server, &DatabaseServer::clientConnected, this, &ServerWindow::displayClientConnected);
    connect(m_server, &DatabaseServer::clientDisconnected, this, &ServerWindow::displayClientDisconnected);

    // 初始化UI状态
    ui->stopButton->setEnabled(false);

    setWindowTitle("数据库服务器");
}

ServerWindow::~ServerWindow()
{
    if (m_server->isRunning()) {
        m_server->stopServer();
    }
    delete ui;
}

void ServerWindow::on_startButton_clicked()
{
    int port = ui->portSpinBox->value();

    if (m_server->startServer(port)) {
        // 更新UI状态
        ui->startButton->setEnabled(false);
        ui->stopButton->setEnabled(true);
        ui->portSpinBox->setEnabled(false);

        displayLogMessage(QString("服务器成功启动在端口 %1").arg(port));
    } else {
        QMessageBox::critical(this, "错误", "无法启动服务器，请检查端口是否被占用。");
    }
}

void ServerWindow::on_stopButton_clicked()
{
    m_server->stopServer();

    // 更新UI状态
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
    ui->portSpinBox->setEnabled(true);
}

void ServerWindow::displayLogMessage(const QString &message)
{
    QString timeStamp = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss] ");
    ui->logTextEdit->append(timeStamp + message);

    // 确保滚动到最新消息
    QTextCursor cursor = ui->logTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->logTextEdit->setTextCursor(cursor);
    ui->logTextEdit->ensureCursorVisible();
}

void ServerWindow::displayClientConnected(const QString &clientInfo)
{
    QListWidgetItem *item = new QListWidgetItem(clientInfo);
    ui->clientsListWidget->addItem(item);
    ui->clientsListWidget->scrollToItem(item);
}

void ServerWindow::displayClientDisconnected(const QString &clientInfo)
{
    // 在已连接客户端列表中找到对应的客户端条目并移除
    // 注意：这里的实现比较简单，实际应用中可能需要更精确的匹配
    for (int i = 0; i < ui->clientsListWidget->count(); ++i) {
        QString item = ui->clientsListWidget->item(i)->text();
        if (item.contains(clientInfo.section(']', 1, 1).trimmed())) {
            delete ui->clientsListWidget->takeItem(i);
            break;
        }
    }
}
