#include <sstream>
#include <iostream>

#include "speak.h"
#include "pony.h"

Speak::Speak(Pony* parent, const std::string filepath, const std::vector<std::string> &options)
    :parent(parent), path(filepath)/*, music(nullptr)*/
{

    name = options[1];
    text = options[2];

    if(options.size()>3){
        // TODO: parse all of the soundfiles names
        // for now, we get only the first
        if(options[3] != ""){
            soundfile = std::move(std::string(options[3], 0, options[3].find(',')));
        }

        std::string lower(options[4]);
        for(auto &i: lower){ i = std::tolower(i); }
        skip_normally = lower == "true"?true:false;
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
