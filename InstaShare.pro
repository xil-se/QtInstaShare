QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DESTDIR    = bin
TARGET     = InstaShare
TEMPLATE   = app
CONFIG    += c++11

SOURCES   += main.cpp \
             urldialog.cpp \
             uploaddialog.cpp

HEADERS   += urldialog.h \
             uploaddialog.h

FORMS     += urldialog.ui \
             uploaddialog.ui

RESOURCES += res/res.qrc
