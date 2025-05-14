#ifndef TOOLFUNCTION_H
#define TOOLFUNCTION_H
#include <QString>
#include <QTextBrowser>

namespace Utils {

void writeLog(const QString &entry);
QStringList splitCommand(const QString &command);
QStringList readKeysInfor(const QStringList info);
void showHelp();
bool ensureDirExists(const QString &path);
void setOutputShell(QTextBrowser *shell);
void print(const QString &msg);
QString formatAsTable(const QStringList &lines);
} // namespace Utils
#endif // TOOLFUNCTION_H
