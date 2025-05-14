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
        out << "[" << time << "] " << currentUser << ": " << entry << "\n";
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
            else{
                QTextStream cout(stdout);
                qDebug()<<"error: Error key defined";
                Utils::print("[!] Error key defined'\n");
                return {"error",""};
            }
        }
        if(keys.contains('1')&&keys.contains('4')){
            QTextStream cout(stdout);
            qDebug()<<"[!] Error key conflict: "+info[0]+"\n";
            Utils::print("[!] Error key conflict: "+info[0]+"\n");
            return {"error",""};
        }
        if(keys.contains('1')&&keys.contains('2')){
            keys.remove('2');
        }
        if(keys.contains('1')&&keys.contains('3')){
            keys.remove('3');
        }
        return {keys,defaultValues};
    }
    else{
        return {"0",""};
    }
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
        return "2025-5-14";
    }
    if(colType=="FLOAT"||colType=="DOUBLE"){
        return "0.0";
    }
    return " ";
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
