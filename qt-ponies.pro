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
    src/csv_parser.cpp \
    src/interaction.cpp

HEADERS  += \
    src/pony.h \
    src/behavior.h \
    src/effect.h \
    src/speak.h \    
    src/configwindow.h \
    src/csv_parser.h \
    src/interaction.h

FORMS += \
    src/configwindow.ui

TRANSLATIONS = \
    translations/qt-ponies_de.ts \
    translations/qt-ponies_pl.ts \
    translations/qt-ponies_fr.ts \
    translations/qt-ponies_el.ts
                  
RESOURCES += \
    qt-ponies.qrc

unix {
  BINDIR = $${PREFIX}/bin
  DATADIR =$${PREFIX}/share

  target.path = $$BINDIR

  icon.path = $$DATADIR/pixmaps/
  icon.files += res/$${TARGET}.png

  desktop.path = $$DATADIR/applications/
  desktop.files = res/$${TARGET}.desktop

  data.path = $$DATADIR/qt-ponies
  data.files += desktop-ponies

  INSTALLS += target icon desktop data
}
