/*
 * Qt-ponies - ponies on the desktop
 * Copyright (C) 2013 funeralismatic, XRevan86
 * Copyright (C) 2012-2013 mysha
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

#ifndef SPEAK_H
#define SPEAK_H

#include <QString>
#include <QObject>
#include <QList>

#include <vector>
#include <memory>

#include "csv_parser.h"

class Pony;

namespace Phonon {
    class AudioOutput;
    class MediaObject;
}

class Speak : public QObject
{
    Q_OBJECT
public:
    Speak(Pony* parent, const QString filepath, const std::vector<QVariant> &options);
    ~Speak();

    void play();

    static const CSVParser::ParseTypes OptionTypes;

    QString name;
    QString text;
    QList<QVariant> soundfiles;
    bool skip_normally;

public slots:
    void stop();

private:
    Pony* parent;
    QString path;

    Phonon::AudioOutput *audioOutput;
    Phonon::MediaObject *mediaObject;
};

#endif // Speak_H
