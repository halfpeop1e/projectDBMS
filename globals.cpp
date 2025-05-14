#include "globals.h"
#include <QStandardPaths>
#include <QDir>
QString baseDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Resource";;
QString currentUser="guest";
Auth::Role currentRole;
QTextBrowser *showinShell = nullptr;
QString usingDatabase;
<<<<<<< HEAD
QString dbRoot = baseDir;             // 所有数据库存放路径
QString userFile = baseDir+"/users.txt"; // 用户信息存储文件
QString logFile = baseDir+"/log.txt";    // 日志文件
=======
QString dbRoot = "E:/dbms/Resource";             // 所有数据库存放路径
QString userFile = "E:/dbms/Resource/users.txt"; // 用户信息存储文件
QString logFile = "E:/dbms/Resource/log.txt";    // 日志文件
>>>>>>> 4f5259024b3ec705ff10ce045c4198f930548b4e
MainWindow *mainWindow = nullptr;
void initializeUserFile() {
    // 获取资源目录路径

    QDir().mkpath(baseDir); // 确保目录存在

    QString userFilePath = userFile;
    QString logFilePath=logFile;
    QFileInfo fileInfo(userFilePath);
    QFileInfo logfileInfo(logFilePath);
    // 如果文件不存在，创建并写入默认用户
    if (!fileInfo.exists()) {
        QFile file(userFilePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "admin,admin123,admin\n";
            file.close();
        }
    }
    if(!logfileInfo.exists()){
        QFile file(logFilePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "Initialize success\n";
            file.close();
        }
    }
}
