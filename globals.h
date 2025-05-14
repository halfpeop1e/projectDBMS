#ifndef GLOBALS_H
#define GLOBALS_H
#include <QString>
#include <QTextBrowser>
#include "mainwindow.h"
namespace Auth {
enum Role {
    GUEST = 0,    // 只读权限
    NORMAL = 1,   // 基本操作权限
    ADMIN = 2     // 所有权限
};

const QMap<QString, Role> ROLE_MAP = {
    {"guest", GUEST},
    {"normal", NORMAL},
    {"admin", ADMIN}
};
bool checkPermission(Role requiredRole);
QString getRoleName(Role role);
}
// 全局变量
extern QString currentUser;
extern QString usingDatabase;
extern Auth::Role currentRole;
extern QTextBrowser *showinShell;
extern QString dbRoot;
extern QString userFile;
extern QString logFile;
extern MainWindow *mainWindow;
#endif // GLOBALS_H
