#include "globals.h"

QString currentUser="guest";
Auth::Role currentRole;
QTextBrowser *showinShell = nullptr;
QString usingDatabase;
QString dbRoot = "/Users/fengzhu/Resource";             // 所有数据库存放路径
QString userFile = "/Users/fengzhu/Resource/users.txt"; // 用户信息存储文件
QString logFile = "/Users/fengzhu/Resource/log.txt";    // 日志文件
MainWindow *mainWindow = nullptr;
