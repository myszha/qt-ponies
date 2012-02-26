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
