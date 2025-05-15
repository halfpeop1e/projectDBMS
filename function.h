#ifndef FUNCTION_H
#define FUNCTION_H
#include <QString>
#include <QTextBrowser>
namespace Utils {
extern QString dbRoot;
extern QString userFile;
extern QString logFile;
void writeLog(const QString &entry);
QStringList splitCommand(const QString &command);
void showHelp();
bool ensureDirExists(const QString &path);
void setOutputShell(QTextBrowser *shell);
void print(const QString &msg);
QString formatAsTable(const QStringList &lines);
} // namespace Utils

#endif // FUNCTION_H
