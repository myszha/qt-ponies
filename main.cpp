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
    QCoreApplication::setOrganizationName("Myshsoft");
    QCoreApplication::setApplicationName("qt-ponies");

    app.setQuitOnLastWindowClosed(false);

//    QFile qss(":/styles/style.qss");
        QFile qss("style.qss");
    qss.open(QFile::ReadOnly);
    std::cout<<qss.size()<<std::endl;
    app.setStyleSheet( QString::fromUtf8(qss.readAll()) );
    qss.close();


    ConfigWindow config;

    if(config.ponies.size() == 0) {
        config.show();
    }

    return app.exec();
}
