QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
QMAKE_CXXFLAGS += -finput-charset=UTF-8
QMAKE_LFLAGS += -finput-charset=UTF-8
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dbmaintenance.cpp \
    dbmsoperate.cpp \
    globals.cpp \
    interprete.cpp \
    main.cpp \
    mainwindow.cpp \
    toolfunction.cpp \
    usersession.cpp \
    userspace.cpp

HEADERS += \
    dbmaintenance.h \
    dbmsoperate.h \
    globals.h \
    interprete.h \
    mainwindow.h \
    toolfunction.h \
    usersession.h \
    userspace.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    ../Resource/log.txt \
    ../Resource/username/dd/121.txt \
    ../Resource/users.txt
