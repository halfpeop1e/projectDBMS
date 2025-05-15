#ifndef DBMAINTENANCE_H
#define DBMAINTENANCE_H
#include <QObject>
#include <QString>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QProgressDialog>

class DBMaintenance : public QObject
{
    Q_OBJECT

public:
    explicit DBMaintenance(QObject *parent = nullptr);
    bool backupSystem(const QString &backupPath, QWidget *parent = nullptr);
    bool restoreSystem(const QString &backupPath, QWidget *parent = nullptr);
    bool backupDatabase(const QString &dbName, const QString &backupPath, QWidget *parent = nullptr);
    bool restoreDatabase(const QString &dbName, const QString &backupPath, QWidget *parent = nullptr);
    static QString getBackupInfo(const QString &backupPath);
signals:
    void progressChanged(int value);
    void statusMessage(const QString &message);

private:
    bool copyDir(const QString &src, const QString &dest, bool overwrite = false);
    bool removeDir(const QString &dirName);
    bool createZip(const QString &srcDir, const QString &zipFile);
    bool extractZip(const QString &zipFile, const QString &destDir);
    bool validateBackup(const QString &backupPath);
};
#endif // DBMAINTENANCE_H
