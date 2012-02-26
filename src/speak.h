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

#ifndef SPEAK_H
#define SPEAK_H

#include <vector>
#include <string>

//#include <Phonon/MediaObject>

class Pony;

class Speak
{
public:
    Speak(Pony* parent, const std::string filepath, const std::vector<std::string> &options);

    void play();

    std::string name;
    std::string text;
    std::string soundfile;
    bool skip_normally;

private:
    Pony* parent;
    std::string path;
//    Phonon::MediaObject *music;
};

#endif // Speak_H
