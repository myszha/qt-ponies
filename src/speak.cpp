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

#include <sstream>
#include <iostream>

#include "speak.h"
#include "pony.h"

// audio: http://developer.qt.nokia.com/doc/qt-4.7/qtmultimedia.html

Speak::Speak(Pony* parent, const QString filepath, const std::vector<QString> &options)
    :parent(parent), path(filepath)/*, music(nullptr)*/
{

    if(options.size() == 2) { // Speak, "text"
        text = options[1];
        skip_normally = false;
    }else{ // Speak, name, "text"
        name = options[1].toLower();
        text = options[2];

        if(options.size()>3){ // Speak, name, "text", {"file.mp3", "file.ogg"}, skip_normally
            // TODO: parse all of the soundfiles names
            // for now, we get only the first
            if(options[3] != "") {
                soundfile = options[3].section(',',0,0);
            }

            skip_normally = (options[4].compare("true", Qt::CaseInsensitive) == 0)?true:false;
        }
    }
}

void Speak::play()
{
    //TODO: play the media file
    /*if(music == nullptr)
    music = Phonon::createPlayer(Phonon::MusicCategory,
                             Phonon::MediaSource(QString::fromStdString("desktop-ponies/" + path + "/" + soundfile)));
    music->play();*/
}
