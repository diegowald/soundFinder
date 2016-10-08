#-------------------------------------------------
#
# Project created by QtCreator 2016-10-07T09:15:02
#
#-------------------------------------------------

TEMPLATE = app
TARGET = soundRecorder

QT += multimedia

win32:INCLUDEPATH += $$PWD

HEADERS = \
    audiorecorder.h \
    qaudiolevel.h

SOURCES = \
    main.cpp \
    audiorecorder.cpp \
    qaudiolevel.cpp

FORMS += audiorecorder.ui

QT+=widgets
