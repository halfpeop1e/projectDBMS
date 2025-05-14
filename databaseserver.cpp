#include "databaseserver.h"
#include <QDebug>
#include <QHostAddress>
#include <QDateTime>

DatabaseServer::DatabaseServer(QObject *parent)
    : QObject(parent),
    m_server(new QTcpServer(this))
{
    connect(m_server, &QTcpServer::newConnection, this, &DatabaseServer::handleNewConnection);
}
DatabaseServer::~DatabaseServer()
{
    stopServer();
}

bool DatabaseServer::startServer(int port)
{
    if (m_server->isListening()) {
        stopServer();
    }

    if (!m_server->listen(QHostAddress::Any, port)) {
        emit logMessage(QString("服务器启动失败: %1").arg(m_server->errorString()));
        return false;
    }

    emit logMessage(QString("服务器开始监听端口 %1").arg(port));
    return true;
}

QString DatabaseServer::errorString() const
{
    return m_server->errorString();
}

void DatabaseServer::stopServer()
{
    if (m_server->isListening()) {
        for (QTcpSocket *socket : m_clientSockets) {
            if (socket && socket->state() == QAbstractSocket::ConnectedState) {
                socket->disconnectFromHost();
            }
        }
        m_server->close();
        emit logMessage("服务器已停止");
    }
}

bool DatabaseServer::isRunning() const
{
    return m_server->isListening();
}

void DatabaseServer::handleNewConnection()
{
    QTcpSocket *clientSocket = m_server->nextPendingConnection();

    if (!clientSocket) {
        emit logMessage("错误：获取客户端连接失败");
        return;
    }

    // 保存客户端信息（立即保存）
    QString clientInfo = QString("[%1] %2:%3")
                             .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                             .arg(clientSocket->peerAddress().toString())
                             .arg(clientSocket->peerPort());
    m_clientInfoMap.insert(clientSocket, clientInfo);  // 使用insert保存

    clientSocket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);

    connect(clientSocket, &QTcpSocket::disconnected, this, [this, clientSocket]() {
        this->handleClientDisconnected(clientSocket);
    });

    connect(clientSocket, &QTcpSocket::readyRead, this, [this, clientSocket]() {
        this->handleReadyRead(clientSocket);
    });

    connect(clientSocket, &QAbstractSocket::errorOccurred, this, [this, clientSocket](QAbstractSocket::SocketError socketError) {
        this->handleSocketError(clientSocket, socketError);
    });

    m_clientSockets.append(clientSocket);

    QString logMsg = QString("[%1] 新客户端连接: %2")
                         .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                         .arg(clientInfo);

    emit clientConnected(clientInfo);
    emit logMessage(logMsg);
}

void DatabaseServer::handleClientDisconnected(QTcpSocket *clientSocket)
{
    if (clientSocket) {
        QString clientInfo = m_clientInfoMap.value(clientSocket, "未知客户端");

        QString logMsg = QString("[%1] 客户端断开连接: %2")
                             .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                             .arg(clientInfo);

        m_clientSockets.removeOne(clientSocket);
        m_clientInfoMap.remove(clientSocket);

        emit clientDisconnected(clientInfo);
        emit logMessage(logMsg);

        clientSocket->deleteLater();
    }
}

void DatabaseServer::handleSocketError(QTcpSocket *clientSocket, QAbstractSocket::SocketError socketError)
{
    if (clientSocket) {
        QString clientInfo = m_clientInfoMap.value(clientSocket, "未知客户端");
        QString errorMsg = QString("客户端 %1 发生错误: %2")
                               .arg(clientInfo)
                               .arg(clientSocket->errorString());
        emit logMessage(errorMsg);
    }
}

void DatabaseServer::handleReadyRead(QTcpSocket *clientSocket)
{
    if (!clientSocket || clientSocket->state() != QAbstractSocket::ConnectedState) {
        return;
    }

    // 直接从map获取预先保存的完整客户端信息
    QString clientInfo = m_clientInfoMap.value(clientSocket, "未知客户端");

    while (clientSocket->canReadLine()) {
        QByteArray rawData = clientSocket->readLine().trimmed();
        QString command = QString::fromUtf8(rawData);

        emit logMessage(QString("%1 接收命令: %2")
                            .arg(clientInfo)  // 这里已经包含时间戳
                            .arg(command));

        processClientCommand(command, clientSocket);
        clientSocket->readAll();  // 清空缓冲区
    }
}

QString DatabaseServer::processClientCommand(const QString& command, QTcpSocket* clientSocket)
{
    // 这里实现命令处理逻辑
    // 但不返回任何响应给客户端
    return QString(); // 返回空字符串
}

void DatabaseServer::sendToClient(QTcpSocket* clientSocket, const QString& message)
{
    // 空实现，不发送任何消息给客户端
    Q_UNUSED(clientSocket);
    Q_UNUSED(message);
}
