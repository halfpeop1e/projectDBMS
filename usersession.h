#ifndef USERSESSION_H
#define USERSESSION_H
#include "globals.h"
namespace Session {
void setCurrentUser(const QString& user,Auth::Role role);
Auth::Role getCurrentRole();
QString getCurrentUser();
}

#endif // USERSESSION_H
