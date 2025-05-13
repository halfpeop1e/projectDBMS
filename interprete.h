#ifndef INTERPRETE_H
#define INTERPRETE_H
#include <QString>
#include <QTextBrowser>

namespace Auth {
enum Role {
    GUEST = 0,    // 只读权限
    NORMAL = 1,   // 基本操作权限
    ADMIN = 2     // 所有权限
};

const QMap<QString, Role> ROLE_MAP = {
    {"guest", GUEST},
    {"normal", NORMAL},
    {"admin", ADMIN}
};
bool checkPermission(Role requiredRole);
QString getRoleName(Role role);
}

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

namespace User {
bool signup(const QString& username, const QString& password);
bool login(QString& user);
bool grantRole(const QString& adminUser, const QString& username, const QString& role);
}// namespace User

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
class IndexManager {
public:
    IndexManager(const QString& user, const QString& db);
    bool createIndex(const QString& tableName, const QString& columnName, const QString& indexType);
    bool dropIndex(const QString& tableName, const QString& columnName);
    QVector<qint64> queryWithIndex(const QString& tableName, const QString& columnName, const QString& key);
    bool indexExists(const QString& tableName, const QString& columnName);
private:
    QString getIndexPath(const QString& tableName, const QString& columnName);
    bool buildIndex(const QString& tableName, const QString& columnName, const QString& indexType);
    QString currentUser;
    QString usingDatabase;
};
} // namespace DBMS

namespace Interpreter {
void interpret(const QString &command);
}

// 全局变量
extern QString currentUser;
extern QString usingDatabase;

#endif // INTERPRETE_H
