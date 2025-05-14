#ifndef DATABASESERVER_H
#define DATABASESERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDateTime>
#include <QList>
#include <QHash>

class DatabaseServer : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseServer(QObject *parent = nullptr);
    ~DatabaseServer();

    bool startServer(int port);
    void stopServer();
    bool isRunning() const;
    QString errorString() const;
    QString processClientCommand(const QString& command, QTcpSocket* clientSocket);
    static void sendToClient(QTcpSocket* clientSocket, const QString& message);
signals:
    void logMessage(const QString &message);
    void clientConnected(const QString &clientInfo);
    void clientDisconnected(const QString &clientInfo);

private slots:
    void handleNewConnection();
    void handleClientDisconnected(QTcpSocket *clientSocket);
    void handleSocketError(QTcpSocket *clientSocket, QAbstractSocket::SocketError socketError);
    void handleReadyRead(QTcpSocket *clientSocket);


private:
    QTcpServer *m_server;
    QList<QTcpSocket*> m_clientSockets;
    QHash<QTcpSocket*, QString> m_clientInfoMap;  // 添加这行
};

#endif // DATABASESERVER_H
