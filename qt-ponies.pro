
QT       += core gui

TARGET = qt-ponies
OBJECTS_DIR = bin
MOC_DIR = src/moc
UI_DIR = src/ui
RCC_DIR = src/rcc
TEMPLATE = app

#CONFIG += static

QMAKE_CXXFLAGS += -std=c++0x

SOURCES += src/main.cpp \
    src/pony.cpp \
    src/behavior.cpp \
    src/effect.cpp \
    src/speak.cpp \ 
    src/configwindow.cpp

HEADERS  += \
    src/pony.h \
    src/behavior.h \
    src/effect.h \
    src/speak.h \    
    src/configwindow.h \
    src/csv_parser.h

FORMS += \
    src/configwindow.ui

RESOURCES += \
    qt-ponies.qrc
