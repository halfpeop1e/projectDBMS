/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QGroupBox *showarea;
    QTextBrowser *shell;
    QLineEdit *input;
    QPushButton *send;
    QGroupBox *functionarea;
    QPushButton *clear;
    QPushButton *load;
    QPushButton *show_database;
    QPushButton *selectall;
    QLineEdit *fastinput;
    QPushButton *comfirm;
    QPushButton *cancel;
    QLabel *inputDBname;
    QTreeView *treeView;
    QTextBrowser *contentdisplay;
    QMenuBar *menubar;
    QMenu *menuminiDB;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(917, 670);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        showarea = new QGroupBox(centralwidget);
        showarea->setObjectName("showarea");
        showarea->setGeometry(QRect(470, 210, 421, 391));
        shell = new QTextBrowser(showarea);
        shell->setObjectName("shell");
        shell->setGeometry(QRect(20, 30, 381, 321));
        input = new QLineEdit(showarea);
        input->setObjectName("input");
        input->setGeometry(QRect(20, 350, 331, 31));
        send = new QPushButton(showarea);
        send->setObjectName("send");
        send->setGeometry(QRect(350, 350, 51, 31));
        functionarea = new QGroupBox(centralwidget);
        functionarea->setObjectName("functionarea");
        functionarea->setGeometry(QRect(470, 0, 421, 211));
        clear = new QPushButton(functionarea);
        clear->setObjectName("clear");
        clear->setGeometry(QRect(110, 40, 101, 51));
        load = new QPushButton(functionarea);
        load->setObjectName("load");
        load->setGeometry(QRect(300, 40, 91, 51));
        show_database = new QPushButton(functionarea);
        show_database->setObjectName("show_database");
        show_database->setGeometry(QRect(10, 40, 101, 51));
        selectall = new QPushButton(functionarea);
        selectall->setObjectName("selectall");
        selectall->setGeometry(QRect(210, 40, 91, 51));
        fastinput = new QLineEdit(functionarea);
        fastinput->setObjectName("fastinput");
        fastinput->setGeometry(QRect(20, 130, 151, 31));
        comfirm = new QPushButton(functionarea);
        comfirm->setObjectName("comfirm");
        comfirm->setGeometry(QRect(170, 130, 51, 31));
        cancel = new QPushButton(functionarea);
        cancel->setObjectName("cancel");
        cancel->setGeometry(QRect(220, 130, 51, 31));
        inputDBname = new QLabel(functionarea);
        inputDBname->setObjectName("inputDBname");
        inputDBname->setEnabled(true);
        inputDBname->setGeometry(QRect(20, 110, 111, 16));
        treeView = new QTreeView(centralwidget);
        treeView->setObjectName("treeView");
        treeView->setGeometry(QRect(10, 10, 441, 321));
        contentdisplay = new QTextBrowser(centralwidget);
        contentdisplay->setObjectName("contentdisplay");
        contentdisplay->setGeometry(QRect(10, 330, 441, 261));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 917, 43));
        menuminiDB = new QMenu(menubar);
        menuminiDB->setObjectName("menuminiDB");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menuminiDB->menuAction());

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        showarea->setTitle(QCoreApplication::translate("MainWindow", "\346\230\276\347\244\272\345\214\272", nullptr));
        input->setText(QString());
        send->setText(QCoreApplication::translate("MainWindow", "\347\241\256\345\256\232", nullptr));
        functionarea->setTitle(QCoreApplication::translate("MainWindow", "\345\212\237\350\203\275\345\214\272", nullptr));
        clear->setText(QCoreApplication::translate("MainWindow", "\345\277\253\346\215\267\345\210\240\351\231\244", nullptr));
        load->setText(QCoreApplication::translate("MainWindow", "\345\212\240\350\275\275\350\241\250", nullptr));
        show_database->setText(QCoreApplication::translate("MainWindow", "\346\225\260\346\215\256\345\272\223\345\277\253\346\215\267\346\267\273\345\212\240", nullptr));
        selectall->setText(QCoreApplication::translate("MainWindow", "\345\205\250\351\200\211", nullptr));
        comfirm->setText(QCoreApplication::translate("MainWindow", "\347\241\256\345\256\232", nullptr));
        cancel->setText(QCoreApplication::translate("MainWindow", "\345\217\226\346\266\210", nullptr));
        inputDBname->setText(QCoreApplication::translate("MainWindow", "\350\257\267\350\276\223\345\205\245\346\225\260\346\215\256\345\272\223\345\220\215\357\274\201", nullptr));
        menuminiDB->setTitle(QCoreApplication::translate("MainWindow", "miniDB", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
