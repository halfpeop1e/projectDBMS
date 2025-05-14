
#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QMainWindow>
#include "databaseserver.h"

namespace Ui {
class ServerWindow;
}

class ServerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow();

private slots:
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void displayLogMessage(const QString &message);
    void displayClientConnected(const QString &clientInfo);
    void displayClientDisconnected(const QString &clientInfo);

private:
    Ui::ServerWindow *ui;
    DatabaseServer *m_server;
};

#endif // SERVERWINDOW_H
