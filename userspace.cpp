#include "userspace.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QStringList>
#include <QTextBrowser>
#include <QTextStream>
#include <QtGlobal>
#include "globals.h"
#include "toolfunction.h"
#include "usersession.h""
// 用户模块
namespace User {

bool signup(const QString& username, const QString& password) {
    const QString& role = "normal";

    QFile file(userFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(',');
        if (parts.size() >= 2 && parts[0] == username) {
            Utils::print("用户已存在");
            file.close();
            return false; // 用户已存在
        }
    }
    file.close(); // 读完关闭，再以写入模式打开

    if (!file.open(QIODevice::Append | QIODevice::Text)) // Append 模式更安全
        return false;

    QTextStream out(&file);
    out << username << "," << password << "," << role << "\n";
    file.close();
    QString path=dbRoot+"/"+username;
    QDir().mkpath(path);
    currentUser=username;
    mainWindow->updateDirectoryView(currentUser);
    return true;
}

bool login(QString &user, QString &username1, QString &password1)
{
    QTextStream cin(stdin);
    QTextStream cout(stdout);
    cout.flush();
    QString username = username1;
    cout.flush();
    QString password = password1;

    QFile file(userFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(',');
        if (parts.size() >= 2 && parts[0] == username && parts[1] == password) {
            user = username;
            QString roleStr = parts[2].trimmed().toLower();
            Auth::Role role = Auth::ROLE_MAP.value(parts[2].trimmed(), Auth::NORMAL);
            qDebug() << "登录用户角色:" << roleStr << "映射为:" << role;

            Session::setCurrentUser(user, role);
            mainWindow->on_currentuser_textChanged(currentUser);
            return true;
        }
    }
    return false;
}
bool grantRole(const QString& adminUser, const QString& username, const QString& role) {
    // 检查权限
    if (Session::getCurrentUser() != adminUser ||
        !Auth::checkPermission(Auth::ADMIN)) {
        Utils::print("[!] 拒绝访问. 仅 ADMIN 能够进行授权.\n");
        return false;
    }

    // 验证角色有效性
    if (!Auth::ROLE_MAP.contains(role)) {
        Utils::print("[!] 非法的角色格式. 权限角色: guest, normal, admin\n");
        return false;
    }

    // 打开用户文件
    QFile file(userFile);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        Utils::print("[!] 打开用户数据失败.\n");
        return false;
    }

    // 读取并修改用户数据
    QStringList lines;
    QTextStream in(&file);
    bool found = false;

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(',');
        if (parts.size() >= 3 && parts[0] == username) {
            parts[2] = role;
            line = parts.join(",");
            found = true;
        }
        lines.append(line);
    }

    // 写回文件
    if (found) {
        file.resize(0); // 清空文件
        QTextStream out(&file);
        foreach (const QString &line, lines) {
            out << line << "\n";
        }
        Utils::print("成功将权限 " + role + " 授权于 " + username + "\n");
        return true;
    } else {
        Utils::print("[!]角色未找到.\n");
        return false;
    }
}
}
