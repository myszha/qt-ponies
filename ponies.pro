#-------------------------------------------------
#
# Project created by QtCreator 2011-12-13T09:50:14
#
#-------------------------------------------------

QT       += core gui

TARGET = ponies
TEMPLATE = app

#CONFIG += static

QMAKE_CXXFLAGS += -std=c++0x

SOURCES += main.cpp \
    pony.cpp \
    behavior.cpp \
    effect.cpp \
    speak.cpp \ 
    configwindow.cpp

HEADERS  += \
    pony.h \
    behavior.h \
    effect.h \
    speak.h \    
    configwindow.h \
    csv_parser.h

FORMS += \
    configwindow.ui
