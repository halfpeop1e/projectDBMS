#include "globals.h"

QString currentUser="guest";
Auth::Role currentRole;
QTextBrowser *showinShell = nullptr;
QString usingDatabase;
QString dbRoot = "E:/dbms/Resource";             // 所有数据库存放路径
QString userFile = "E:/dbms/Resource/users.txt"; // 用户信息存储文件
QString logFile = "E:/dbms/Resource/log.txt";    // 日志文件
MainWindow *mainWindow = nullptr;
