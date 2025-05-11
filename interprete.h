#ifndef INTERPRETE_H
#define INTERPRETE_H
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
QString getDefaultValue(const QString &type);
} // namespace Utils

namespace User {
bool signup(const QString &username, const QString &password);
bool login(QString &user);
} // namespace User

namespace DBMS {
void createDatabase(const QString &dbName);
void dropDatabase(const QString &dbName);
void useDatabase(const QString &dbName);
void createTable(const QString &tableName, const QString &columns);
void dropTable(const QString &tableName);
void insertInto(const QString &tableName, const QString &values);
void selectFrom(const QString &tableName);
void selectAdvanced(const QString &command);
void selectAdvancedInternal(const QString &query, QStringList &result);
void deleteFrom(const QString &tableName, const QString &condition);
void headerManage(const QString command);
} // namespace DBMS

namespace Interpreter {
void interpret(const QString &command);
}

// 全局变量
extern QString currentUser;
extern QString usingDatabase;

#endif // INTERPRETE_H
