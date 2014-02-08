QT += core gui network

lessThan(QT_MAJOR_VERSION, 5) {
    DEFINES += IS_QT4
    packagesExist(phonon) {
        QT += phonon
        DEFINES += USE_PHONON
    }
    unix:!macx {
        LIBS += -lX11 -lXfixes
    }
} else {
    QT += widgets
    packagesExist(phonon4qt5) {
        QT += phonon4qt5
        DEFINES += USE_PHONON
    }
}

TARGET = qt-ponies
OBJECTS_DIR = bin
MOC_DIR = src/moc
UI_DIR = src/ui
RCC_DIR = src/rcc
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++0x -Wextra

SOURCES += src/main.cpp \
    src/pony.cpp \
    src/behavior.cpp \
    src/effect.cpp \
    src/speak.cpp \
    src/configwindow.cpp \
    src/csv_parser.cpp \
    src/interaction.cpp \
    src/debugwindow.cpp \
    src/singleapplication.cpp

HEADERS  += \
    src/pony.h \
    src/behavior.h \
    src/effect.h \
    src/speak.h \
    src/configwindow.h \
    src/csv_parser.h \
    src/interaction.h \
    src/debugwindow.h \
    src/singleapplication.h

FORMS += \
    src/configwindow.ui \
    src/debugwindow.ui

TRANSLATIONS = \
    translations/qt-ponies_de.ts \
    translations/qt-ponies_ru.ts \
    translations/qt-ponies_pl.ts \
    translations/qt-ponies_fr.ts \
    translations/qt-ponies_el.ts

RESOURCES += qt-ponies.qrc

unix {
    isEmpty(PREFIX) {
        PREFIX = /usr
    }

    BINDIR = $${PREFIX}/bin
    DATADIR =$${PREFIX}/share

    target.path = $$BINDIR

    icon.path = $$DATADIR/pixmaps/
    icon.files += res/$${TARGET}.png

    desktop.path = $$DATADIR/applications/
    desktop.files = res/$${TARGET}.desktop

    data.path = $$DATADIR/qt-ponies
    data.files += desktop-ponies

    translations.path = $$DATADIR/qt-ponies/translations/
    translations.files = translations/*.qm

    INSTALLS += target icon desktop data translations
}
