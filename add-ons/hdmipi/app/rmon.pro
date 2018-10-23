#-------------------------------------------------
#
# Project created by QtCreator 2018-04-25T21:40:53
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = rmon
TEMPLATE = app
CONFIG 	   += c++11

SUBDIRS = \
    QSsh

INCLUDEPATH += QSsh/src/libs/ssh

SOURCES += main.cpp\
        rmon.cpp \
    inputfilter.cpp \
    kbdkey.cpp \
    connectiondialog.cpp

HEADERS  += rmon.h \
    inputfilter.h \
    kbdkey.h \
    connectiondialog.h

FORMS    += rmon.ui \
    connectiondialog.ui

LIBS       += -lVLCQtCore -lVLCQtWidgets -L/lib/qtcreator/ -lQSsh
