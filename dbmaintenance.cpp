#include "dbmaintenance.h"
#include "globals.h"
#include <QDirIterator>
#include <QMessageBox>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>

DBMaintenance::DBMaintenance(QObject *parent) : QObject(parent)
{
}

bool DBMaintenance::backupSystem(const QString &backupPath, QWidget *parent)
{
    QDir backupDir(backupPath);
    if (!backupDir.exists()) {
        if (!backupDir.mkpath(".")) {
            QMessageBox::critical(parent, tr("错误"), tr("无法创建备份目录: %1").arg(backupPath));
            return false;
        }
    }
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString fullBackupPath = backupDir.filePath("dbms_backup_" + timestamp);
    if (!QDir().mkpath(fullBackupPath)) {
        QMessageBox::critical(parent, tr("错误"), tr("无法创建备份子目录: %1").arg(fullBackupPath));
        return false;
    }
    QFile metaFile(fullBackupPath + "/backup_meta.json");
    if (!metaFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(parent, tr("错误"), tr("无法创建备份元数据文件"));
        return false;
    }
    QJsonObject metaData;
    metaData["version"] = "1.0";
    metaData["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    metaData["user"] = currentUser;
    metaData["dbRoot"] = dbRoot;
    QTextStream metaStream(&metaFile);
    metaStream << QJsonDocument(metaData).toJson();
    metaFile.close();
    if (!copyDir(dbRoot, fullBackupPath + "/data")) {
        QMessageBox::critical(parent, tr("错误"), tr("复制数据库文件时出错"));
        return false;
    }
    if (!QFile::copy(userFile, fullBackupPath + "/users.txt")) {
        QMessageBox::critical(parent, tr("错误"), tr("复制用户文件时出错"));
        return false;
    }
    if (!QFile::copy(logFile, fullBackupPath + "/log.txt")) {
        QMessageBox::critical(parent, tr("错误"), tr("复制日志文件时出错"));
        return false;
    }

    return true;
}
bool DBMaintenance::restoreSystem(const QString &backupPath, QWidget *parent)
{
    if (!validateBackup(backupPath)) {
        QMessageBox::critical(parent, tr("错误"), tr("备份文件验证失败"));
        return false;
    }
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(parent, tr("确认"),
                                  tr("这将覆盖所有现有数据。确定要继续吗?"),
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return false;
    }
    QString tempBackup = QDir::tempPath() + "/dbms_temp_backup_" +
                         QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    if (!backupSystem(tempBackup, parent)) {
        QMessageBox::critical(parent, tr("错误"), tr("无法创建临时备份"));
        return false;
    }
    try {
        if (!removeDir(dbRoot)) {
            throw tr("无法清除现有数据库目录");
        }
        if (!copyDir(backupPath + "/data", dbRoot)) {
            throw tr("还原数据库文件时出错");
        }
        if (QFile::exists(backupPath + "/users.txt")) {
            if (!QFile::remove(userFile)) {
                throw tr("无法移除现有用户文件");
            }
            if (!QFile::copy(backupPath + "/users.txt", userFile)) {
                throw tr("还原用户文件时出错");
            }
        }
        if (QFile::exists(backupPath + "/log.txt")) {
            if (!QFile::remove(logFile)) {
                throw tr("无法移除现有日志文件");
            }
            if (!QFile::copy(backupPath + "/log.txt", logFile)) {
                throw tr("还原日志文件时出错");
            }
        }
        removeDir(tempBackup);
        return true;
    } catch (const QString &error) {
        QMessageBox::critical(parent, tr("错误"), error);

        if (!restoreSystem(tempBackup, parent)) {
            QMessageBox::critical(parent, tr("错误"),
                                  tr("还原失败"));
        } else {
            QMessageBox::information(parent, tr("信息"),
                                     tr("已恢复原系统"));
        }
        return false;
    }
}
bool DBMaintenance::backupDatabase(const QString &dbName, const QString &backupPath, QWidget *parent)
{
    QString dbPath = dbRoot + "/" + currentUser + "/" + dbName;
    if (!QDir(dbPath).exists()) {
        QMessageBox::critical(parent, tr("错误"), tr("数据库不存在").arg(dbName));
        return false;
    }
    QDir backupDir(backupPath);
    if (!backupDir.exists()) {
        if (!backupDir.mkpath(".")) {
            QMessageBox::critical(parent, tr("错误"), tr("无法创建备份目录: %1").arg(backupPath));
            return false;
        }
    }
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString fullBackupPath = backupDir.filePath(dbName + "_backup_" + timestamp);
    if (!QDir().mkpath(fullBackupPath)) {
        QMessageBox::critical(parent, tr("错误"), tr("无法创建备份子目录").arg(fullBackupPath));
        return false;
    }
    QFile metaFile(fullBackupPath + "/db_meta.json");
    if (!metaFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(parent, tr("错误"), tr("无法创建备份元数据文件"));
        return false;
    }
    QJsonObject metaData;
    metaData["version"] = "1.0";
    metaData["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    metaData["user"] = currentUser;
    metaData["database"] = dbName;
    QTextStream metaStream(&metaFile);
    metaStream << QJsonDocument(metaData).toJson();
    metaFile.close();
    if (!copyDir(dbPath, fullBackupPath + "/" + dbName)) {
        QMessageBox::critical(parent, tr("错误"), tr("复制数据库文件时出错"));
        return false;
    }

    return true;
}
bool DBMaintenance::restoreDatabase(const QString &dbName, const QString &backupPath, QWidget *parent)
{
    QString dbBackupPath = backupPath + "/" + dbName;
    if (!QDir(dbBackupPath).exists()) {
        QMessageBox::critical(parent, tr("错误"), tr("备份中找不到数据库: %1").arg(dbName));
        return false;
    }
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(parent, tr("确认"),
                                  tr("这将覆盖数据库 %1 的所有现有数据。确定要继续吗?").arg(dbName),
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return false;
    }

    QString dbPath = dbRoot + "/" + currentUser + "/" + dbName;
    QString tempBackup = QDir::tempPath() + "/" + dbName + "_temp_backup_" +
                         QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    if (QDir(dbPath).exists() && !backupDatabase(dbName, tempBackup, parent)) {
        QMessageBox::critical(parent, tr("错误"), tr("无法创建临时备份"));
        return false;
    }
    try {
        if (QDir(dbPath).exists() && !removeDir(dbPath)) {
            throw tr("无法清除现有数据库目录");
        }
        if (!copyDir(dbBackupPath, dbPath)) {
            throw tr("还原数据库文件时出错");
        }
        if (QDir(tempBackup).exists()) {
            removeDir(tempBackup);
        }
        return true;
    } catch (const QString &error) {
        QMessageBox::critical(parent, tr("错误"), error);

        if (QDir(tempBackup).exists() && !restoreDatabase(dbName, tempBackup, parent)) {
            QMessageBox::critical(parent, tr("严重错误"),
                                  tr("还原失败且无法恢复原数据库！"));
        } else if (QDir(tempBackup).exists()) {
            QMessageBox::information(parent, tr("信息"),
                                     tr("已恢复原数据库"));
        }
        return false;
    }
}
QString DBMaintenance::getBackupInfo(const QString &backupPath)
{
    QFile metaFile(backupPath + "/backup_meta.json");
    if (!metaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return tr("无法读取备份信息");
    }
    QJsonDocument doc = QJsonDocument::fromJson(metaFile.readAll());
    metaFile.close();
    if (doc.isNull()) {
        return tr("无效的备份元数据");
    }
    QJsonObject meta = doc.object();
    QString info = tr("备份信息:\n");
    info += tr("时间: %1\n").arg(meta["timestamp"].toString());
    info += tr("用户: %1\n").arg(meta["user"].toString());
    info += tr("版本: %1\n").arg(meta["version"].toString());

    return info;
}
bool DBMaintenance::copyDir(const QString &src, const QString &dest, bool overwrite)
{
    QDir srcDir(src);
    if (!srcDir.exists()) {
        return false;
    }
    QDir destDir(dest);
    if (!destDir.exists() && !destDir.mkpath(".")) {
        return false;
    }
    QDirIterator it(src, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        const QString srcPath = it.filePath();
        QString relativePath = srcDir.relativeFilePath(srcPath);
        QString destPath = QDir(dest).filePath(relativePath);
        if (it.fileInfo().isDir()) {
            if (!QDir().mkpath(destPath)) {
                return false;
            }
        } else {
            if (QFile::exists(destPath)) {
                if (overwrite) {
                    QFile::remove(destPath);
                } else {
                    continue;
                }
            }
            if (!QFile::copy(srcPath, destPath)) {
                return false;
            }
        }
        emit progressChanged(it.filePath().length() * 100 / src.length());
    }
    return true;
}
bool DBMaintenance::removeDir(const QString &dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists()) {
        QDirIterator it(dirName, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            QFileInfo info = it.fileInfo();
            if (info.isDir() && info.fileName() != "." && info.fileName() != "..") {
                result = QDir().rmdir(info.absoluteFilePath());
            } else if (info.isFile()) {
                result = QFile::remove(info.absoluteFilePath());
            }
            if (!result) {
                return false;
            }
        }
        result = dir.rmdir(dirName);
    }

    return result;
}
bool DBMaintenance::validateBackup(const QString &backupPath)
{
    QFile metaFile(backupPath + "/backup_meta.json");
    if (!metaFile.exists() || !metaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QJsonDocument doc = QJsonDocument::fromJson(metaFile.readAll());
    metaFile.close();
    if (doc.isNull() || !doc.isObject()) {
        return false;
    }
    if (!QDir(backupPath + "/data").exists()) {
        return false;
    }
    return true;
}
