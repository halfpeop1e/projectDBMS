#include "mainwindow.h"
#include "interprete.h"
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QRegularExpression>
#include <QStringList>
#include <QDebug>
#include <QTextBrowser>

// 全局变量
QString currentUser;
QString usingDatabase;
QTextBrowser* showinShell = nullptr;
// 工具函数区
extern MainWindow *mainWindow;
namespace Utils {
QString dbRoot = "../../Resource"; // 所有数据库存放路径
QString userFile = "../../Resource/users.txt"; // 用户信息存储文件
QString logFile = "../../Resource/log.txt"; // 日志文件
void setOutputShell(QTextBrowser* shell) {
    showinShell = shell;
}
void print(const QString& msg) {
    if (showinShell) {
        showinShell->append(msg);
    } else {
        QTextStream(stdout) << msg << "\n";
    }
}
void writeLog(const QString& entry) {
    QFile file(logFile);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        out << "[" << time << "] " << currentUser << ": " << entry << "\n";
        file.close();
    }
}

QStringList splitCommand(const QString& command) {
    QRegularExpression regex("\\s+");
    return command.split(regex, Qt::SkipEmptyParts);
}

void showHelp() {
    QTextStream cout(stdout);
    print( "========Commands========\n");
    print ("REGISTER username password\n");
    print ("LOGIN username password\n");
    print ("CREATE DATABASE dbname\n");
    print ("DROP DATABASE dbname\n");
    print("USE dbname\n");
    print ("CREATE TABLE tablename (col1,col2,...)\n");
    print("DROP TABLE tablename\n");
    print ("INSERT INTO tablename VALUES (val1,val2,...)\n");
    print("SELECT * FROM tablename\n");
    print("DELETE FROM tablename WHERE col=value\n");
    print("EXIT / QUIT\n\n");
}

bool ensureDirExists(const QString& path) {
    QDir dir(path);
    if (!dir.exists()) {
        return dir.mkpath(".");
    }
    return true;
}
}

// 用户模块
namespace User {
bool signup(const QString& username, const QString& password) {
    QFile file(Utils::userFile);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
        return false;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(',');
        if (parts.size() >= 2 && parts[0] == username) {
            Utils::print("用户已存在");
            return false; // 用户已存在
        }
    }
    QTextStream out(&file);
    out << username << "," << password << "\n";
    return true;
}

bool login(QString& user,QString& username1,QString& password1) {
    QTextStream cin(stdin);
    QTextStream cout(stdout);
    cout.flush();
    QString username = username1;
    cout.flush();
    QString password = password1;

    QFile file(Utils::userFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(',');
        if (parts.size() >= 2 && parts[0] == username && parts[1] == password) {
            user = username;
            return true;
        }
    }
    return false;
}
}
namespace Session {
QString currentUser;

void setCurrentUser(const QString& user) {
    currentUser = user;
}

QString getCurrentUser() {
    return currentUser;
}
}
// DBMS模块
namespace DBMS {
void createDatabase(const QString& dbName) {
    QString path = Utils::dbRoot + "/" + currentUser + "/" + dbName;
    if (Utils::ensureDirExists(path)) {
        QTextStream cout(stdout);
        Utils::print("Database '" + dbName + "' created.\n");
        Utils::writeLog("Created database " + dbName);
    }
}

void dropDatabase(const QString& dbName) {
    QString path = Utils::dbRoot + "/" + currentUser + "/" + dbName;
    QDir dir(path);
    if (dir.exists()) {
        dir.removeRecursively();
        QTextStream cout(stdout);
        Utils::print("Database '" + dbName + "' dropped.\n");
        Utils::writeLog("Dropped database " + dbName);
        if (usingDatabase == dbName) {
            usingDatabase.clear();
        }
    }
}

void useDatabase(const QString& dbName) {
    QString path = Utils::dbRoot + "/" + currentUser + "/" + dbName;
    if (QDir(path).exists()) {
        usingDatabase = dbName;
        QTextStream cout(stdout);
       Utils::print ("Using database '" +dbName + "'.\n");
        Utils::writeLog("Using database " + dbName);
    } else {
        QTextStream cout(stdout);
       Utils::print ("[!] Database does not exist.\n");
    }
}

void createTable(const QString& tableName, const QString& columns) {
    if (usingDatabase.isEmpty()) {
        QTextStream cout(stdout);
        Utils::print( "[!] No database selected.\n");
        return;
    }
    QString path = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + tableName + ".txt";
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << columns << "\n";
        QTextStream cout(stdout);
       Utils::print ("Table '" + tableName + "' created.\n");
        Utils::writeLog("Created table " + tableName);
    }
}

void dropTable(const QString& tableName) {
    if (usingDatabase.isEmpty()) {
        QTextStream cout(stdout);
        Utils::print("[!] No database selected.\n");
        return;
    }
    QString path = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + tableName + ".txt";
    QFile file(path);
    if (file.exists()) {
        file.remove();
        QTextStream cout(stdout);
       Utils::print ("Table '" + tableName + "' dropped.\n");
        Utils::writeLog("Dropped table " + tableName);
    }
}

void insertInto(const QString& tableName, const QString& values) {
    if (usingDatabase.isEmpty()) {
        QTextStream cout(stdout);
       Utils::print ("[!] No database selected.\n");
        return;
    }
    QString path = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + tableName + ".txt";
    QFile file(path);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << values << "\n";
        QTextStream cout(stdout);
       Utils::print ("Inserted into '" + tableName + "'.\n");
        Utils::writeLog("Inserted into " + tableName);
    }
}

void selectFrom(const QString& tableName) {
    if (usingDatabase.isEmpty()) {
        QTextStream cout(stdout);
       Utils::print ("[!] No database selected.\n");
        return;
    }
    QString path = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + tableName + ".txt";
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QTextStream cout(stdout);
       Utils::print ("==== " + tableName + " ====\n");
        while (!in.atEnd()) {
           Utils::print( in.readLine() + "\n");
        }
       Utils::print ("====================\n");
        Utils::writeLog("Selected from " + tableName);
    }
}

void deleteFrom(const QString& tableName, const QString& condition) {
    if (usingDatabase.isEmpty()) {
        QTextStream cout(stdout);
        Utils::print ("[!] No database selected.\n");
        return;
    }
    QString path = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + tableName + ".txt";
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QStringList lines;
    QTextStream in(&file);
    QString header = in.readLine(); // 读取表头
    QStringList columns = header.split(',');
    int idx = -1;
    QString colName, colValue;
    QStringList parts = condition.split('=');
    if (parts.size() == 2) {
        colName = parts[0].trimmed();
        colValue = parts[1].trimmed();
        idx = columns.indexOf(colName);
    }
    if (idx == -1) {
        QTextStream cout(stdout);
        Utils::print( "[!] Invalid condition.\n");
        return;
    }
    lines.append(header);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList values = line.split(',');
        if (values.value(idx) != colValue) {
            lines.append(line);
        }
    }
    file.close();
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&file);
        for (const QString& l : lines) {
            out << l << "\n";
        }
    }
    QTextStream cout(stdout);
    Utils::print("Deleted matching rows from '" + tableName + "'.\n");
    Utils::writeLog("Deleted from " + tableName + " where " + condition);
}
}

// 解释器模块
namespace Interpreter {
void interpret(const QString& command) {
    QStringList tokens = Utils::splitCommand(command);
    if (tokens.isEmpty())
        return;
    QString op = tokens[0].toUpper();

    if (op == "HELP") {
        Utils::showHelp();
    }
    else if (op == "REGISTER" && tokens.size() >= 3) {
        if (User::signup(tokens[1], tokens[2])) {
            QTextStream cout(stdout);
           Utils::print ("Register success.\n");
        } else {
            QTextStream cout(stdout);
           Utils::print ("[!] Username exists.\n");
        }
    }
    else if (op == "LOGIN" && tokens.size() >= 3) {
        if (User::login(currentUser,tokens[1], tokens[2])) {
            QTextStream cout(stdout);
            Utils::print("LOGIN success. Welcome, " + currentUser + "\n");
            Session::setCurrentUser(currentUser);// 保存当前用户
             mainWindow->updateDirectoryView(currentUser);

        } else {
            QTextStream cout(stdout);
            Utils::print("[!] Login failed. Invalid username or password.\n");
        }
    }
    else if (op == "CREATE" && tokens.size() >= 3 && tokens[1].toUpper() == "DATABASE") {
        DBMS::createDatabase(tokens[2]);
    }
    else if (op == "DROP" && tokens.size() >= 3 && tokens[1].toUpper() == "DATABASE") {
        DBMS::dropDatabase(tokens[2]);
    }
    else if (op == "USE" && tokens.size() >= 2) {
        DBMS::useDatabase(tokens[1]);
    }
    else if (op == "CREATE" && tokens.size() >= 3 && tokens[1].toUpper() == "TABLE") {
        QString tableName = tokens[2];
        int start = command.indexOf('(');
        int end = command.indexOf(')');
        if (start != -1 && end != -1 && end > start) {
            QString cols = command.mid(start + 1, end - start - 1);
            DBMS::createTable(tableName, cols);
        }
    }
    else if (op == "DROP" && tokens.size() >= 3 && tokens[1].toUpper() == "TABLE") {
        DBMS::dropTable(tokens[2]);
    }
    else if (op == "INSERT" && tokens.size() >= 4 && tokens[1].toUpper() == "INTO") {
        QString tableName = tokens[2];
        int start = command.indexOf('(');
        int end = command.indexOf(')');
        if (start != -1 && end != -1 && end > start) {
            QString vals = command.mid(start + 1, end - start - 1);
            DBMS::insertInto(tableName, vals);
        }
    }
    else if (op == "SELECT" && tokens.size() >= 4 && tokens[1] == "*" && tokens[2].toUpper() == "FROM") {
        DBMS::selectFrom(tokens[3]);
    }
    else if (op == "DELETE" && tokens.size() >= 5 && tokens[1].toUpper() == "FROM") {
        QString tableName = tokens[2];
        QString condition = tokens.mid(4).join(' ');
        DBMS::deleteFrom(tableName, condition);
    }
    else {
        QTextStream cout(stdout);
       Utils::print ("[!] Unknown command.\n");
    }
}
}
