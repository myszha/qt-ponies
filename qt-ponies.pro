
QT       += core gui

#TODO: change to pkgsrc when Qt 4.8 is available
PHONON=""

!isEmpty(PHONON){
    QT += phonon
    DEFINES += USE_PHONON
}

TARGET = qt-ponies
OBJECTS_DIR = bin
MOC_DIR = src/moc
UI_DIR = src/ui
RCC_DIR = src/rcc
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++0x

SOURCES += src/main.cpp \
    src/pony.cpp \
    src/behavior.cpp \
    src/effect.cpp \
    src/speak.cpp \ 
    src/configwindow.cpp \
    src/csv_parser.cpp

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
