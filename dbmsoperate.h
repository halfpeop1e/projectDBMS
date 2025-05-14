#ifndef DBMSOPERATE_H
#define DBMSOPERATE_H
#include<QString>
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
bool updateRecord(const QString& tableName, const QMap<QString, QString>& updates, const QString& whereClause);
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

#endif // DBMSOPERATE_H
