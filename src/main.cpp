/*
 * Qt-ponies - ponies on the desktop
 * Copyright (C) 2012 mysha
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtGui/QApplication>
#include <QFile>
#include <iostream>

#include "configwindow.h"
#include "pony.h"

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
