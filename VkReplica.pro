#-------------------------------------------------
#
# Project created by QtCreator 2016-10-25T16:58:22
#
#-------------------------------------------------

QT       += core gui webenginewidgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VkReplica
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    vklongpollserver.cpp

HEADERS  += mainwindow.h \
    vklongpollserver.h

FORMS    += mainwindow.ui
