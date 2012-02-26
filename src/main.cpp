#include <QtGui/QApplication>
#include <QFile>
#include <iostream>

#include "configwindow.h"
#include "pony.h"

void remove_pony()
{

}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());
    QCoreApplication::setOrganizationName("qt-ponies");
    QCoreApplication::setApplicationName("qt-ponies");

    app.setQuitOnLastWindowClosed(false);

//    QFile qss(":/styles/res/style.qss");
    QFile qss("res/style.qss");
    qss.open(QFile::ReadOnly);
    app.setStyleSheet( QString::fromUtf8(qss.readAll()) );
    qss.close();


    ConfigWindow config;

    if(config.ponies.size() == 0) {
        config.show();
    }

    return app.exec();
}
