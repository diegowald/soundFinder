#-------------------------------------------------
#
# Project created by QtCreator 2016-10-09T07:01:39
#
#-------------------------------------------------

#QT       += core gui webenginewidgets
QT       += webenginewidgets
#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = soundFinderClient
TEMPLATE = app


SOURCES += main.cpp\
    browser.cpp \
    browserwindow.cpp \
    tabwidget.cpp \
    urllineedit.cpp \
    webview.cpp \
    webpage.cpp \
    webpopupwindow.cpp

HEADERS  +=  \
    browser.h \
    browserwindow.h \
    tabwidget.h \
    urllineedit.h \
    webview.h \
    webpage.h \
    webpopupwindow.h

FORMS    += \
    certificateerrordialog.ui \
    passworddialog.ui

RESOURCES += data/simplebrowser.qrc

#CONFIG += mobility c++11
CONFIG += c++11


