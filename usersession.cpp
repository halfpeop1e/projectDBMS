#include "usersession.h"

namespace Session {
void setCurrentUser(const QString& user,Auth::Role role) {
    currentUser = user;
    currentRole = role;
}


Auth::Role getCurrentRole(){
    return currentRole;
}
QString getCurrentUser(){
    return currentUser;
}
}
