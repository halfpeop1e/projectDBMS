QT += core network sql
QT -= gui
QT += widgets  # 关键！添加 widgets 模块
QT += widgets network sql
CONFIG += c++11 console
CONFIG -= app_bundle

TARGET = DBMSServer
TEMPLATE = app

SOURCES += \
    databaseserver.cpp \
    main.cpp \
    serverwindow.cpp

HEADERS += \
    databaseserver.h \
    serverwindow.h

FORMS += \
    serverwindow.ui
