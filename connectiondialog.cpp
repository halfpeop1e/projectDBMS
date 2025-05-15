#include "connectiondialog.h"
#include "ui_connectiondialog.h"
#include <QMessageBox>
#include <QTcpSocket>
#include <QDebug>

ConnectionDialog::ConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionDialog),
    m_socket(nullptr)  // 初始化socket指针为nullptr
{
    ui->setupUi(this);
    // 设置默认值
    ui->IPline->setText("localhost");
    ui->Portline->setText("");
    ui->DBBox->setCurrentText("");
    setWindowTitle("连接到数据库服务器");
}

ConnectionDialog::~ConnectionDialog()
{
    // 在析构函数中确保释放socket资源
    disconnectFromServer();
    delete ui;
}

QString ConnectionDialog::getIpline() const
{
    return ui->IPline->text();
}

int ConnectionDialog::getPortline() const
{
    return ui->Portline->text().toInt();
}

QString ConnectionDialog::getDatabaseName() const
{
    return ui->DBBox->currentText();
}

QTcpSocket* ConnectionDialog::getSocket() const
{
    return m_socket;
}

// 为防止自动删除socket，添加一个方法将socket所有权转移
QTcpSocket* ConnectionDialog::releaseSocket() {
    QTcpSocket* socket = m_socket;  // 将类成员m_socket的指针赋值给局部变量socket
    m_socket = nullptr;             // 将类成员m_socket设为空指针
    return socket;                  // 返回存储了原始指针值的局部变量
}


void ConnectionDialog::setConnectionInfo(const QString &ip, int port, const QString &dbName)
{
    ui->IPline->setText(ip);
    ui->Portline->setText(QString::number(port));
    ui->DBBox->setCurrentText(dbName);
}

void ConnectionDialog::on_ConnectButton_clicked()
{
    // 获取连接信息
    QString ip = getIpline();
    int port = getPortline();

    // 验证输入不为空
    if (ip.isEmpty() || port <= 0) {
        QMessageBox::warning(this, "输入验证", "请填写必要的连接信息。");
        return;
    }

    // 如果已经有存在的连接，先断开
    disconnectFromServer();

    // 创建新的socket连接
    m_socket = new QTcpSocket(this);

    // 连接socket的信号槽
    connect(m_socket, &QTcpSocket::connected, this, &ConnectionDialog::onSocketConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &ConnectionDialog::onSocketDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &ConnectionDialog::onSocketReadyRead);
    connect(m_socket, &QAbstractSocket::errorOccurred, this, &ConnectionDialog::onSocketError);

    // 设置低延迟选项
    m_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);

    // 连接到服务器
    m_socket->connectToHost(ip, port);

    // 等待连接建立(最多等待3秒)
    if (!m_socket->waitForConnected(3000)) {
        QMessageBox::critical(this, "连接错误",
                              QString("无法连接到服务器: %1\n错误: %2")
                                  .arg(ip + ":" + QString::number(port))
                                  .arg(m_socket->errorString()));
        disconnectFromServer();
        return;
    }

    // 连接成功，发送数据库类型信息
    sendDatabaseInfo();

    // 连接成功后接受对话框
    accept();
}

void ConnectionDialog::on_CancelButton_clicked()
{
    // 拒绝对话框
    reject();
}

bool ConnectionDialog::testConnection(const QString &ip, int port)
{
    // 创建临时套接字进行连接测试
    QTcpSocket socket;
    socket.connectToHost(ip, port);

    // 等待连接建立
    if (!socket.waitForConnected(3000)) {
        return false;
    }

    // 断开连接
    socket.disconnectFromHost();
    return true;
}

void ConnectionDialog::sendDatabaseInfo()
{
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        // 发送数据库连接信息
        QString clientInfo = QString("客户端信息: 数据库类型=%1").arg(getDatabaseName());
        m_socket->write(clientInfo.toUtf8());
        m_socket->flush();

        // 等待数据发送完成
        if (!m_socket->waitForBytesWritten(1000)) {
            qDebug() << "警告: 数据发送可能不完整";
        }
    }
}

void ConnectionDialog::disconnectFromServer()
{
    if (m_socket) {
        // 如果socket已连接，先断开连接
        if (m_socket->state() == QAbstractSocket::ConnectedState) {
            m_socket->disconnectFromHost();
            if (m_socket->state() != QAbstractSocket::UnconnectedState) {
                m_socket->waitForDisconnected(1000);
            }
        }

        // 断开所有信号连接
        m_socket->disconnect();

        // 删除socket对象
        m_socket->deleteLater();
        m_socket = nullptr;
    }
}

// 新增：连接成功处理
void ConnectionDialog::onSocketConnected()
{
    qDebug() << "已成功连接到服务器";
    // 这里可以添加连接成功后的操作
}

// 新增：断开连接处理
void ConnectionDialog::onSocketDisconnected()
{
    qDebug() << "已断开与服务器的连接";
    // 这里可以处理连接断开后的操作
}

// 新增：处理接收到的数据
void ConnectionDialog::onSocketReadyRead()
{
    if (m_socket) {
        QByteArray data = m_socket->readAll();
        qDebug() << "从服务器接收数据: " << data;

        // 处理服务器回应
        // 这里可以根据不同的响应执行不同操作
    }
}

// 新增：处理Socket错误
void ConnectionDialog::onSocketError(QAbstractSocket::SocketError socketError)
{
    if (m_socket) {
        qDebug() << "Socket错误: " << m_socket->errorString();

        // 处理错误
        if (socketError != QAbstractSocket::RemoteHostClosedError) {
            QMessageBox::warning(this, "连接错误",
                                 QString("连接发生错误: %1").arg(m_socket->errorString()));
        }
    }
}



