#ifndef USERSPACE_H
#define USERSPACE_H
#include <QString>

namespace User {
bool signup(const QString& username, const QString& password);
bool login(QString &user, QString &username1, QString &password1);
bool grantRole(const QString& adminUser, const QString& username, const QString& role);
}// namespace User

#endif // USERSPACE_H
