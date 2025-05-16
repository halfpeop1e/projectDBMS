#include "toolfunction.h"
#include "globals.h"
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
namespace Utils {
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
        out<< "[" << time << "] " << currentUser << ": " << entry << "\n";
        file.close();
    }
}

QStringList splitCommand(const QString &command)
{
    QRegularExpression regex("\\s+");
    return command.split(regex, Qt::SkipEmptyParts);
}

QStringList readKeysInfor(const QStringList info,int& primaryNums){
    QString defaultValues;
    if(!info.isEmpty()){
        QString keys="";
        for(int i=0;i<info.size();i++){
            if(info[i].toUpper()=="PRIMARY"){
                if(keys.contains('1')){
                    QTextStream cout(stdout);
                    qDebug()<<"error: repeat key defined";
                    Utils::print("[!] repeat key defined'\n");
                    return {"error",""};
                }
                keys += "1";
                primaryNums++;
                if(primaryNums>1){
                    QTextStream cout(stdout);
                    qDebug()<<"error: Multiple primary key defined";
                    Utils::print("[!] Multiple primary key defined'\n");
                    return {"error",""};
                }
            }
            else if(info[i].toUpper()=="DEFAULT"){
                if(keys.contains('4')){
                    QTextStream cout(stdout);
                    qDebug()<<"error: repeat key defined";
                    Utils::print("[!] repeat key defined'\n");
                    return {"error",""};
                }
                keys += "4";
                if(i+1>=info.size()){
                    QTextStream cout(stdout);
                    qDebug()<<"error: Error key defined";
                    Utils::print("[!] Error key defined'\n");
                    return {"error",""};
                }
                defaultValues =info[++i];
            }
            else if(info[i].toUpper()=="NOT"){
                if(keys.contains('2')){
                    QTextStream cout(stdout);
                    qDebug()<<"error: repeat key defined";
                    Utils::print("[!] repeat key defined'\n");
                    return {"error",""};
                }
                if(i+1>=info.size()||info[i+1].toUpper()!="NULL"){
                    QTextStream cout(stdout);
                    qDebug()<<"error: Error key defined";
                    Utils::print("[!] Error key defined'\n");
                    return {"error",""};
                }
                keys += "2";
                i++;
            }
            else if(info[i].toUpper()=="UNIQUE"){
                if(keys.contains('3')){
                    QTextStream cout(stdout);
                    qDebug()<<"error: repeat key defined";
                    Utils::print("[!] repeat key defined'\n");
                    return {"error",""};
                }
                keys += "3";
            }
            else if(info[i].toUpper()=="IDENTITY"){
                if(keys.contains('5')){
                    QTextStream cout(stdout);
                    qDebug()<<"error: repeat key defined";
                    Utils::print("[!] repeat key defined'\n");
                    return {"error",""};
                }
                keys += "5";
            }
            else{
                QTextStream cout(stdout);
                qDebug()<<"error: Error key defined";
                Utils::print("[!] Error key defined'\n");
                return {"error",""};
            }
        }
        // if(keys.contains('1')&&keys.contains('4')){
        //     QTextStream cout(stdout);
        //     qDebug()<<"[!] Error key conflict: "+info[0]+"\n";
        //     Utils::print("[!] Error key conflict: "+info[0]+"主键和default约束冲突 \n");
        //     return {"error",""};
        // }
        if(keys.contains('4')&&keys.contains('5')){
            QTextStream cout(stdout);
            qDebug()<<"[!] Error key conflict: "+info[0]+"\n";
            Utils::print("[!] Error key conflict: "+info[0]+"默认和自增约束冲突 \n");
            return {"error",""};
        }
        if(keys.contains('1')&&keys.contains('2')){
            keys.remove('2');
        }
        if(keys.contains('1')&&keys.contains('3')){
            keys.remove('3');
        }
        if(keys.contains('5')&&keys.contains('3')){
            keys.remove('3');
        }
        if(keys.contains('5')&&keys.contains('2')){
            keys.remove('2');
        }
        return {keys,defaultValues};
    }
    else{
        return {"0",""};
    }
}
bool checkColValue(QString type,int index,QString filePath){
    if(type.toUpper()!="INT"&&type.toUpper()!="VARCHAR"&&type.toUpper()!="DATE"&&type.toUpper()!="FLOAT"
        &&type.toUpper()!="DOUBLE"&&type.toUpper()!="BOOLEN"){
        return false;
    }
    QFile file(filePath);
    if(!file.exists()){
        return false;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream in(&file);
    in.readLine();
    while(!in.atEnd()){
        QString line = in.readLine();
        QStringList cols =line.split(",");
        if(cols[index].toUpper()=="NULL"){
            continue;
        }
        if(type.toUpper()=="INT"){
            bool ok;
            cols[index].toInt(&ok);
            if(!ok){
                return false;
            }
        }
        else if(type.toUpper()=="DATE"){
            QRegularExpression regex("^\\d{4}-(0[1-9]|1[0-2])-(0[1-9]|[12][0-9]|3[01])$");
            if(!regex.match(cols[index]).hasMatch()){
                return false;
            }
        }
        else if(type.toUpper()=="FLOAT"){
            bool ok;
            cols[index].toFloat(&ok);
            if(!ok){
                return false;
            }
        }
        else if(type.toUpper()=="DOUBLE"){
            bool ok;
            cols[index].toDouble(&ok);
            if(!ok){
                return false;
            }
        }
        else if(type.toUpper()=="BOOLEN"){
            if(cols[index]!="0"&&cols[index]!="1"){
                return false;
            }
        }
    }
    return true;
}
bool checkColValue(QString type,QString value){
    if(type.toUpper()!="INT"&&type.toUpper()!="VARCHAR"&&type.toUpper()!="DATE"&&type.toUpper()!="FLOAT"
        &&type.toUpper()!="DOUBLE"&&type.toUpper()!="BOOLEN"){
        return false;
    }
    if(type.toUpper()=="INT"){
        bool ok;
        value.toInt(&ok);
        if(!ok){
            return false;
        }
    }
    else if(type.toUpper()=="DATE"){
        QRegularExpression regex("^\\d{4}-(0[1-9]|1[0-2])-(0[1-9]|[12][0-9]|3[01])$");
        if(!regex.match(value).hasMatch()){
            return false;
        }
    }
    else if(type.toUpper()=="FLOAT"){
        bool ok;
        value.toFloat(&ok);
        if(!ok){
            return false;
        }
    }
    else if(type.toUpper()=="DOUBLE"){
        bool ok;
        value.toDouble(&ok);
        if(!ok){
            return false;
        }
    }
    else if(type.toUpper()=="BOOLEN"){
        if(value!="0"&&value!="1"){
            return false;
        }
    }
    return true;
}
QString getDefaultValue(const QString type){
    //INT|VARCHAR|DATE|FLOAT|DOUBLE|BOOLEAN
    QString colType = type.toUpper();
    if(colType=="INT"||colType=="BOOLEAN"){
        return "0";
    }
    if(colType=="VARCHAR"){
        return " ";
    }
    if(colType=="DATE"){
        return "2025-05-14";
    }
    if(colType=="FLOAT"||colType=="DOUBLE"){
        return "0.0";
    }
    return " ";
}
bool checkDeletePK(QString tablename,int index,QString value){
    QString fkpath = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + "DATATYPE/"
                                                                              "fk_data.txt";
    QFile fkFile(fkpath);
    if(fkFile.open(QIODevice::ReadOnly|QIODevice::Text)){
        QTextStream in(&fkFile);
        while (!in.atEnd()) {
            QString line=in.readLine();
            QStringList list =line.split(",");
            if(list[3]==tablename&&list[4]==QString::number(index)){
                QString path = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + list[1]
                               + ".txt";
                QFile file(path);
                QSet<QString> records;
                if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
                    QTextStream in2(&file);
                    while(!in2.atEnd()){
                        QString line2=in2.readLine();
                        QStringList list2 =line2.split(",");
                        records.insert(list2[list[2].toInt()]);
                    }
                    file.close();
                    if(records.contains(value)){
                        return true;
                    }
                    return false;
                }
            }
        }
        fkFile.close();
    }
    return false;
}
bool checkInsertPK(QString tablename,int index,QString value){
    QString fkpath = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + "DATATYPE/"
                                                                          "fk_data.txt";
    QFile fkFile(fkpath);
    if(fkFile.open(QIODevice::ReadOnly|QIODevice::Text)){
        QTextStream in(&fkFile);
        while (!in.atEnd()) {
            QString line=in.readLine();
            QStringList list =line.split(",");
            if(list[1]==tablename&&list[2]==QString::number(index)){
                QString path = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + list[3]
                               + ".txt";
                QFile file(path);
                QSet<QString> records;
                if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
                     QTextStream in2(&file);
                    while(!in2.atEnd()){
                         QString line2=in2.readLine();
                         QStringList list2 =line2.split(",");
                         records.insert(list2[list[4].toInt()]);
                    }
                    file.close();
                    if(records.contains(value)){
                        return true;
                    }
                    return false;
                }
            }
        }
        fkFile.close();
    }
    return false;
}
void checkIdentity(QString tableName){
    QString path = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + tableName
                   + ".txt";
    QString path2 = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" +"DATATYPE/"+tableName
                    + "_data.txt";
    QFile file2(path2);
    QStringList colsKeys;
    QSet<int> indentityIdex;
    int identityValue=0;
    if (file2.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file2);
        in.readLine();
        QString secondline=in.readLine();
        colsKeys = secondline.split(",");
        for(int i=0;i<colsKeys.size();i++){
            if(colsKeys[i].contains("5")){
                indentityIdex.insert(i);
            }
        }
        file2.close();
    }
    if(indentityIdex.empty()){
        return;
    }
    QFile file(path);
    QStringList outRes;
    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        QTextStream in(&file);
        QTextStream out(&file);
        outRes << in.readLine();
        while (!in.atEnd()){
            identityValue++;
            QString getLine=in.readLine();
            QStringList contends=getLine.split(",");
            for(int col=0;col<contends.size();col++){
                if(indentityIdex.contains(col)){
                    contends[col]=QString::number(identityValue);
                }
            }
            outRes << contends.join(",");
        }
        file.resize(0);
        file.seek(0);
        out << outRes.join('\n');
        file.close();
    }
}
void showHelp()
{
    QTextStream cout(stdout);
    Utils::print(R"(
===== 数据库管理系统帮助文档 =====;
 "以下是支持的命令格式及其说明：";

 "[用户操作]";
1."REGISTER <username> <password> -- 注册新用户";
2."LOGIN <username> <password>   -- 用户登录";
3."SHOWME      -- 显示当前用户及权限";
4."ROLES       -- 显示所有可用权限";
5."GRANT <username> <role>     -- 赋予用户角色（需ADMIN权限";

6."[数据库操作] (需 ADMIN 权限)\n";
7."CREATE DATABASE <db_name>  -- 创建数据库";
8."DROP DATABASE <db_name>    -- 删除数据库";
9."USE <db_name>              -- 使用指定数据库";

 "[数据表操作] (需 ADMIN 权限)'";
10."CREATE TABLE <table_name> (<col1 TYPE, col2 TYPE, ...>)'";
          -- 创建数据表，列名和类型用逗号分隔";
11."DROP TABLE <table_name>      -- 删除数据表";
12."ALTER TABLE <...>     -- 修改表结构";

 "[数据操作] (需 ADMIN 权限)'";
13."INSERT INTO <table_name> (<val1, val2, ...>)'";
 "                    -- 向表中插入一行数据\n";
14."UPDATE <table_name> SET col1=val1, col2=val2 [WHERE condition]";
 "                    -- 更新表中记录，可加 WHERE 条件";
15."DELETE FROM <table_name> WHERE condition";
 "                    -- 删除满足条件的记录";

 "[查询操作]";
16."SELECT <columns> FROM <table> [WHERE condition] [ORDER BY ...] [GROUP BY ...] [JOIN ...] [UNION ...]";
 "                 -- 查询数据，支持多种子句和联合查询\n\n";

 "[索引操作] (需 ADMIN 权限)\n";
17."CREATE INDEX <BTREE|HASH> ON <table>(<column>)'";
 "                         -- 为表某列创建 BTREE 或 HASH 索引";
18."DROP INDEX <table>.<column>      -- 删除某列上的索引";
19.BACKUP DATABASE <dbname> <PATH>
20.RESTORE DATABASE <dbname> <PATH>

 "=======================================)");
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
