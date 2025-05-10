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
// 工具函数区:
extern MainWindow *mainWindow;
namespace Utils {
QString dbRoot = "/Users/fengzhu/Resource"; // 所有数据库存放路径
QString userFile = "/Users/fengzhu/Resource/users.txt"; // 用户信息存储文件
QString logFile = "/Users/fengzhu/Resource/log.txt"; // 日志文件
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
    out << username << "," << password << "\n";
    file.close();
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
void selectAdvancedInternal(const QString& query, QStringList& result) {
    QRegularExpression re(R"(SELECT\s+(.*?)\s+FROM\s+(\w+)\s*(?:JOIN\s+(\w+)\s+ON\s+(\w+)\.(\w+)\s*=\s*(\w+)\.(\w+))?(?:\s+WHERE\s+(.*?))?(?:\s+GROUP\s+BY\s+(.*?))?(?:\s+ORDER\s+BY\s+(.*?))?$)", QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatch match = re.match(query);

    if (!match.hasMatch()) {
        Utils::print("[!] Invalid SELECT syntax.");
        Utils::print("[DEBUG] Full Query: " + query);
        return;
    }

    QString fieldsStr = match.captured(1).trimmed();  // 选中的字段
    QString table1 = match.captured(2).trimmed();     // 主表
    QString joinTable = match.captured(3).trimmed();  // 被连接表
    QString leftTable = match.captured(4).trimmed();
    QString leftField = match.captured(5).trimmed();
    QString rightTable = match.captured(6).trimmed();
    QString rightField = match.captured(7).trimmed();
    QString whereClause = match.captured(8).trimmed();
    QString groupByClause = match.captured(9).trimmed();
    QString orderByClause = match.captured(10).trimmed();

    QStringList rawLines;

    if (!joinTable.isEmpty()) {
        // 加载两张表
        QString path1 = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + table1 + ".txt";
        QString path2 = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + joinTable + ".txt";
        Utils::print(path1+"\n"+path2);
        QFile file1(path1), file2(path2);
        if (!file1.open(QIODevice::ReadOnly | QIODevice::Text) || !file2.open(QIODevice::ReadOnly | QIODevice::Text)) {
            Utils::print("[!] Failed to open tables for JOIN.");
            return;
        }

        QTextStream in1(&file1), in2(&file2);
        QStringList header1 = in1.readLine().split(",");
        QStringList header2 = in2.readLine().split(",");
        QList<QStringList> rows1, rows2;


        while (!in1.atEnd()) rows1.append(in1.readLine().split(","));
        while (!in2.atEnd()) rows2.append(in2.readLine().split(","));
        Utils::print("[DEBUG] Students file content: ");
        Utils::print(header1.join(","));
        for (const QStringList& row : rows1) {
            Utils::print(row.join(","));
        }

        Utils::print("[DEBUG] People file content: ");
        Utils::print(header2.join(","));
        for (const QStringList& row : rows2) {
            Utils::print(row.join(","));
        }
        Utils::print("[DEBUG] Matching students.id = people.id");
        for (const QStringList& r1 : rows1) {
            for (const QStringList& r2 : rows2) {
                if (r1[header1.indexOf("id")] == r2[header2.indexOf("id")]) {
                    Utils::print("[DEBUG] Match found: " + r1.join(",") + " + " + r2.join(","));
                }
            }
        }
        int idx1 = header1.indexOf(leftField);
        int idx2 = header2.indexOf(rightField);

        QStringList joinedHeader = header1 + header2;
        rawLines.append(joinedHeader.join(","));  // 表头 OK

        for (const QStringList& r1 : rows1) {
            for (const QStringList& r2 : rows2) {
                if (idx1 >= 0 && idx2 >= 0 && r1[idx1] == r2[idx2]) {
                    QStringList joinedRow = r1 + r2;
                    rawLines.append(joinedRow.join(","));  // ✅ join 后加入
                }
            }
        }
    } else {
        QString path = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + table1 + ".txt";
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            Utils::print("[!] Cannot open table file.");
            return;
        }

        QTextStream in(&file);
        QString header = in.readLine();
        rawLines.append(header);
        while (!in.atEnd()) {
            rawLines.append(in.readLine());
        }
    }

    // WHERE 过滤
    if (!whereClause.isEmpty()) {
        QStringList filtered;
        QString headerLine = rawLines.takeFirst();
        QStringList header = headerLine.split(",");
        filtered.append(headerLine);

        QString col, val;
        if (whereClause.contains("=")) {
            QStringList cond = whereClause.split("=");
            col = cond[0].trimmed();
            val = cond[1].trimmed();
        }

        int index = header.indexOf(col);
        for (const QString& line : rawLines) {
            QStringList fields = line.split(",");
            if (fields.value(index).trimmed() == val) {
                filtered.append(line);
            }
        }
        rawLines = filtered;
    }

    // GROUP BY（简单实现，仅支持 COUNT 聚合）
    if (!groupByClause.isEmpty()) {
        QStringList grouped;
        QStringList header = rawLines.takeFirst().split(",");
        int groupIdx = header.indexOf(groupByClause);
        QMap<QString, int> groupCount;

        for (const QString& line : rawLines) {
            QString key = line.split(",").value(groupIdx);
            groupCount[key]++;
        }

        grouped.append(groupByClause + ",COUNT");
        for (auto it = groupCount.begin(); it != groupCount.end(); ++it) {
            grouped.append(it.key() + "," + QString::number(it.value()));
        }
        rawLines = grouped;
    }

    // ORDER BY（升序排序）
    if (!orderByClause.isEmpty()) {
        QString headerLine = rawLines.takeFirst();
        QStringList header = headerLine.split(",");
        int sortIdx = header.indexOf(orderByClause);
        std::sort(rawLines.begin(), rawLines.end(), [sortIdx](const QString& a, const QString& b) {
            return a.split(",").value(sortIdx) < b.split(",").value(sortIdx);
        });
        rawLines.prepend(headerLine);
    }

    // 字段选择
    QStringList header = rawLines.first().split(",");
    QList<int> selectedIndexes;

    if (fieldsStr == "*") {
        for (int i = 0; i < header.size(); ++i)
            selectedIndexes.append(i);
    } else {
        QStringList wanted = fieldsStr.split(",");
        for (QString col : wanted) {
            col = col.trimmed();
            if (col.contains(".")) {
                col = col.section('.', 1);  // 去除 student. 或 people. 的前缀
            }
            int index = header.indexOf(col);
            if (index == -1) {
                Utils::print("[!] Field not found in header: " + col);
                continue;
            }
            selectedIndexes.append(index);
        }
    }

    for (const QString& line : rawLines) {
        QStringList fields = line.split(",");
        QStringList out;
        for (int idx : selectedIndexes)
            out.append(fields.value(idx));
        result.append(out.join(","));
    }
}
void selectAdvanced(const QString& command) {
    if (usingDatabase.isEmpty()) {
        Utils::print("[!] No database selected.");
        return;
    }

    // 简单解析：支持单个 SELECT 语句，支持 UNION、JOIN、WHERE、ORDER BY、GROUP BY
    QString query = command.trimmed();

    // UNION 查询
    if (query.contains("UNION", Qt::CaseInsensitive)) {
        QStringList parts = query.split(QRegularExpression("(?i)UNION"));
        QStringList allResults;
        QStringList header; // 用于保存统一的字段标题

        for (const QString& part : parts) {
            QString subQuery = part.trimmed();
            QStringList result;
            selectAdvancedInternal(subQuery, result);

            if (result.isEmpty()) {
                continue;
            }

            // 如果是第一次，记录字段标题
            if (header.isEmpty()) {
                header = result.first().split(",");
            }

            // 确保合并的每个结果都符合统一的字段标题
            for (const QString& line : result) {
                QStringList fields = line.split(",");
                if (fields.size() == header.size()) {
                    allResults.append(line);
                }
            }
        }

        // 输出所有结果
        Utils::print("==== UNION RESULT ====");
        for (const QString& line : allResults)
            Utils::print(line);
        return;
    }

    // 普通 SELECT
    QStringList result;
    selectAdvancedInternal(query, result);
    Utils::print("==== SELECT RESULT ====");
    for (const QString& line : result)
        Utils::print(line);
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
        mainWindow->updateDirectoryView(currentUser+"/"+tokens[1]);
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
    else if (op == "SELECT") {
        DBMS::selectAdvanced(command);
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
