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
// 全局变量
QString currentUser="guest";
Auth::Role currentRole;
QString usingDatabase;
QTextBrowser *showinShell = nullptr;
// 工具函数区:
extern MainWindow *mainWindow;
namespace Utils {
QString dbRoot = "D:/dbmssource";             // 所有数据库存放路径
QString userFile = "D:/dbmssource/users.txt"; // 用户信息存储文件
QString logFile = "D:/dbmssource/log.txt";    // 日志文件
void setOutputShell(QTextBrowser *shell)
{
    showinShell = shell;
}
void print(const QString &msg)
{
    if (showinShell) {
        showinShell->append(msg);
    } else {
        QTextStream(stdout) << msg << "\n";
    }
}
void writeLog(const QString &entry)
{
    QFile file(logFile);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        out << "[" << time << "] " << currentUser << ": " << entry << "\n";
        file.close();
    }
}

QStringList splitCommand(const QString &command)
{
    QRegularExpression regex("\\s+");
    return command.split(regex, Qt::SkipEmptyParts);
}

void showHelp()
{
    QTextStream cout(stdout);
    print( "========Commands========\n");
    print("GRANT username role  (ADMIN only)\n");
    print("ROLES 展示可赋予的权限\n");
    print("SHOWME 展示当前用户以及权限\n");
    print ("REGISTER username password\n");
    print ("LOGIN username password\n");
    print ("CREATE DATABASE dbname\n");
    print ("DROP DATABASE dbname\n");
    print("USE dbname\n");
    print("CREATE TABLE tablename (col1 type1,col2 type2,...)\n");
    print("DROP TABLE tablename\n");
    print ("INSERT INTO tablename VALUES (val1,val2,...)\n");
    print("SELECT col1,col2.... FROM tablename WHERE ...\n");
    print("DELETE FROM tablename WHERE col=value\n");
    print("ALTER TABLE tablename ADD/DROP/MODIFY columnname (type);\n");
    print("CREATE INDEX BTREE|HASH ON table(column) - 创建索引\n");
    print("DROP INDEX table.column - 删除索引\n");
    print("EXIT / QUIT\n\n");
}

bool ensureDirExists(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        return dir.mkpath(".");
    }
    return true;
}
QString formatAsTable(const QStringList &lines)
{
    if (lines.isEmpty())
        return "";

    // 1. 解析数据并检测数字列
    QList<QStringList> table;
    QList<bool> isNumericCol;

    // 第一行是表头（非数字列）
    if (!lines.isEmpty()) {
        QStringList header = lines.first().split(",", Qt::KeepEmptyParts);
        isNumericCol.fill(false, header.size());
        table.append(header);
    }

    // 检测数字列（从第二行开始）
    for (int i = 1; i < lines.size(); ++i) {
        QStringList fields = lines[i].split(",", Qt::KeepEmptyParts);
        table.append(fields);

        for (int j = 0; j < fields.size() && j < isNumericCol.size(); ++j) {
            if (!isNumericCol[j]) {
                bool ok;
                fields[j].trimmed().toDouble(&ok);
                isNumericCol[j] = ok;
            }
        }
    }

    // 2. 生成HTML表格
    QString html
        = "<table style='border-collapse: collapse; font-family: monospace; width: 100%;'>\n";

    // 表头行（加粗居中）
    html += "  <tr style='background-color: #f0f0f0;'>\n";
    for (int j = 0; j < table[0].size(); ++j) {
        html += QString("    <th style='border: 1px solid black; padding: 4px;'>%1</th>\n")
                    .arg(table[0][j].trimmed());
    }
    html += "  </tr>\n";

    // 数据行
    for (int i = 1; i < table.size(); ++i) {
        html += "  <tr>\n";
        for (int j = 0; j < table[i].size(); ++j) {
            QString align = (j < isNumericCol.size() && isNumericCol[j]) ? "right" : "left";
            QString content = table[i][j].trimmed().isEmpty() ? "&nbsp;" : table[i][j].trimmed();

            // 转义HTML特殊字符
            content.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;");

            html += QString("    <td style='border: 1px solid black; padding: 4px; text-align: "
                            "%1;'>%2</td>\n")
                        .arg(align)
                        .arg(content);
        }
        html += "  </tr>\n";
    }

    html += "</table>";
    return html;
}
} // namespace Utils

namespace Session {
QString currentUser;
Auth::Role currentRole;
void setCurrentUser(const QString& user,Auth::Role role) {
    currentUser = user;
    currentRole = role;
}


Auth::Role getCurrentRole(){
    return currentRole;
}
QString getCurrentUser(){
    return currentUser;
}
}
// 用户模块
namespace User {

bool signup(const QString& username, const QString& password) {
    const QString& role = "normal";

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
    out << username << "," << password << "," << role << "\n";
    file.close();
    QString path=Utils::dbRoot+"/"+username;
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

    QFile file(Utils::userFile);
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
    QFile file(Utils::userFile);
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
namespace DBMS {
void createDatabase(const QString &dbName)
{
    QString path = Utils::dbRoot + "/" + currentUser + "/" + dbName;
    QString typepath = Utils::dbRoot + "/" + currentUser + "/" + dbName + "/" + "DATATYPE";
    QString indexpath = Utils::dbRoot + "/" + currentUser + "/" + dbName + "/" + "INDEXES";

    if (Utils::ensureDirExists(path) &&
        Utils::ensureDirExists(typepath) &&
        Utils::ensureDirExists(indexpath)) {
        Utils::print("数据库 '" + dbName + "' 已创建.\n");
        Utils::writeLog("Created database " + dbName);
    }
}

void dropDatabase(const QString &dbName)
{
    QString path = Utils::dbRoot + "/" + currentUser + "/" + dbName;
    QDir dir(path);
    if (dir.exists()) {
        dir.removeRecursively();
        QTextStream cout(stdout);
        Utils::print("数据库 '" + dbName + "' 已删除.\n");
        Utils::writeLog("Dropped database " + dbName);
        if (usingDatabase == dbName) {
            usingDatabase.clear();
        }
    }
}

void useDatabase(const QString &dbName)
{
    QString path = Utils::dbRoot + "/" + currentUser + "/" + dbName;
    if (QDir(path).exists()) {
        usingDatabase = dbName;
        QTextStream cout(stdout);
        Utils::print("Using database '" + dbName + "'.\n");
        Utils::writeLog("Using database " + dbName);
    } else {
        QTextStream cout(stdout);

       Utils::print ("[!] 数据库不存在.\n");
    }
}

// void createTable(const QString& tableName, const QString& columns) {
//     if (usingDatabase.isEmpty()) {
//         QTextStream cout(stdout);
//         Utils::print( "[!] 未选择数据库.\n");
//         return;
//     }
//     QString path = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + tableName + ".txt";
//     QFile file(path);
//     if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
//         QTextStream out(&file);
//         out << columns << "\n";
//         QTextStream cout(stdout);
//        Utils::print ("表 '" + tableName + "' 已创建.\n");
//         Utils::writeLog("Created table " + tableName);
//     }
// }



void createTable(const QString &tableName, const QString &columns)
{

    if (usingDatabase.isEmpty()) {
        QTextStream cout(stdout);
        Utils::print("[!] 未选择数据库.\n");
        return;
    }
    //在这判断columns是否满足格式
    QRegularExpression regx(
        "\\s*[a-zA-Z_]\\w*\\s+"
        "(INT|VARCHAR|DATE|FLOAT|DOUBLE|BOOLEAN)"
        "(?:\\s+[a-zA-Z_0-9]\\w*)*"
        "(?:\\s*,\\s*[a-zA-Z_]\\w*\\s+"
        "(INT|VARCHAR|DATE|FLOAT|DOUBLE|BOOLEAN)"
        "(?:\\s+[a-zA-Z_0-9]\\w*)*)*",
        QRegularExpression::CaseInsensitiveOption
        );
    if(!regx.match(columns).hasMatch()){
        QTextStream cout(stdout);
        qDebug()<<"receive: " <<columns;
        Utils::print("[!] Invalid columns format. Expected format: 'name1 type1,name2 type2,...'\n");
        return;
    }
    QString path = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + tableName
                   + ".txt";
    QString path2 = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" +"DATATYPE/"+tableName
                    + "_data" +".txt";

    QFile file(path);
    QFile file2(path2);

    if (file.exists() || file2.exists()) {
        QTextStream cout(stdout);
        Utils::print("[!] Error: 表 '" + tableName + "' 已经存在.\n");
        return;
    }

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)&&
        file2.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        QTextStream out2(&file2);
        QStringList pairs = columns.split(",", Qt::SkipEmptyParts);
        QStringList firstColumnNames;
        QStringList secondColumnNames;
        QStringList allKEY;
        QStringList defaultValues;
        int primaryNums=0;
        for (const QString &pair : pairs) {
            QStringList parts = pair.trimmed().split(" ", Qt::SkipEmptyParts);
            if (parts.size() >= 1) {
                firstColumnNames << parts[0];
                secondColumnNames << parts[1];
            }
            if(parts.size()>2){
                QString keys="";
                for(int i=2;i<parts.size();i++){
                    if(parts[i].toUpper()=="PRIMARY"){
                        if(keys.contains('1')){
                            file.close();
                            file2.close();
                            dropTable(tableName);
                            QTextStream cout(stdout);
                            qDebug()<<"error: repeat key defined";
                            Utils::print("[!] repeat key defined'\n");
                            return;
                        }
                        keys += "1";
                        primaryNums++;
                        if(primaryNums>1){
                            QTextStream cout(stdout);
                            qDebug()<<"error: Multiple primary key defined";
                            Utils::print("[!] Multiple primary key defined'\n");
                            file.close();
                            file2.close();
                            dropTable(tableName);
                            return;
                        }
                    }
                    else if(parts[i].toUpper()=="DEFAULT"){
                        if(keys.contains('4')){
                            file.close();
                            file2.close();
                            dropTable(tableName);
                            QTextStream cout(stdout);
                            qDebug()<<"error: repeat key defined";
                            Utils::print("[!] repeat key defined'\n");
                            return;
                        }
                        keys += "4";
                        if(i+1>=parts.size()){
                            file.close();
                            file2.close();
                            dropTable(tableName);
                            QTextStream cout(stdout);
                            qDebug()<<"error: Error key defined";
                            Utils::print("[!] Error key defined'\n");
                            return;
                        }
                        defaultValues << parts[++i];
                    }
                    else if(parts[i].toUpper()=="NOT"){
                        if(keys.contains('2')){
                            file.close();
                            file2.close();
                            dropTable(tableName);
                            QTextStream cout(stdout);
                            qDebug()<<"error: repeat key defined";
                            Utils::print("[!] repeat key defined'\n");
                            return;
                        }
                        if(i+1>=parts.size()||parts[i+1].toUpper()!="NULL"){
                            file.close();
                            file2.close();
                            dropTable(tableName);
                            QTextStream cout(stdout);
                            qDebug()<<"error: Error key defined";
                            Utils::print("[!] Error key defined'\n");
                            return;
                        }
                        keys += "2";
                        i++;
                    }
                    else if(parts[i].toUpper()=="UNIQUE"){
                        if(keys.contains('3')){
                            file.close();
                            file2.close();
                            dropTable(tableName);
                            QTextStream cout(stdout);
                            qDebug()<<"error: repeat key defined";
                            Utils::print("[!] repeat key defined'\n");
                            return;
                        }
                        keys += "3";
                    }
                    else{
                        file.close();
                        file2.close();
                        dropTable(tableName);
                        QTextStream cout(stdout);
                        qDebug()<<"error: Error key defined";
                        Utils::print("[!] Error key defined'\n");
                        return;
                    }
                }
                if(keys.contains('1')&&keys.contains('4')){
                    file.close();
                    file2.close();
                    dropTable(tableName);
                    QTextStream cout(stdout);
                    qDebug()<<"[!] Error key conflict: "+parts[0]+"\n";
                    Utils::print("[!] Error key conflict: "+parts[0]+"\n");
                    return;
                }
                if(keys.contains('1')&&keys.contains('2')){
                    keys.remove('2');
                }
                if(keys.contains('1')&&keys.contains('3')){
                    keys.remove('3');
                }
                allKEY << keys;
            }
            else{
                allKEY << "0";
            }
        }
        out << firstColumnNames.join(",") << "\n";
        out2 << secondColumnNames.join(",")<< "\n";
        out2 << allKEY.join(",") << "\n";
        out2 << defaultValues.join(",") << "\n";
        file.close();
        file2.close();
        QTextStream cout(stdout);
        Utils::print("Table '" + tableName + "' created.\n");
        Utils::writeLog("Created table " + tableName);
    }
}

void dropTable(const QString &tableName)
{
    if (usingDatabase.isEmpty()) {
        QTextStream cout(stdout);
       Utils::print ("[!] 未选择数据库.\n");
        return;
    }
    QString path = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + tableName
                   + ".txt";
    QString path2 = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" +"DATATYPE/"+tableName
                    + "_data" +".txt";
    QFile file(path);
    QFile file2(path2);
    if (file.exists()) {
        file.remove();
        QTextStream cout(stdout);
        Utils::print("Table '" + tableName + "' dropped.\n");
        Utils::writeLog("Dropped table " + tableName);
    }
    if(file2.exists()){
        file2.remove();
    }
}

void insertInto(const QString &tableName, const QString &values)
{
    if (usingDatabase.isEmpty()) {
        QTextStream cout(stdout);
        Utils::print("[!] No database selected.\n");
        return;
    }
    QRegularExpression spaceSeparated("^\\s*\\w+\\s+\\w+\\s*(,\\s*\\w+\\s+\\w+\\s*)*$");
    QRegularExpression commaSeparated("^\\s*\\w+\\s*(,\\s*\\w+\\s*)*$");

    QString path = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + tableName
                   + ".txt";
    QString path2 = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" +"DATATYPE/"+tableName
                   + "_data.txt";
    QFile file2(path2);
    QFile file(path);
    QStringList cols;
    QStringList colsName;
    QStringList colsKeys;
    QStringList colDefault;
    QVector<QSet<QString>> columnSets;

    if (file2.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file2);
        QString firstline=in.readLine();
        QString secondline=in.readLine();
        QString thirdline=in.readLine();
        cols = firstline.split(",", Qt::SkipEmptyParts);
        colsKeys = secondline.split(",", Qt::SkipEmptyParts);
        colDefault = thirdline.split(",",Qt::SkipEmptyParts);
        file2.close();
    }
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString firstline=in.readLine();
        colsName = firstline.split(",", Qt::SkipEmptyParts);
        columnSets.resize(colsName.size());
        while (!in.atEnd()){
            QString getLine=in.readLine();
            QStringList contends=getLine.split(",", Qt::SkipEmptyParts);
            for(int col=0;col<contends.size();col++){
                columnSets[col].insert(contends[col]);
            }
            for(int col=0;col<contends.size();col++){
                if(columnSets[col].find("NULL")!=columnSets[col].end()){
                    columnSets[col].remove("NULL");
                }
            }
        }
        file.close();
    }

    //类似(typename value,typename value,...)的输入
    if(spaceSeparated.match(values).hasMatch()){
        QStringList pairs=values.split(",", Qt::SkipEmptyParts);
        if(pairs.size()>cols.size()){
            Utils::print("[!] Too many values inserted");
            return;
        }
        if (file.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&file);
            QStringList rowData;
            int defaultID=0;
            for (int i=0;i<cols.size();i++){
                bool hasValue = false;
                for (const QString &pair : pairs){
                    QStringList keyValue = pair.trimmed().split(" ", Qt::SkipEmptyParts);
                    if(keyValue[0]==colsName[i]){
                        if(keyValue[1].toUpper()=="NULL"){
                            keyValue[1]="NULL";
                        }
                        if(!colsKeys[i].contains("0")){
                            if(colsKeys[i].contains("1")){
                                if(columnSets[i].find(keyValue[1])!=columnSets[i].end()){
                                    Utils::print("[!] 主键"+colsName[i]+"不能拥有重复值");
                                    return;
                                }
                                if(keyValue[1]=="NULL"){
                                    Utils::print("[!] 主键"+colsName[i]+"不能为空值");
                                    return;
                                }
                            }
                            if(colsKeys[i].contains("2")){
                                if(keyValue[1]=="NULL"){
                                    Utils::print("[!] "+colsName[i]+"不能为空值");
                                    return;
                                }
                            }
                            if(colsKeys[i].contains("3")){
                                if(columnSets[i].find(keyValue[1])!=columnSets[i].end()){
                                    Utils::print("[!] "+colsName[i]+":不能拥有重复值");
                                    return;
                                }
                            }
                        }
                        rowData<<keyValue[1];
                        if(colsKeys[i].contains("4")){
                            defaultID++;
                        }
                        hasValue=true;
                        break;
                    }
                }
                if(!hasValue){
                    if(colsKeys[i].contains("2")||colsKeys[i].contains("1")){
                        Utils::print("[!] "+colsName[i]+"不能为空值");
                        return;
                    }
                    if(colsKeys[i].contains("4")){
                        rowData << colDefault[defaultID++];
                    }
                    else{
                        rowData <<"NULL";
                    }
                }
            }
            out << rowData.join(",")<<"\n";
            file.close();
            QTextStream cout(stdout);
            Utils::print("Inserted into '" + tableName + "'.\n");
            Utils::writeLog("Inserted into " + tableName);

            qint64 insertPos = file.pos() - rowData.join(",").length() - 1;
            file.close();

            // 更新所有索引
            QString indexPath = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/INDEXES/";
            QDir indexDir(indexPath);

            foreach (QString indexFile, indexDir.entryList(QStringList() << tableName + "_*.idx")) {
                QString columnName = indexFile.mid(tableName.length() + 1, indexFile.length() - tableName.length() - 5);
                int columnIndex = colsName.indexOf(columnName);

                if (columnIndex != -1) {
                    QString keyValue = rowData[columnIndex];
                    QString fullIndexPath = indexPath + indexFile;

                    QFile idxFile(fullIndexPath);
                    if (idxFile.open(QIODevice::Append | QIODevice::Text)) {
                        QTextStream out(&idxFile);
                        out << keyValue << ":" << insertPos << "\n";
                        idxFile.close();
                    }
                }
            }

            Utils::print("Inserted into '" + tableName + "'.\n");
            Utils::writeLog("Inserted into " + tableName);
        }
    }
    //类似(value,value,...)的输入
    else if(commaSeparated.match(values).hasMatch()){
        QStringList inValues=values.split(",", Qt::SkipEmptyParts);
        if(inValues.size()>cols.size()){
            Utils::print("[!] Too many values inserted");
            return;
        }
        if(inValues.size()<cols.size()){
            Utils::print("[!] Too less values inserted");
            return;
        }
        if (file.open(QIODevice::Append | QIODevice::Text)){
            QTextStream out(&file);
            QStringList rowData;
            for(int i=0;i<cols.size();i++){
                if(inValues[i].toUpper()=="NULL"){
                    inValues[i]="NULL";
                }
                if(!colsKeys[i].contains("0")){
                    if(colsKeys[i].contains("1")){
                        if(columnSets[i].find(inValues[i])!=columnSets[i].end()){
                            Utils::print("[!] 主键"+colsName[i]+"不能拥有重复值");
                            return;
                        }
                        if(inValues[i].toUpper()=="NULL"){
                            Utils::print("[!] 主键"+colsName[i]+"不能为空值");
                            return;
                        }
                    }
                    if(colsKeys[i].contains("2")){
                        if(inValues[i].toUpper()=="NULL"){
                            Utils::print("[!] "+colsName[i]+"不能为空值");
                            return;
                        }
                    }
                    if(colsKeys[i].contains("3")){
                        if(columnSets[i].find(inValues[i])!=columnSets[i].end()){
                            Utils::print("[!] "+colsName[i]+":不能拥有重复值");
                            return;
                        }
                    }
                }
                rowData << inValues[i];
            }
            out << rowData.join(",")<<"\n";
            file.close();
            QTextStream cout(stdout);
            Utils::print("Inserted into '" + tableName + "'.\n");
            Utils::writeLog("Inserted into " + tableName);
        }
    }
    else{
        Utils::print("[!] Invalid columns format. Expected format: 'name1 values1,name2 values2,...' or 'values1,values2,...'\n");
        return;
    }
}

void selectFrom(const QString &tableName)
{
    if (usingDatabase.isEmpty()) {
        Utils::print("[!] 未选择数据库.\n");
        return;
    }
    QString path = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + tableName
                   + ".txt";
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QStringList lines;
        while (!in.atEnd()) {
            lines.append(in.readLine());
        }
        Utils::print(Utils::formatAsTable(lines));
        Utils::writeLog("Selected from " + tableName);
    }
}
void selectAdvancedInternal(const QString &query, QStringList &result)
{
    QRegularExpression re(
        R"(SELECT\s+(.*?)\s+FROM\s+(\w+)\s*(?:JOIN\s+(\w+)\s+ON\s+(\w+)\.(\w+)\s*=\s*(\w+)\.(\w+))?(?:\s+WHERE\s+(.*?))?(?:\s+GROUP\s+BY\s+(.*?))?(?:\s+ORDER\s+BY\s+(.*?))?$)",
        QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatch match = re.match(query);

    if (!match.hasMatch()) {
        Utils::print("[!] 无效的 SELECT .");
        return;
    }

    QString fieldsStr = match.captured(1).trimmed(); // 选中的字段
    QString table1 = match.captured(2).trimmed();    // 主表
    QString joinTable = match.captured(3).trimmed(); // 被连接表
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
        QString path1 = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + table1
                        + ".txt";
        QString path2 = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + joinTable
                        + ".txt";
        Utils::print(path1 + "\n" + path2);
        QFile file1(path1), file2(path2);
        if (!file1.open(QIODevice::ReadOnly | QIODevice::Text) || !file2.open(QIODevice::ReadOnly | QIODevice::Text)) {
            Utils::print("[!] 打开用于JOIN的表失败.");
            return;
        }

        QTextStream in1(&file1), in2(&file2);
        QStringList header1 = in1.readLine().split(",");
        QStringList header2 = in2.readLine().split(",");
        QList<QStringList> rows1, rows2;

        while (!in1.atEnd())
            rows1.append(in1.readLine().split(","));
        while (!in2.atEnd())
            rows2.append(in2.readLine().split(","));
        int idx1 = header1.indexOf(leftField);
        int idx2 = header2.indexOf(rightField);

        QStringList joinedHeader = header1 + header2;

        rawLines.append(joinedHeader.join(","));  // 表头


        for (const QStringList &r1 : rows1) {
            for (const QStringList &r2 : rows2) {
                if (idx1 >= 0 && idx2 >= 0 && r1[idx1] == r2[idx2]) {
                    QStringList joinedRow = r1 + r2;
                    rawLines.append(joinedRow.join(","));  //  join 后加入
                }
            }
        }
    } else {
        QString path = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + table1
                       + ".txt";
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            Utils::print("[!] 无法打开表文件.");
            return;
        }

        QTextStream in(&file);
        QString header = in.readLine();
        rawLines.append(header);
        while (!in.atEnd()) {
            rawLines.append(in.readLine());
        }
    }

    if (!whereClause.isEmpty()) {
        QRegularExpression re(R"((\w+)\s*(<=|>=|=|<|>)\s*(.+))");
        QRegularExpressionMatch match = re.match(whereClause.trimmed());

        if (match.hasMatch() && match.captured(2) == "=") {
            QString col = match.captured(1).trimmed();
            QString val = match.captured(3).trimmed().replace("'", "").replace("\"", "");

            DBMS::IndexManager idxManager(currentUser, usingDatabase);
            if (idxManager.indexExists(table1, col)) {
                QVector<qint64> locations = idxManager.queryWithIndex(table1, col, val);

                if (!locations.isEmpty()) {
                    QString tablePath = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + table1 + ".txt";
                    QFile tableFile(tablePath);

                    if (tableFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                        QTextStream in(&tableFile);
                        QString header = in.readLine();
                        result.append(header);

                        for (qint64 pos : locations) {
                            tableFile.seek(pos);
                            result.append(in.readLine());
                        }

                        tableFile.close();
                        return;
                    }
                }
            }
        }
    }

    // WHERE 过滤
    if (!whereClause.isEmpty()) {
        QStringList filtered;
        QString headerLine = rawLines.takeFirst();
        QStringList header = headerLine.split(",");
        filtered.append(headerLine);

        QString col, val;

        QRegularExpression re(R"((\w+)\s*(<=|>=|=|<|>)\s*(.+))");
        QRegularExpressionMatch match = re.match(whereClause.trimmed());

        if (match.hasMatch()) {
            col = match.captured(1).trimmed();
            QString op = match.captured(2).trimmed();
            val = match.captured(3).trimmed();

            int index = header.indexOf(col);
            if (index == -1) {
                qWarning() << "Column not found:" << col;
            } else {
                for (const QString &line : rawLines) {
                    QStringList fields = line.split(",");
                    QString fieldVal = fields.value(index).trimmed();

                    // 首先尝试数字比较，回退到字符串比较
                    bool ok1, ok2;
                    double fieldNum = fieldVal.toDouble(&ok1);
                    double valNum = val.toDouble(&ok2);

                    bool match = false;
                    if (ok1 && ok2) {
                        if (op == "=")
                            match = (fieldNum == valNum);
                        else if (op == "<")
                            match = (fieldNum < valNum);
                        else if (op == ">")
                            match = (fieldNum > valNum);
                        else if (op == "<=")
                            match = (fieldNum <= valNum);
                        else if (op == ">=")
                            match = (fieldNum >= valNum);
                    } else {
                        if (op == "=")
                            match = (fieldVal == val);
                        else if (op == "<")
                            match = (fieldVal < val);
                        else if (op == ">")
                            match = (fieldVal > val);
                        else if (op == "<=")
                            match = (fieldVal <= val);
                        else if (op == ">=")
                            match = (fieldVal >= val);
                    }

                    if (match) {
                        filtered.append(line);
                    }
                }
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

        for (const QString &line : rawLines) {
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
        std::sort(rawLines.begin(), rawLines.end(), [sortIdx](const QString &a, const QString &b) {
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
                col = col.section('.', 1); // 去除 student. 或 people. 的前缀
            }
            int index = header.indexOf(col);
            if (index == -1) {
                Utils::print("[!] 在标题中未找到字段: " + col);
                continue;
            }
            selectedIndexes.append(index);
        }
    }
    QStringList formattedResult;
    for (const QString &line : rawLines) {
        QStringList fields = line.split(",");
        QStringList out;
        for (int idx : selectedIndexes)
            out.append(fields.value(idx));
        formattedResult.append(out.join(","));
    }

    result = formattedResult;
}
void selectAdvanced(const QString &command)
{
    if (usingDatabase.isEmpty()) {
        Utils::print("[!] 未选择数据库.");
        return;
    }

    // 简单解析：支持单个 SELECT 语句，支持 UNION、JOIN、WHERE、ORDER BY、GROUP BY
    QString query = command.trimmed();
    QStringList result;

    // UNION 查询
    if (query.contains("UNION", Qt::CaseInsensitive)) {
        QStringList parts = query.split(QRegularExpression("(?i)UNION"));
        QStringList allResults;
        QStringList header; // 用于保存统一的字段标题

        for (const QString &part : parts) {
            QString subQuery = part.trimmed();
            QStringList result;
            selectAdvancedInternal(subQuery, result);

            if (result.isEmpty()) {
                continue;
            }

            // 如果是第一次，记录字段标题
            if (header.isEmpty()) {
                header = result.first().split(",");
                allResults.append(result.first());
            }

            // 确保合并的每个结果都符合统一的字段标题
            for (int j = 1; j < result.size(); ++j) {
                QString line = result[j];
                QStringList fields = line.split(",");
                if (fields.size() == header.size()) {
                    allResults.append(line);
                }
            }
        }

        // 输出所有结果
        Utils::print("==== UNION RESULT ====");
        Utils::print(Utils::formatAsTable(allResults));
        return;
        return;
    }

    // 普通 SELECT
    selectAdvancedInternal(query, result);
    Utils::print("==== SELECT RESULT ====");
    Utils::print(Utils::formatAsTable(result));
}

void deleteFrom(const QString &tableName, const QString &condition)
{
    if (usingDatabase.isEmpty()) {
        QTextStream cout(stdout);

        Utils::print ("[!]未选择数据库.\n");

        return;
    }
    QString path = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + tableName
                   + ".txt";
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

        Utils::print( "[!] 无效状态.\n");

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
        for (const QString &l : lines) {
            out << l << "\n";
        }
    }
    QTextStream cout(stdout);
    Utils::print("于" + tableName +  "删除了对应的项 ''.\n");
    Utils::writeLog("Deleted from " + tableName + " where " + condition);
}
void headerManage(const QString command){

    if (usingDatabase.isEmpty()) {
        QTextStream cout(stdout);
        Utils::print("[!] No database selected.\n");
        return;
    }

    QRegularExpression regex(
        "ALTER\\s+TABLE\\s+(\\w+)\\s+"
        "(ADD|MODIFY|DROP)\\s+"
        "(\\w+)"
        "(?:\\s+(\\w+)(?:\\s+(.*))?)?"
        "(?=\\s*|$)",
        QRegularExpression::CaseInsensitiveOption
        );
    QRegularExpressionMatch match = regex.match(command);

    if (match.hasMatch()) {
        //处理指令
        QString tableName = match.captured(1);
        QString operation = match.captured(2);
        QString columnName = match.captured(3);
        QString extraInfo = match.captured(4);

        QString path = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + tableName
                       + ".txt";
        QString path2 = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + "DATATYPE/"+tableName
                        +"_data.txt";
        QFile file(path);
        QFile file2(path2);

        if (!file.exists()||!file2.exists()) {
            qDebug() << "Error: Table file does not exist";
            Utils::print("[!] Table file does not exist\n");
            return;
        }

        if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            qDebug() << "Failed to open file:" << path << "Error:" << file.errorString();
            return;
        }
        if (!file2.open(QIODevice::ReadWrite | QIODevice::Text)) {
            qDebug() << "Failed to open file:" << path2 << "Error:" << file2.errorString();
        }
        if (operation.toUpper() == "ADD") {

            if (extraInfo.toUpper() != "INT" &&
                extraInfo.toUpper() != "VARCHAR" &&
                extraInfo.toUpper() != "TEXT" &&
                extraInfo.toUpper() != "DATE" &&
                extraInfo.toUpper() != "FLOAT" &&
                extraInfo.toUpper() != "DOUBLE" &&
                extraInfo.toUpper() != "BOOLEAN"){

                Utils::print("[!] Error type\n");
                return;
            }
            //添加到列
            QTextStream in(&file);
            QString firstLine = in.readLine();
            QString rest = in.readAll();
            file.resize(0);

            QTextStream out(&file);
            if (firstLine.isEmpty()) {
                out << columnName << "\n";
            } else {
                out << firstLine << "," << columnName << "\n";
            }
            QStringList rows = rest.split("\n", Qt::SkipEmptyParts);
            for (QString &row : rows) {
                out << row << "," << "NULL" << "\n";
            }
            file.close();

            //添加到列定义
            QTextStream in2(&file2);
            QString firstLine2 = in2.readLine();
            QString rest2 = in2.readAll();
            file2.resize(0);

            QTextStream out2(&file2);
            if (firstLine2.isEmpty()) {
                out2 << extraInfo << "\n";
            } else {
                out2 << firstLine2 << "," << extraInfo << "\n";
            }
            out2 << rest2;
            file2.close();
        }
        else if (operation.toUpper() == "MODIFY") {
            if (extraInfo.toUpper() != "INT" &&
                extraInfo.toUpper() != "VARCHAR" &&
                extraInfo.toUpper() != "TEXT" &&
                extraInfo.toUpper() != "DATE" &&
                extraInfo.toUpper() != "FLOAT" &&
                extraInfo.toUpper() != "DOUBLE" &&
                extraInfo.toUpper() != "BOOLEAN"){

                Utils::print("[!] Error type\n");
                return;
            }
            QTextStream in(&file);
            QString columnLine = in.readLine();
            QStringList columns = columnLine.split(",", Qt::SkipEmptyParts);

            QTextStream in2(&file2);
            QString defLine = in2.readLine();
            QStringList definitions = defLine.split(",", Qt::SkipEmptyParts);

            int columnIndex = columns.indexOf(columnName);
            if (columnIndex == -1) {
                qDebug() << "Error: Column" << columnName << "not found in table";
                Utils::print("[!] Column '" + columnName + "' not found in table\n");
                return;
            }
            // 修改对应位置的列定义
            definitions[columnIndex] = extraInfo;
            QString rest = in2.readAll();
            file2.resize(0);
            file2.seek(0);
            QTextStream out2(&file2);
            out2 << definitions.join(",") << "\n";
            out2 << rest;
            file.close();
            file2.close();
        }
        else if (operation.toUpper() == "DROP") {
            // 读取file的所有内容
            QTextStream in(&file);
            QStringList fileLines;
            while (!in.atEnd()) {
                fileLines << in.readLine();
            }

            // 读取file2的所有内容
            QTextStream in2(&file2);
            QStringList file2Lines;
            while (!in2.atEnd()) {
                file2Lines << in2.readLine();
            }

            // 检查是否有数据
            if (fileLines.isEmpty() || file2Lines.isEmpty()) {
                qDebug() << "Error: Empty table file";
                Utils::print("[!] Empty table file\n");
                return;
            }

            QStringList columns = fileLines.first().split(",", Qt::SkipEmptyParts);
            int columnIndex = columns.indexOf(columnName);
            if (columnIndex == -1) {
                qDebug() << "Error: Column" << columnName << "not found in table";
                Utils::print("[!] Column '" + columnName + "' not found in table\n");
                return;
            }

            // 处理file的所有行
            for (int i = 0; i < fileLines.size(); ++i) {
                QStringList fields = fileLines[i].split(",", Qt::SkipEmptyParts);
                fields.removeAt(columnIndex);
                fileLines[i] = fields.join(",");
            }

            // 处理file2的所有行
            QStringList fields = file2Lines[0].split(",", Qt::SkipEmptyParts);
            fields.removeAt(columnIndex);
            file2Lines[0] = fields.join(",");
            for(int i=columnIndex+1;i<file2Lines.size()-1;i++){
                file2Lines[i]=file2Lines[i+1];
            }
            file2Lines.resize(file2Lines.size()-1);

            file.resize(0);
            file.seek(0);
            QTextStream out(&file);
            out << fileLines.join("\n");

            file2.resize(0);
            file2.seek(0);
            QTextStream out2(&file2);
            out2 << file2Lines.join("\n");

            qDebug() << "Column" << columnName << "dropped successfully";
            Utils::print("[+] Column '" + columnName + "' dropped successfully\n");

        }
        Utils::print("Alter table: " + tableName + operation + columnName +extraInfo);
        Utils::writeLog("Alter table: " + tableName + operation + columnName +extraInfo);
    }else {
        qDebug() << "Invalid ALTER TABLE syntax";
       }
}
IndexManager::IndexManager(const QString& user, const QString& db)
    : currentUser(user), usingDatabase(db) {
    QString indexPath = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/INDEXES";
    QDir().mkpath(indexPath);
}

QString IndexManager::getIndexPath(const QString& tableName, const QString& columnName) {
    return Utils::dbRoot + "/" + currentUser + "/" + usingDatabase +
           "/INDEXES/" + tableName + "_" + columnName + ".idx";
}

bool IndexManager::createIndex(const QString& tableName, const QString& columnName, const QString& indexType) {
    QString indexPath = getIndexPath(tableName, columnName);
    QFile indexFile(indexPath);

    if (indexFile.exists()) {
        Utils::print("[!] 索引已存在");
        return false;
    }

    if (indexFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&indexFile);
        out << "INDEX_TYPE:" << indexType << "\n";
        out << "TABLE:" << tableName << "\n";
        out << "COLUMN:" << columnName << "\n";
        indexFile.close();
        return buildIndex(tableName, columnName, indexType);
    }
    return false;
}

bool IndexManager::buildIndex(const QString& tableName, const QString& columnName, const QString& indexType) {
    QString tablePath = Utils::dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + tableName + ".txt";
    QFile tableFile(tablePath);

    if (!tableFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&tableFile);
    QStringList headers = in.readLine().split(",");
    int columnIndex = headers.indexOf(columnName);

    if (columnIndex == -1) {
        Utils::print("[!] 表中未发现字段");
        return false;
    }

    QString indexPath = getIndexPath(tableName, columnName);
    QFile indexFile(indexPath);

    if (!indexFile.open(QIODevice::Append | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&indexFile);
    qint64 lineOffset = tableFile.pos();

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList values = line.split(",");
        QString key = values.value(columnIndex).trimmed();
        out << key << ":" << lineOffset << "\n";
        lineOffset = tableFile.pos();
    }

    indexFile.close();
    tableFile.close();
    Utils::print("[+] 成功创建索引");
    return true;
}

QVector<qint64> IndexManager::queryWithIndex(const QString& tableName, const QString& columnName, const QString& key) {
    QVector<qint64> results;
    QString indexPath = getIndexPath(tableName, columnName);
    QFile indexFile(indexPath);

    if (!indexFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return results;
    }

    indexFile.readLine(); // Skip metadata
    while (!indexFile.atEnd()) {
        QString line = indexFile.readLine();
        QStringList parts = line.split(":");
        if (parts.size() == 2 && parts[0].trimmed() == key) {
            results.append(parts[1].trimmed().toLongLong());
        }
    }

    indexFile.close();
    return results;
}

bool IndexManager::dropIndex(const QString& tableName, const QString& columnName) {
    QString indexPath = getIndexPath(tableName, columnName);
    if (QFile::remove(indexPath)) {
        Utils::print("[+] 删除索引成功");
        return true;
    }
    Utils::print("[!] 删除索引失败");
    return false;
}

bool IndexManager::indexExists(const QString& tableName, const QString& columnName) {
    QString indexPath = getIndexPath(tableName, columnName);
    return QFile::exists(indexPath);
}

} // namespace DBMS

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
            QFile file(Utils::dbRoot+currentUser);
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
