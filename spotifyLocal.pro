#-------------------------------------------------
#
# Project created by QtCreator 2019-07-29T19:43:02
#
#-------------------------------------------------

QT       += core gui network core-private core xml multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = oauth
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += /usr/include/qt/QtSql
INCLUDEPATH += /usr/include/qt/QtWidgets
INCLUDEPATH += /usr/include/qt/QtPrintSupport


SOURCES += main.cpp\
        SpotifySearch.cpp \
        mainwindow.cpp \
        qabstractoauth.cpp \
        qabstractoauth2.cpp \
        qabstractoauthreplyhandler.cpp \
        qoauth1.cpp \
        qoauth1signature.cpp \
        qoauth2authorizationcodeflow.cpp \
        qoauthhttpserverreplyhandler.cpp \
        qoauthoobreplyhandler.cpp

HEADERS  += mainwindow.h \
    SpotifySearch.h \
    clientid.h \
    qabstractoauth.h \
    qabstractoauth2.h \
    qabstractoauth2_p.h \
    qabstractoauth_p.h \
    qabstractoauthreplyhandler.h \
    qoauth1.h \
    qoauth1_p.h \
    qoauth1signature.h \
    qoauth1signature_p.h \
    qoauth2authorizationcodeflow.h \
    qoauth2authorizationcodeflow_p.h \
    qoauthglobal.h \
    qoauthhttpserverreplyhandler.h \
    qoauthhttpserverreplyhandler_p.h \
    qoauthoobreplyhandler.h

FORMS    += mainwindow.ui

SUBDIRS += \
    oauth.pro

DISTFILES += \
    .gitignore \
    LICENSE \
    webapi.pro.user

RESOURCES += \
    resource.qrc
