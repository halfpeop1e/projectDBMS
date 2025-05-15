#ifndef TOOLFUNCTION_H
#define TOOLFUNCTION_H
#include <QString>
#include <QTextBrowser>

namespace Utils {

void writeLog(const QString &entry);
QStringList splitCommand(const QString &command);
QString getDefaultValue(const QString type);
bool checkColValue(QString type,int index,QString filePath);
bool checkColValue(QString type,QString value);
QStringList readKeysInfor(const QStringList info,int& primaryNums);
void showHelp();
bool ensureDirExists(const QString &path);
void setOutputShell(QTextBrowser *shell);
void checkIdentity(QString tableName);
void print(const QString &msg);
QString formatAsTable(const QStringList &lines);
} // namespace Utils
#endif // TOOLFUNCTION_H
