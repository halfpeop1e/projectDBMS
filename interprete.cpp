#include "interprete.h"
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
#include "mainwindow.h"
#include "globals.h"
#include"toolfunction.h"
#include "usersession.h"
#include "userspace.h"
#include "dbmsoperate.h"
//验证模块
namespace Auth {
bool checkPermission(Role requiredRole) {
    return Session::getCurrentRole() >= requiredRole;
}

QString getRoleName(Role role) {
    return ROLE_MAP.key(role, "normal");
} // namespace User
}
// DBMS模块

// 解释器模块
namespace Interpreter {
void interpret(const QString &command)
{
    QStringList tokens = Utils::splitCommand(command);
    if (tokens.isEmpty())
        return;
    QString op = tokens[0].toUpper();

    if (op == "HELP") {
        Utils::showHelp();
    } else if (op == "REGISTER" && tokens.size() >= 3) {
        if (User::signup(tokens[1], tokens[2])) {
            QTextStream cout(stdout);

           Utils::print ("注册成功.\n");
        } else {
            QTextStream cout(stdout);
           Utils::print ("[!] 注册失败.\n");

        }
    } else if (op == "LOGIN" && tokens.size() >= 3) {
        if (User::login(currentUser, tokens[1], tokens[2])) {
            QTextStream cout(stdout);

            Utils::print("登录成功. 欢迎你, " + currentUser + "\n");// 保存当前用户
            QFile file(dbRoot+currentUser);
             mainWindow->updateDirectoryView(currentUser);


        } else {
            QTextStream cout(stdout);
            Utils::print("[!] 登录失败，请检查用户名或密码.\n");
        }

    }
    else if (op == "ROLES") {
        Utils::print("可用的权限: guest, normal, admin\n");
    }
    else if (op == "SHOWME") {
        QString roleName = Auth::getRoleName(Session::getCurrentRole());
        Utils::print("当前角色及其权限: " + currentUser + " (" + roleName + ")\n");
    }
    else if (op == "GRANT" && tokens.size() >= 3) {
        QString username = tokens[1];
        QString role = tokens[2].toLower();
        User::grantRole(currentUser, username, role);
    }
    else if (op == "CREATE" && tokens.size() >= 3 && tokens[1].toUpper() == "DATABASE") {
        if (!Auth::checkPermission(Auth::ADMIN)) {
            Utils::print("[!]请求无效，需要ADMIN权限.\n");
            return;
        }
        DBMS::createDatabase(tokens[2]);
    }
    else if (op == "DROP" && tokens.size() >= 3 && tokens[1].toUpper() == "DATABASE") {
        if (!Auth::checkPermission(Auth::ADMIN)) {
            Utils::print("[!]请求无效，需要ADMIN权限.\n");
            return;
        }

        DBMS::dropDatabase(tokens[2]);
    } else if (op == "USE" && tokens.size() >= 2) {
        DBMS::useDatabase(tokens[1]);

        mainWindow->updateDirectoryView(currentUser+"/"+tokens[1]);
    }
    else if (op == "CREATE" && tokens.size() >= 3 && tokens[1].toUpper() == "TABLE") {
        if (!Auth::checkPermission(Auth::ADMIN)) {
            Utils::print("[!]请求无效，需要ADMIN权限.\n");
            return;
        }

        QString tableName = tokens[2];
        int start = command.indexOf('(');
        int end = command.indexOf(')');
        if (start != -1 && end != -1 && end > start) {
            QString cols = command.mid(start + 1, end - start - 1);
            DBMS::createTable(tableName, cols);
        }

    }
    else if (op == "DROP" && tokens.size() >= 3 && tokens[1].toUpper() == "TABLE") {
        if (!Auth::checkPermission(Auth::ADMIN)) {
            Utils::print("[!]请求无效，需要ADMIN权限.\n");
            return;
        }
        DBMS::dropTable(tokens[2]);
    }
    else if (op == "INSERT" && tokens.size() >= 4 && tokens[1].toUpper() == "INTO") {
        if (!Auth::checkPermission(Auth::ADMIN)) {
            Utils::print("[!]请求无效，需要ADMIN权限.\n");
            return;
        }

        QString tableName = tokens[2];
        int start = command.indexOf('(');
        int end = command.indexOf(')');
        if (start != -1 && end != -1 && end > start) {
            QString vals = command.mid(start + 1, end - start - 1);
            DBMS::insertInto(tableName, vals);
        }
    } else if (op == "SELECT") {
        DBMS::selectAdvanced(command);

    }else if (op == "DELETE" && tokens.size() >= 5 && tokens[1].toUpper() == "FROM") {
        if (!Auth::checkPermission(Auth::ADMIN)) {
            Utils::print("[!]请求无效，需要ADMIN权限.\n");
            return;
        }
        QString tableName = tokens[2];
        QString condition = tokens.mid(4).join(' ');
        DBMS::deleteFrom(tableName, condition);
    } else if(op == "ALTER"){
        if (!Auth::checkPermission(Auth::ADMIN)) {
            Utils::print("[!]请求无效，需要ADMIN权限.\n");
            return;
        }
        DBMS::headerManage(tokens.join(' '));
    }else if (op == "CREATE" && tokens.size() >= 4 && tokens[1].toUpper() == "INDEX") {
        if (!Auth::checkPermission(Auth::ADMIN)) {
            Utils::print("[!] 需要ADMIN权限.\n");
            return;
        }

        QRegularExpression re("CREATE\\s+INDEX\\s+(\\w+)\\s+ON\\s+(\\w+)\\((\\w+)\\)",
                              QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = re.match(command);

        if (match.hasMatch()) {
            QString indexType = match.captured(1).toUpper();
            QString tableName = match.captured(2);
            QString columnName = match.captured(3);

            if (indexType != "BTREE" && indexType != "HASH") {
                Utils::print("[!] 无效的索引类型，支持 BTREE 或 HASH\n");
                return;
            }

            DBMS::IndexManager idxManager(currentUser, usingDatabase);
            if (idxManager.createIndex(tableName, columnName, indexType)) {
                Utils::print("[+] 索引创建成功\n");
            } else {
                Utils::print("[!] 索引创建失败\n");
            }
        } else {
            Utils::print("[!] 无效的CREATE INDEX语法\n");
        }
    }else if (op == "DROP" && tokens.size() >= 3 && tokens[1].toUpper() == "INDEX") {
        if (!Auth::checkPermission(Auth::ADMIN)) {
            Utils::print("[!] 需要ADMIN权限.\n");
            return;
        }

        QStringList parts = tokens[2].split(".");
        if (parts.size() != 2) {
            Utils::print("[!] 无效的DROP INDEX语法，使用: DROP INDEX table.column\n");
            return;
        }

        DBMS::IndexManager idxManager(currentUser, usingDatabase);
        if (idxManager.dropIndex(parts[0], parts[1])) {
            Utils::print("[+] 索引删除成功\n");
        } else {
            Utils::print("[!] 索引删除失败\n");
        }
    }else {
        QTextStream cout(stdout);
        Utils::print("[!] Unknown command.\n");
    }
}
}
