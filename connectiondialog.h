#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>
#include <QTcpSocket>

namespace Ui {
class ConnectionDialog;
}

class ConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionDialog(QWidget *parent = nullptr);
    ~ConnectionDialog();

    // 获取连接信息
    QString getIpline() const;
    int getPortline() const;
    QString getDatabaseName() const;
    QTcpSocket* getSocket() const;  // 新增：获取Socket

    // 设置连接信息
    void setConnectionInfo(const QString &ip, int port, const QString &dbName);

    // 测试连接
    static bool testConnection(const QString &ip, int port);



    QTcpSocket* releaseSocket();

public slots:
    // 新增：处理Socket相关的槽函数
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketReadyRead();
    void onSocketError(QAbstractSocket::SocketError socketError);

private slots:
    void on_ConnectButton_clicked();
    void on_CancelButton_clicked();

private:
    Ui::ConnectionDialog *ui;
    QTcpSocket *m_socket;  // 新增：持久化的Socket连接

    // 新增：发送数据库信息
    void sendDatabaseInfo();

    // 新增：断开与服务器的连接
    void disconnectFromServer();
};

#endif // CONNECTIONDIALOG_H
