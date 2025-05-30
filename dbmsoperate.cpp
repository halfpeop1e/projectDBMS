#include "dbmsoperate.h"
#include "globals.h"
#include"toolfunction.h"
#include "mainwindow.h"
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

namespace DBMS {
void createDatabase(const QString &dbName)
{
    QString path = dbRoot + "/" + currentUser + "/" + dbName;
    QString typepath = dbRoot + "/" + currentUser + "/" + dbName + "/" + "DATATYPE";
    QString indexpath = dbRoot + "/" + currentUser + "/" + dbName + "/" + "INDEXES";

    if (Utils::ensureDirExists(path) &&
        Utils::ensureDirExists(typepath) &&
        Utils::ensureDirExists(indexpath)) {
        Utils::print("数据库 '" + dbName + "' 已创建.\n");
        Utils::writeLog("Created database " + dbName);
    }
}

void dropDatabase(const QString &dbName)
{
    QString path = dbRoot + "/" + currentUser + "/" + dbName;
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
    QString path = dbRoot + "/" + currentUser + "/" + dbName;
    if (QDir(path).exists()) {
        usingDatabase = dbName;
        mainWindow->on_currentusingdb_textChanged(usingDatabase);
        QTextStream cout(stdout);
        Utils::print("正在使用'" + dbName + "'.\n");
        Utils::writeLog("Using database " + dbName);
    } else {
        QTextStream cout(stdout);

        Utils::print ("[!] 数据库不存在.\n");
    }
}


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
        Utils::print("[!] 错误的字段格式. 期望格式为: 'name1 type1,name2 type2,...'\n");
        return;
    }
    QString path = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + tableName
                   + ".txt";
    QString path2 = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" +"DATATYPE/"+tableName
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
        QStringList firstColumnNames; //列名
        QStringList secondColumnNames;//列类型
        QStringList allKEY;//约束值
        QStringList defaultValues;//默认值
        int primaryKeyNums=0;
        for (const QString &pair : pairs) {
            QStringList parts = pair.trimmed().split(" ", Qt::SkipEmptyParts);
            if (parts.size() >= 2) {
                firstColumnNames << parts[0];
                secondColumnNames << parts[1];
            }
            if(parts.size() > 2){
                QStringList info = parts.mid(2);
                QStringList getInfo = Utils::readKeysInfor(info,primaryKeyNums);
                if(getInfo[0]=="error"){
                    file.close();
                    file2.close();
                    dropTable(tableName);
                    return;
                }
                allKEY << getInfo[0];
                defaultValues << getInfo[1];
            }
            else{
                allKEY << "0";
                defaultValues << "";
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
    QString path = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + tableName
                   + ".txt";
    QString path2 = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" +"DATATYPE/"+tableName
                    + "_data" +".txt";
    QFile file(path);
    QFile file2(path2);
    if (file.exists()) {
        file.remove();
        QTextStream cout(stdout);
        Utils::print("表 '" + tableName + "' 已删除.\n");
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
        Utils::print("[!]未选择数据库.\n");
        return;
    }
    QRegularExpression spaceSeparated(
        "^\\s*"
        "(\\d+|\\d+\\.\\d+|\\d{4}-\\d{2}-\\d{2}|\\w+)"
        "\\s+"
        "(\\d+|\\d+\\.\\d+|\\d{4}-\\d{2}-\\d{2}|\\w+)"
        "\\s*"
        "(,\\s*"
        "(\\d+|\\d+\\.\\d+|\\d{4}-\\d{2}-\\d{2}|\\w+)"
        "\\s+"
        "(\\d+|\\d+\\.\\d+|\\d{4}-\\d{2}-\\d{2}|\\w+)"
        "\\s*)*$"
        );
    QRegularExpression commaSeparated(
        "^\\s*"
        "(\\d+|(\\d+\\.\\d+)|(\\d{4}-\\d{2}-\\d{2})|\\w+)"
        "\\s*"
        "(,\\s*"
        "(\\d+|(\\d+\\.\\d+)|(\\d{4}-\\d{2}-\\d{2})|\\w+)"
        "\\s*)*$"
        );

    QString path = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + tableName
                   + ".txt";
    QString path2 = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" +"DATATYPE/"+tableName
                    + "_data.txt";
    QFile file2(path2);
    QFile file(path);
    QStringList cols;
    QStringList colsName;
    QStringList colsKeys;
    QStringList colDefault;
    int identityNums=0;
    int identityValue=0;
    QVector<QSet<QString>> columnSets;

    if (file2.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file2);
        QString firstline=in.readLine();
        QString secondline=in.readLine();
        QString thirdline=in.readLine();
        cols = firstline.split(",", Qt::SkipEmptyParts);
        colsKeys = secondline.split(",", Qt::SkipEmptyParts);
        colDefault = thirdline.split(",",Qt::SkipEmptyParts);
        for(auto i:colsKeys){
            if(i.contains("5")){
                identityNums++;
            }
        }
        file2.close();
    }
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString firstline=in.readLine();
        colsName = firstline.split(",", Qt::SkipEmptyParts);
        columnSets.resize(colsName.size());
        while (!in.atEnd()){
            identityValue++;
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
        if(pairs.size()>cols.size()-identityNums){
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
                        if(colsKeys[i].contains("5")){
                            Utils::print("[!] 自增约束无法插入值"+cols[i].toUpper());
                            return;
                        }
                        if(colsKeys[i].contains("7")){
                            if(!Utils::checkInsertPK(tableName,i,keyValue[1])){
                                Utils::print("[!] 外键约束无法插入值"+cols[i].toUpper());
                                return;
                            }
                        }
                        if(keyValue[1].toUpper()=="NULL"){
                            keyValue[1]="NULL";
                        }
                        if(keyValue[1]!="NULL"&&!Utils::checkColValue(cols[i],keyValue[1])){
                            Utils::print("[!] 输入值不符合"+cols[i].toUpper());
                            return;
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
                        rowData<<keyValue[1].trimmed();
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
                        rowData << colDefault[defaultID++].trimmed();
                    }
                    else if(colsKeys[i].contains("5")){
                        rowData << QString::number(identityValue+1).trimmed();
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
            QString indexPath = dbRoot + "/" + currentUser + "/" + usingDatabase + "/INDEXES/";
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
        if(inValues.size()>cols.size()-identityNums){
            Utils::print("[!] Too many values inserted");
            return;
        }
        if(inValues.size()<cols.size()-identityNums){
            Utils::print("[!] Too less values inserted");
            return;
        }
        if (file.open(QIODevice::Append | QIODevice::Text)){
            QTextStream out(&file);
            QStringList rowData;
            int insertNums=0;
            for(int i=0;i<cols.size();i++){
                if(colsKeys[i].contains("5")){
                    rowData << QString::number(identityValue+1);
                    continue;
                }
                if(colsKeys[i].contains("7")){
                    if(!Utils::checkInsertPK(tableName,i,inValues[insertNums])){
                        Utils::print("[!] 外键约束无法插入值"+cols[i].toUpper());
                        return;
                    }
                }
                if(inValues[insertNums].toUpper()=="NULL"){
                    inValues[insertNums]="NULL";
                }
                if(inValues[insertNums]!="NULL"&&!Utils::checkColValue(cols[i],inValues[insertNums])){
                    Utils::print("[!] 输入值不符合"+cols[i].toUpper());
                    return;
                }
                if(!colsKeys[i].contains("0")){
                    if(colsKeys[i].contains("1")){
                        if(columnSets[i].find(inValues[insertNums])!=columnSets[i].end()){
                            Utils::print("[!] 主键"+colsName[i]+"不能拥有重复值");
                            return;
                        }
                        if(inValues[insertNums].toUpper()=="NULL"){
                            Utils::print("[!] 主键"+colsName[i]+"不能为空值");
                            return;
                        }
                    }
                    if(colsKeys[i].contains("2")){
                        if(inValues[insertNums].toUpper()=="NULL"){
                            Utils::print("[!] "+colsName[i]+"不能为空值");
                            return;
                        }
                    }
                    if(colsKeys[i].contains("3")){
                        if(columnSets[i].find(inValues[insertNums])!=columnSets[i].end()){
                            Utils::print("[!] "+colsName[i]+":不能拥有重复值");
                            return;
                        }
                    }
                }
                rowData << inValues[insertNums++].trimmed();
            }
            out << rowData.join(",")<<"\n";
            file.close();
            QTextStream cout(stdout);
            Utils::print("已插入表 '" + tableName + "'.\n");
            Utils::writeLog("Inserted into " + tableName);
        }
    }
    else{
        Utils::print("[!]错误的字段格式. 期望的格式为: 'name1 values1,name2 values2,...' or 'values1,values2,...'\n");
        return;
    }
}

bool updateRecord(const QString& tableName, const QMap<QString, QString>& updates, const QString& whereClause) {
    QString path = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + tableName + ".txt";
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Utils::print("[!] 无法打开表文件");
        return false;
    }

    QTextStream in(&file);
    QString headerLine = in.readLine();
    QStringList headers = headerLine.split(",");
    QMap<int, QString> updateIndices;

    // 找出所有要更新列的索引
    for (auto it = updates.begin(); it != updates.end(); ++it) {
        int index = headers.indexOf(it.key());
        if (index == -1) {
            Utils::print("[!] 未找到列：" + it.key());
            return false;
        }
        updateIndices[index] = it.value();
    }

    int whereColIndex = -1;
    QString op, whereColumn, whereValue;
    bool hasWhere = false;

    if (!whereClause.isEmpty()) {
        QRegularExpression re(R"((\w+)\s*(<=|>=|=|<|>)\s*(.+))");
        QRegularExpressionMatch match = re.match(whereClause.trimmed());

        if (match.hasMatch()) {
            whereColumn = match.captured(1).trimmed();
            op = match.captured(2).trimmed();
            whereValue = match.captured(3).trimmed();
            whereColIndex = headers.indexOf(whereColumn);

            if (whereColIndex == -1) {
                Utils::print("[!] WHERE 子句中的列在表中未找到");
                return false;
            }

            hasWhere = true;
        } else {
            Utils::print("[!] 无效的 WHERE 子句");
            return false;
        }
    }

    QList<QString> lines;
    lines.append(headerLine);

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList values = line.split(",");
        bool match = true;

        if (hasWhere) {
            QString val = values.value(whereColIndex).trimmed();

            if (op == "=") match = (val == whereValue);
            else if (op == "<") match = (val < whereValue);
            else if (op == ">") match = (val > whereValue);
            else if (op == "<=") match = (val <= whereValue);
            else if (op == ">=") match = (val >= whereValue);
        }

        if (match) {
            for (auto it = updateIndices.begin(); it != updateIndices.end(); ++it) {
                values[it.key()] = it.value();  // 多列更新
            }
        }

        lines.append(values.join(","));
    }

    file.close();

    // 写回文件
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&file);
        for (const QString& l : lines) {
            out << l << "\n";
        }
        file.close();
        return true;
    } else {
        Utils::print("[!] 写回文件失败");
        return false;
    }
}


void selectFrom(const QString &tableName)
{
    if (usingDatabase.isEmpty()) {
        Utils::print("[!] 未选择数据库.\n");
        return;
    }
    QString path = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + tableName
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
int findFieldIndex(const QStringList &header, const QString &fieldName, bool &ambiguous) {
    int index = -1;
    int count = 0;

    for (int i = 0; i < header.size(); ++i) {
        QString h = header[i].trimmed();
        if (h == fieldName || h.endsWith("." + fieldName)) {
            index = i;
            ++count;
        }
    }

    ambiguous = (count > 1);
    return index;
}
void selectAdvancedInternal(const QString &query, QStringList &result)
{
    QRegularExpression re(
        R"(SELECT\s+(.*?)\s+FROM\s+(\w+)\s*(?:JOIN\s+(\w+)\s+ON\s+(\w+)\.(\w+)\s*=\s*(\w+)\.(\w+))?(?:\s+WHERE\s+(.*?))?(?:\s+GROUP\s+BY\s+(.*?))?(?:\s+ORDER\s+BY\s+(.*?))?$)",
        QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = re.match(query);
    if (!match.hasMatch()) {
        Utils::print("[!] 无效的 SELECT 语句.");
        return;
    }

    QString fieldsStr = match.captured(1).trimmed();
    QString table1 = match.captured(2).trimmed();
    QString joinTable = match.captured(3).trimmed();
    QString leftTable = match.captured(4).trimmed();
    QString leftField = match.captured(5).trimmed();
    QString rightTable = match.captured(6).trimmed();
    QString rightField = match.captured(7).trimmed();
    QString whereClause = match.captured(8).trimmed();
    QString groupByClause = match.captured(9).trimmed();
    QString orderByClause = match.captured(10).trimmed();

    QStringList rawLines;

    // ===== 读取表数据或执行 JOIN =====
    if (!joinTable.isEmpty()) {
        QFile file1(dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + table1 + ".txt");
        QFile file2(dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + joinTable + ".txt");
        if (!file1.open(QIODevice::ReadOnly | QIODevice::Text) || !file2.open(QIODevice::ReadOnly | QIODevice::Text)) {
            Utils::print("[!] 打开用于 JOIN 的表失败.");
            return;
        }

        QTextStream in1(&file1), in2(&file2);
        QStringList header1 = in1.readLine().split(",");
        QStringList header2 = in2.readLine().split(",");
        QList<QStringList> rows1, rows2;
        while (!in1.atEnd()) rows1.append(in1.readLine().split(","));
        while (!in2.atEnd()) rows2.append(in2.readLine().split(","));

        int idx1 = header1.indexOf(leftField);
        int idx2 = header2.indexOf(rightField);

        QStringList joinedHeader;
        for (const QString &h : header1) joinedHeader.append(table1 + "." + h);
        for (const QString &h : header2) joinedHeader.append(joinTable + "." + h);
        rawLines.append(joinedHeader.join(","));

        for (const QStringList &r1 : rows1) {
            for (const QStringList &r2 : rows2) {
                if (idx1 >= 0 && idx2 >= 0 && r1.value(idx1) == r2.value(idx2)) {
                    rawLines.append((r1 + r2).join(","));
                }
            }
        }
    } else {
        QFile file(dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + table1 + ".txt");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            Utils::print("[!] 无法打开表文件.");
            return;
        }
        QTextStream in(&file);
        rawLines.append(in.readLine());
        while (!in.atEnd()) rawLines.append(in.readLine());
    }

    // ===== WHERE 子句处理 =====
    if (!whereClause.isEmpty()) {
        QStringList filtered;
        QString headerLine = rawLines.takeFirst();
        QStringList header = headerLine.split(",");
        filtered.append(headerLine);

        QRegularExpression re(R"((\w+)\s*(<=|>=|=|<|>)\s*(.+))");
        QRegularExpressionMatch m = re.match(whereClause);
        if (m.hasMatch()) {
            QString col = m.captured(1).trimmed(), op = m.captured(2), val = m.captured(3).trimmed();
            bool ambiguous = false;
            int index = findFieldIndex(header, col, ambiguous);
            if (ambiguous) {
                Utils::print("[!] WHERE 字段不唯一，请加前缀: " + col); return;
            }
            if (index == -1) {
                Utils::print("[!] WHERE 字段未找到: " + col); return;
            }
            IndexManager indexManager(currentUser, usingDatabase);
            if (op == "=" && indexManager.indexExists(table1, col)) {
                QVector<qint64> offsets = indexManager.queryWithIndex(table1, col, val);

                QFile tableFile(dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + table1 + ".txt");
                if (tableFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in(&tableFile);

                    for (qint64 offset : offsets) {
                        tableFile.seek(offset);
                        QString line = in.readLine().trimmed();

                        qDebug() << "Read from offset" << offset << ":" << line;

                        if (!line.isEmpty()) {
                            filtered.append(line);
                        }
                    }

                    tableFile.close();
                    rawLines = filtered;

                }

            }
            else
            {
                for (const QString &line : rawLines) {
                    QStringList fields = line.split(",");
                    QString fieldVal = fields.value(index).trimmed();
                    bool ok1, ok2;
                    double fNum = fieldVal.toDouble(&ok1), vNum = val.toDouble(&ok2);
                    bool match = false;
                    if (ok1 && ok2) {
                        if (op == "=") match = (fNum == vNum);
                        else if (op == "<") match = (fNum < vNum);
                        else if (op == ">") match = (fNum > vNum);
                        else if (op == "<=") match = (fNum <= vNum);
                        else if (op == ">=") match = (fNum >= vNum);
                    } else {
                        if (op == "=") match = (fieldVal == val);
                        else if (op == "<") match = (fieldVal < val);
                        else if (op == ">") match = (fieldVal > val);
                        else if (op == "<=") match = (fieldVal <= val);
                        else if (op == ">=") match = (fieldVal >= val);
                    }
                    if (match) filtered.append(line);
                }
            }
            rawLines = filtered;
        }
    }

    // ===== 聚合函数 =====
    if (fieldsStr.contains("COUNT") || fieldsStr.contains("SUM") || fieldsStr.contains("AVG") ||
        fieldsStr.contains("MIN") || fieldsStr.contains("MAX")) {
        QRegularExpression aggRe(R"((COUNT|SUM|AVG|MIN|MAX)\s*\((\w+|\*)\))");
        QRegularExpressionMatch aggMatch = aggRe.match(fieldsStr);
        if (aggMatch.hasMatch()) {
            QString func = aggMatch.captured(1), field = aggMatch.captured(2);
            QStringList header = rawLines.first().split(",");
            int fieldIndex = -1;
            bool ambiguous = false;
            if (field != "*") fieldIndex = findFieldIndex(header, field, ambiguous);
            if (ambiguous) { Utils::print("[!] 聚合字段不唯一: " + field); return; }
            if (fieldIndex == -1 && field != "*") { Utils::print("[!] 聚合字段未找到: " + field); return; }

            double resultValue = 0;
            int count = 0;
            for (int i = 1; i < rawLines.size(); ++i) {
                QStringList fields = rawLines[i].split(",");
                if (field == "*") {
                    count++;
                    continue;
                }
                QString val = fields.value(fieldIndex).trimmed();
                bool ok;
                double num = val.toDouble(&ok);
                if (!ok) continue;
                if (func == "SUM" || func == "AVG") resultValue += num;
                if (func == "MIN" && (count == 0 || num < resultValue)) resultValue = num;
                if (func == "MAX" && (count == 0 || num > resultValue)) resultValue = num;
                count++;
            }
            if (func == "AVG" && count > 0) resultValue /= count;
            if (func == "COUNT") resultValue = count;
            result.append(QString::number(resultValue));
            return;
        }
    }

    // ===== ORDER BY 子句处理 =====
    if (!orderByClause.isEmpty()) {
        QString headerLine = rawLines.takeFirst();
        QStringList header = headerLine.split(",");
        bool ambiguous = false;
        int sortIdx = findFieldIndex(header, orderByClause.trimmed(), ambiguous);
        if (ambiguous) Utils::print("[!] ORDER BY 字段不唯一: " + orderByClause);
        if (sortIdx == -1) Utils::print("[!] ORDER BY 字段未找到: " + orderByClause);
        else {
            std::sort(rawLines.begin(), rawLines.end(), [sortIdx](const QString &a, const QString &b) {
                return a.split(",").value(sortIdx) < b.split(",").value(sortIdx);
            });
        }
        rawLines.prepend(headerLine);
    }

    // ===== 字段选择 =====
    QStringList header = rawLines.first().split(",");
    QList<int> selectedIndexes;
    if (fieldsStr == "*") {
        for (int i = 0; i < header.size(); ++i) selectedIndexes.append(i);
    } else {
        QStringList wanted = fieldsStr.split(",");
        for (QString col : wanted) {
            col = col.trimmed();
            bool ambiguous = false;
            int index = findFieldIndex(header, col, ambiguous);
            if (ambiguous) { Utils::print("[!] 字段不唯一，请使用前缀: " + col); continue; }
            if (index == -1) { Utils::print("[!] 字段未找到: " + col); continue; }
            selectedIndexes.append(index);
        }
    }

    for (const QString &line : rawLines) {
        QStringList fields = line.split(",");
        QStringList out;
        for (int idx : selectedIndexes) {
            QString val = fields.value(idx).trimmed();
            out.append(val.isEmpty() ? " " : val);
        }
        result.append(out.join(","));
    }
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
    QStringList FK;
    QString path2 = dbRoot + "/" + currentUser + "/" + usingDatabase + "/DATATYPE/" + tableName
                   + "_data.txt";
    QString path = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + tableName
                   + ".txt";
    QFile file2(path2);
    if (file2.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream in(&file2);
        in.readLine();
        QString secondline=in.readLine();
        QStringList colsKeys = secondline.split(",");
        for(int i=0;i<colsKeys.size();i++){
            if(colsKeys[i].contains("8")){
                FK << QString::number(i);
            }
        }
        file2.close();
    }
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
        }else{
            for(int i=0;i<FK.size();i++){
                if(Utils::checkDeletePK(tableName,FK[i].toInt(),values[FK[i].toInt()])){
                    Utils::print("[!] 已有外键引用，删除失败\n");
                    return;
                }
            }
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
    Utils::checkIdentity(tableName);
}
void fkConstraint(QString fkName,QString tablename,QString proTablename,QString colname,QString ProColname){
    int proIndex;
    int index;
    QSet<QString> proValues;
    QString ProPath = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" +  proTablename
                      + ".txt";
    QFile proFile(ProPath);
    if(proFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream in(&proFile);
        QString firstline=in.readLine();
        QStringList colsName = firstline.split(",",Qt::SkipEmptyParts);
        proIndex =colsName.indexOf(ProColname);
        if(proIndex==-1){
            Utils::print("[!] 未找到列名\n");
            return;
        }
        qDebug()<<"idpdex="+QString::number(proIndex);
        while(!in.atEnd()){
            QString restline=in.readLine();
            if(restline.isEmpty()){
                break;
            }
            QStringList restCol =restline.split(",",Qt::SkipEmptyParts);
            proValues.insert(restCol[proIndex]);
        }
        proFile.close();
    }
    QString path = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" +  tablename
                      +  ".txt";
    QFile file(path);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream in(&file);
        QString firstline=in.readLine();
        QStringList colsName = firstline.split(",",Qt::SkipEmptyParts);
        index =colsName.indexOf(colname);
         qDebug()<<"iddex="+QString::number(index);
        while (!in.atEnd()) {
            QString restline=in.readLine();
            if(restline.isEmpty()){
                break;
            }
            QStringList restCol =restline.split(",",Qt::SkipEmptyParts);
            if(!proValues.contains(restCol[index])){
                Utils::print("[!] 无法为这2组数据添加外键 \n");
                file.close();
                return;
            }
        }
        file.close();
    }
    QString ProPath2 = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" +"DATATYPE/"+  proTablename
                      + "_data.txt";
    QFile proFile2(ProPath2);{
        if(proFile2.open(QIODevice::ReadWrite | QIODevice::Text)){
            QStringList content;
            QTextStream in(&proFile2);
            content<<in.readLine();
            QString line2=in.readLine();
            QStringList cols2=line2.split(",",Qt::SkipEmptyParts);
            if(!cols2[proIndex].contains("1")&&!cols2[proIndex].contains("3")){
                Utils::print("[!] 只能将主键或不重复约束的键设为外键 \n");
                proFile2.close();
                return;
            }
            cols2[proIndex]+="8";
            content << cols2.join(",");
            content << in.readLine();
            proFile2.resize(0);
            proFile2.seek(0);
            QTextStream out(&proFile2);
            out << content.join("\n")+"\n";
            proFile2.close();
        }
    }

    QString path2 = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" +"DATATYPE/"+  tablename
                       + "_data.txt";
    QFile file2(path2);{
        if(file2.open(QIODevice::ReadWrite | QIODevice::Text)){
            QStringList content;
            QTextStream in(&file2);
            content<<in.readLine();
            QString line2=in.readLine();
            QStringList cols2=line2.split(",",Qt::SkipEmptyParts);
            cols2[index]+="7";
            content << cols2.join(",");
            content << in.readLine();
            file2.resize(0);
            file2.seek(0);
            QTextStream out(&file2);
            out << content.join("\n")+"\n";
            file2.close();
        }
    }
    QString fkpath = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + "DATATYPE/"
                                                                              "fk_data.txt";
    QFile fkFile(fkpath);
    if(fkFile.open(QIODevice::Append|QIODevice::Text)){
        QTextStream out(&fkFile);
        out<<fkName+","+tablename+","+QString::number(index)+","+proTablename+","+QString::number(proIndex)+"\n";
    }
    return;
}
void headerManage(const QString command){

    if (usingDatabase.isEmpty()) {
        QTextStream cout(stdout);
        Utils::print("[!]未选择数据库.\n");
        return;
    }

    QRegularExpression regex(
        "ALTER\\s+TABLE\\s+(\\w+)\\s+"
        "(ADD|MODIFY|DROP)\\s+"
        "(\\w+)"
        "(?:\\s+([^\\s]+(?:\\s+[^\\s]+)*))?"
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

        QString path = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + tableName
                       + ".txt";
        QString path2 = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + "DATATYPE/"+tableName
                        +"_data.txt";
        QFile file(path);
        QFile file2(path2);

        if (!file.exists()||!file2.exists()) {
            qDebug() << "Error: Table file does not exist";
            Utils::print("[!] 表文件不存在\n");
            return;
        }

        if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            qDebug() << "Failed to open file:" << path << "Error:" << file.errorString();
            return;
        }
        if (!file2.open(QIODevice::ReadWrite | QIODevice::Text)) {
            qDebug() << "Failed to open file:" << path2 << "Error:" << file2.errorString();
        }                                                              //添加
        //新增对 MODIFY PRIMARY KEY 的处理
        if (operation.toUpper() == "MODIFY" &&
            columnName.toUpper() == "PRIMARY" &&
            extraInfo.toUpper().startsWith("KEY")) {

            QStringList parts = extraInfo.split(" ", Qt::SkipEmptyParts);
            if (parts.size() < 2) {
                Utils::print("[!] MODIFY PRIMARY KEY 语法错误，缺少字段名\n");
                return;
            }

            QString newPrimaryKey = parts[1];

            // 读取表文件第一行：字段名列表
            QTextStream in(&file);
            QString headerLine = in.readLine();
            QStringList headers = headerLine.split(",", Qt::SkipEmptyParts);
            int pkIndex = headers.indexOf(newPrimaryKey);
            if (pkIndex == -1) {
                Utils::print("[!] 指定的主键字段不存在: " + newPrimaryKey + "\n");
                return;
            }

            // 读取数据类型文件的三行
            QTextStream in2(&file2);
            QString typeLine = in2.readLine();
            QString statusLine = in2.readLine();
            QString defaultLine = in2.readLine();

            QStringList statusList = statusLine.split(",", Qt::SkipEmptyParts);
            if (statusList.size() != headers.size()) {
                Utils::print("[!] 字段状态长度与字段数不匹配\n");
                return;
            }

            // 设置指定列为主键（1），其余为 0
            for (int i = 0; i < statusList.size(); ++i)
                statusList[i] = (i == pkIndex ? "1" : "0");

            // 回写文件
            file2.resize(0);
            file2.seek(0);
            QTextStream out2(&file2);
            out2 << typeLine << "\n";
            out2 << statusList.join(",") << "\n";
            out2 << defaultLine << "\n";

            file.close();
            file2.close();

            Utils::print("[+] 主键已修改为: " + newPrimaryKey + "\n");
            Utils::writeLog("Alter table: " + tableName + " MODIFY PRIMARY KEY " + newPrimaryKey);
            return;
        }

        if (operation.toUpper() == "ADD") {
            if(extraInfo.isEmpty()){
                qDebug() << "Error: need more information";
                Utils::print("[!] Error type need more information\n");
                return;
            }

            QStringList keyInfo=extraInfo.split(" ",Qt::SkipEmptyParts);
            //从这里进入处理外键
            if(columnName.toUpper()=="CONSTRAINT"&&
                keyInfo[0].toUpper() != "INT" &&
                keyInfo[0].toUpper() != "VARCHAR" &&
                keyInfo[0].toUpper() != "TEXT" &&
                keyInfo[0].toUpper() != "DATE" &&
                keyInfo[0].toUpper() != "FLOAT" &&
                keyInfo[0].toUpper() != "DOUBLE" &&
                keyInfo[0].toUpper() != "BOOLEAN"){
                if(keyInfo.size()!=4){
                    Utils::print("[!] 格式错误！期待格式：ALTER TABLE <tablename> ADD CONSTRAINT 外键名称 子表的列的名称 父表名 父表的列的名称	\n");
                        return;
                }
                QString fkName =keyInfo[0];
                QString fkpath = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + "DATATYPE/"
                                "fk_data.txt";
                QFile fkFile(fkpath);
                if(fkFile.open(QIODevice::ReadOnly|QIODevice::Text)){
                    QTextStream in(&fkFile);
                    while (!in.atEnd()) {
                        QString line=in.readLine();
                        QStringList list =line.split(",");
                        if(list[0]==fkName){
                            Utils::print("[!] 重复外键名称 \n");
                            return;
                        }
                    }
                    fkFile.close();
                }
                fkConstraint(fkName,tableName,keyInfo[2],keyInfo[1],keyInfo[3]);
                return;
            }
            if (keyInfo[0].toUpper() != "INT" &&
                keyInfo[0].toUpper() != "VARCHAR" &&
                keyInfo[0].toUpper() != "TEXT" &&
                keyInfo[0].toUpper() != "DATE" &&
                keyInfo[0].toUpper() != "FLOAT" &&
                keyInfo[0].toUpper() != "DOUBLE" &&
                keyInfo[0].toUpper() != "BOOLEAN"){

                Utils::print("[!] Error type\n");
                return;
            }  
            //添加到列定义
            QTextStream in2(&file2);
            QTextStream out2(&file2);
            QString firstLine2 = in2.readLine();
            QString secondLine2;
            QString thirdLine2;
            QString newFile;
            QStringList consInfo=keyInfo.mid(1);
            QStringList getInfo;
            int primaryKey=0;
            if (firstLine2.isEmpty()) {
                getInfo=Utils::readKeysInfor(consInfo,primaryKey);
                if(getInfo[0]=="error"){
                    file2.close();
                    return;
                }
                newFile += keyInfo[0]+"\n";
                newFile += getInfo[0]+"\n";
                newFile += getInfo[1]+"\n";
            } else {
                newFile += firstLine2 + "," + keyInfo[0] + "\n";
                secondLine2 = in2.readLine();
                if(!in2.atEnd()){
                    thirdLine2 = in2.readLine();
                }
                if(secondLine2.contains("1")){
                    primaryKey=1;
                }
                getInfo=Utils::readKeysInfor(consInfo,primaryKey);
                if(getInfo[0]=="error"){
                    file2.close();
                    return;
                }
                newFile+=secondLine2+","+getInfo[0]+"\n";
                if(getInfo[1].isEmpty()){
                    newFile+=thirdLine2+"\n";
                }
                else{
                    if(thirdLine2.isEmpty()){
                        newFile+=getInfo[1]+"\n";
                    }else{
                        newFile+=thirdLine2+","+getInfo[1] +"\n";
                    }
                }
            }
            file2.seek(0);
            file2.resize(0);

            //添加到列
            QTextStream in(&file);
            QString firstLine = in.readLine();
            QString contentLine = in.readAll();
            file.seek(0);
            file.resize(0);
            QString defaultValue;
            QString newfile;
            QTextStream out(&file);
            int identityValue=1;
            if(getInfo[1].isEmpty()){
                if(getInfo[0].contains("2")){
                    defaultValue=Utils::getDefaultValue(keyInfo[0]);
                }else if(getInfo[0].contains("5")){
                    defaultValue="IDENTITY";
                }
                else{
                    defaultValue="NULL";
                }
            }else{
                defaultValue=getInfo[1];
            }
            if (firstLine.isEmpty()) {
                newfile += columnName + "\n";
            } else {
                newfile += firstLine + "," + columnName + "\n";
            }
            if(!contentLine.isEmpty()){
                QStringList rows = contentLine.split("\n", Qt::SkipEmptyParts);
                if(rows.size()>1&&getInfo[0].contains("2")&&getInfo[0].contains("3")){
                    Utils::print("[!] 无法满足"+columnName+"既是非空又是不重复还存在2条以上的数据"+"\n");
                    return;
                }
                if(rows.size()>1&&getInfo[0].contains("3")&&getInfo[0].contains("4")){
                    Utils::print("[!] 无法满足"+columnName+"既是不重复又是默认赋值还存在2条以上的数据"+"\n");
                    return;
                }
                if(rows.size()>1&&getInfo[0].contains("1")){
                    Utils::print("[!] 无法满足"+columnName+"是主键还存在2条以上的数据"+"\n");
                    return;
                }
                for (QString &row : rows) {
                    if(defaultValue=="IDENTITY"){
                        newfile += row + "," + QString::number(identityValue++) + "\n";
                    }
                    else{
                        newfile += row + "," + defaultValue + "\n";
                    }
                }
            }
            out2 << newFile;
            file2.close();
            out << newfile;
            file.close();
        }                                                            //修改
        else if (operation.toUpper() == "MODIFY") {
            if(extraInfo.isEmpty()){
                qDebug() << "Error: need more information";
                Utils::print("[!] Error type need more information\n");
                return;
            }
            QStringList keyInfo=extraInfo.split(" ",Qt::SkipEmptyParts);
            if (keyInfo[0].toUpper() != "INT" &&
                keyInfo[0].toUpper() != "VARCHAR" &&
                keyInfo[0].toUpper() != "TEXT" &&
                keyInfo[0].toUpper() != "DATE" &&
                keyInfo[0].toUpper() != "FLOAT" &&
                keyInfo[0].toUpper() != "DOUBLE" &&
                keyInfo[0].toUpper() != "BOOLEAN"){

                Utils::print("[!] Error type\n");
                return;
            }

            QStringList consInfo=keyInfo.mid(1);
            QStringList getInfo;
            int priNum=0;

            QTextStream in(&file);
            QString columnLine = in.readLine();
            QStringList columns = columnLine.split(",", Qt::SkipEmptyParts);

            int columnIndex = columns.indexOf(columnName);
            if (columnIndex == -1) {
                qDebug() << "Error: Column" << columnName << "not found in table";
                Utils::print("[!] Column '" + columnName + "' not found in table\n");
                return;
            }

            QTextStream in2(&file2);
            QString defLine = in2.readLine();
            QStringList definitions = defLine.split(",");
            if(!Utils::checkColValue(keyInfo[0],columnIndex,path)){
                Utils::print("[!] Column " + columnName + " 无法修改，表中数据无法转换为修改数据\n");
                return;
            }
            QString defLine2 = in2.readLine();
            QStringList definitions2 = defLine2.split(",");
            for(int i=0;i<definitions2.size();i++){
                if(i==columnIndex){
                    continue;
                }
                if(definitions2[i].contains("1")){
                    priNum=1;
                    break;
                }
            }
            QString defLine3 = in2.readLine();
            QStringList definitions3 = defLine3.split(",");

            getInfo=Utils::readKeysInfor(consInfo,priNum);
            if(getInfo[0]=="error"){
                file.close();
                file2.close();
                return;
            }
            // 修改对应位置的列定义
            definitions[columnIndex] = keyInfo[0];
            definitions2[columnIndex] = getInfo[0];
            definitions3[columnIndex] = getInfo[1];

            file2.resize(0);
            file2.seek(0);
            QTextStream out2(&file2);
            out2 << definitions.join(",") << "\n";
            out2 << definitions2.join(",") << "\n";
            out2 << definitions3.join(",") << "\n";
            file.close();
            file2.close();
        }                                                          //删除
        else if (operation.toUpper() == "DROP") {
            // 读取file的所有内容
            QTextStream in(&file);
            QStringList fileLines;
            while (!in.atEnd()) {
                fileLines << in.readLine();
            }

            // 检查是否有数据
            if (fileLines.isEmpty()) {
                qDebug() << "Error: Empty table file";
                Utils::print("[!] 表文件为空\n");
                return;
            }

            QStringList columns = fileLines.first().split(",", Qt::SkipEmptyParts);
            int columnIndex = columns.indexOf(columnName);
            if (columnIndex == -1) {
                qDebug() << "Error: Column" << columnName << "not found in table";
                Utils::print("[!] 字段 '" + columnName + "' 在表中未找到\n");
                return;
            }

            // 处理file的所有行
            for (int i = 0; i < fileLines.size(); ++i) {
                QStringList fields = fileLines[i].split(",", Qt::SkipEmptyParts);
                fields.removeAt(columnIndex);
                fileLines[i] = fields.join(",");
            }

            // 读取file2的所有内容
            QTextStream in2(&file2);
            QStringList file2Lines;
            while (!in2.atEnd()) {
                file2Lines << in2.readLine();
            }

            // 处理file2的所有行
            for (int i = 0; i < file2Lines.size(); ++i) {
                QStringList fields = file2Lines[i].split(",", Qt::SkipEmptyParts);
                fields.removeAt(columnIndex);
                file2Lines[i] = fields.join(",");
            }

            file.resize(0);
            file.seek(0);
            QTextStream out(&file);
            out << fileLines.join("\n");

            file2.resize(0);
            file2.seek(0);
            QTextStream out2(&file2);
            out2 << file2Lines.join("\n");

            qDebug() << "Column" << columnName << "dropped successfully";
            Utils::print("[+] 字段 '" + columnName + "' 成功删除\n");

        }
        Utils::print("更改了表: " + tableName + operation + columnName +extraInfo);
        Utils::writeLog("Alter table: " + tableName + operation + columnName +extraInfo);
    }else {
        Utils::print("Invalid ALTER TABLE syntax");
    }
}
IndexManager::IndexManager(const QString& user, const QString& db)
    : currentUser(user), usingDatabase(db) {
    QString indexPath = dbRoot + "/" + currentUser + "/" + usingDatabase + "/INDEXES";
    QDir().mkpath(indexPath);
}

QString IndexManager::getIndexPath(const QString& tableName, const QString& columnName) {
    return dbRoot + "/" + currentUser + "/" + usingDatabase +
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
    QString tablePath = dbRoot + "/" + currentUser + "/" + usingDatabase + "/" + tableName + ".txt";
    QFile tableFile(tablePath);
    if (!tableFile.open(QIODevice::ReadOnly)) {
        Utils::print("[!] 无法打开数据文件");
        return false;
    }

    QByteArray headerLine = tableFile.readLine();
    QStringList headers = QString::fromUtf8(headerLine).split(",");
    int columnIndex = headers.indexOf(columnName);
    if (columnIndex == -1) {
        Utils::print("[!] 表中未找到字段 " + columnName);
        return false;
    }

    QString indexPath = getIndexPath(tableName, columnName);
    QFile indexFile(indexPath);
    if (!indexFile.open(QIODevice::Append | QIODevice::Text)) {
        Utils::print("[!] 无法写入索引文件");
        return false;
    }

    QTextStream out(&indexFile);
    QMap<QString, QVector<qint64>> indexMap;

    while (!tableFile.atEnd()) {
        qint64 offset = tableFile.pos();  // 放在读取之前
        QByteArray lineData = tableFile.readLine();
        QString line = QString::fromUtf8(lineData);
        if (line.trimmed().isEmpty()) continue;

        QStringList values = line.split(",");
        if (values.size() <= columnIndex) continue;

        QString key = values[columnIndex].trimmed();
        indexMap[key].append(offset);
    }

    for (auto it = indexMap.begin(); it != indexMap.end(); ++it) {
        for (qint64 offset : it.value()) {
            out << it.key() << ":" << offset << "\n";
        }
    }

    tableFile.close();
    indexFile.close();
    Utils::print("[+] 成功创建 B 树风格索引（QMap）");

    return true;
}


QVector<qint64> IndexManager::queryWithIndex(const QString& tableName, const QString& columnName, const QString& key) {
    QVector<qint64> results;
    QString indexPath = getIndexPath(tableName, columnName);
    QFile indexFile(indexPath);
    if (!indexFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Utils::print("[!] 无法打开索引文件: " + indexPath);
        return results;
    }

    QTextStream in(&indexFile);
    // 跳过前三行元信息
    for (int i = 0; i < 3; i++) in.readLine();

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split(":");
        if (parts.size() != 2) continue;

        if (parts[0].trimmed() == key) {
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

