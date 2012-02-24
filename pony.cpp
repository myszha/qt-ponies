//#include <QGraphicsEffect>
#include <QApplication>
#include <QDesktopWidget>
#include <QString>
#include <QDateTime>
#include <QMenu>

#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>

#include <cmath>

#include "pony.h"
#include "csv_parser.h"
#include "configwindow.h"

Pony::Pony(const std::string path, ConfigWindow *config, QWidget *parent) :
    QMainWindow(parent), label(this), gen(rd()), config(config)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);
#ifdef Q_WS_X11
    setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint);
#endif

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(display_menu(const QPoint &)));

    text_label.hide();
    text_label.setAlignment(Qt::AlignHCenter);
    text_label.setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    text_label.setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
#ifdef Q_WS_X11
    text_label.setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint | Qt::ToolTip);
#endif

    move(QApplication::desktop()->width()/2,QApplication::desktop()->height()/2);
    x_center = x()+25;
    y_center = y()+25;


    std::ifstream ifile;
    try {
        ifile.open("desktop-ponies/" + path + "/pony.ini");
    } catch (std::ifstream::failure e) {
        std::cerr << "ERROR: Cannot open pony.ini for pony: '"<< path << "'" << std::endl;
        std::cerr << e.what() << std::endl;
        throw std::exception();
    }

    if( ifile.is_open() ) {
        std::string line;

        while (!ifile.eof() ) {
            std::getline(ifile, line);

            if(line[0] != '\'') {
                std::vector<std::string> csv_data;
                csvline_populate(csv_data, line, ',');

                if(csv_data[0] == "Name") {
                    name = csv_data[1]; //Name,"name"
                }
                else if(csv_data[0] == "Behavior") {
                    Behavior b(this, path, csv_data);
                    behaviors.insert({b.name, std::move(b)});
                }
                else if(csv_data[0] == "Speak") {
                    Speak s(this, path, csv_data);
                    speak_lines.insert({s.name, std::move(s)});
                }
            }
        }

        ifile.close();
    }else{
        std::cerr << "ERROR: Cannot read pony.ini for pony: '"<< path << "'" << std::endl;
        throw std::exception();
    }

    // Select behaviour that will can be choosen randomly
    for(auto &i: behaviors) {
        if(i.second.skip_normally == false) {
            random_behaviors.push_back(&i.second);
        }
    }

    std::sort(random_behaviors.begin(), random_behaviors.end(), [](const Behavior *val1, const Behavior *val2){ return val1->probability < val2->probability;} );
    total_behavior_probability = 0;
    for(auto &i: random_behaviors) {
        total_behavior_probability += i->probability;
    }

    // Select speech line that will can be choosen randomly
    for(auto &i: speak_lines) {
        if(i.second.skip_normally == false) {
            random_speak_lines.push_back(&i.second);
        }
    }

    current_behavior = nullptr;
    change_behavior();
    this->show();

}

Pony::~Pony()
{
}

std::shared_ptr<Pony> Pony::get_shared_ptr()
{
    return shared_from_this();
}

void Pony::display_menu(const QPoint &pos)
{
    QMenu *menu = new QMenu(this);
    menu->addAction(QString::fromStdString(name))->setEnabled(false);
    menu->addSeparator();
    menu->addAction("Remove pony", config, SLOT(remove_pony()));
    menu->addAction("Remove every pony", config, SLOT(remove_pony_all()));
    menu->exec(mapToGlobal(pos));
}

void Pony::update_animation(QMovie* animation)
{
    label.setMovie(animation);
    resize(animation->currentImage().size());
    label.resize(animation->currentImage().size());
    label.repaint();
    // FIXME: sometimes an old or wrong frame is visible when changing animations
}

void Pony::change_behavior()
{
    if(current_behavior != nullptr) {
        current_behavior->deinit();
    }

    // Check if linked behavior is present
    if(current_behavior != nullptr && current_behavior->linked_behavior != "") {
        if( behaviors.find(current_behavior->linked_behavior) == behaviors.end()) {
            std::cerr << "ERROR: Pony: '"<<name<<"' linked behavior:'"<< current_behavior->linked_behavior << "' from: '"<< current_behavior->name << "' not present."<<std::endl;
        }else{
            current_behavior = &behaviors.at(current_behavior->linked_behavior);
        }
    }else{
        // If linked behavior not present, select random behavior using roulette-wheel selection
        float total = 0;
        std::uniform_real_distribution<> dis(0, total_behavior_probability);
        float rnd = dis(gen);
        for(auto &i: random_behaviors){
            total += i->probability;
            if(rnd <= total) {
                current_behavior = i;
                break;
            }
        }

    }

    int64_t dur_min = std::round(current_behavior->duration_min*1000);
    int64_t dur_max = std::round(current_behavior->duration_max*1000);
    if(dur_min == dur_max) {
        behavior_duration = dur_min;
    }else{
        std::uniform_int_distribution<> dis(dur_min, dur_max);
        behavior_duration = dis(gen);
    }


    //current_behavior->info();
    std::cout << "Pony: '"<<name<<"' behavior: '"<< current_behavior->name <<"' for " << behavior_duration << "msec" <<std::endl;

    behavior_started = QDateTime::currentMSecsSinceEpoch();
    current_behavior->init();
    update_animation(current_behavior->current_animation);

    // Select speech line to display
    // starting_line for current behavior or random
    Speak* current_speech_line = nullptr;
    if(current_behavior->starting_line != ""){
        if( speak_lines.find(current_behavior->starting_line) == speak_lines.end()) {
            std::cerr << "ERROR: Pony: '"<<name<<"' starting line:'"<< current_behavior->starting_line<< "' from: '"<< current_behavior->name << "' not present."<<std::endl;
        }else{
            current_speech_line = &speak_lines.at(current_behavior->starting_line);
        }
    }else{
        std::uniform_int_distribution<> int_dis(0, random_speak_lines.size()-1);
        current_speech_line = random_speak_lines[int_dis(gen)];
    }

    text_label.setText(QString::fromStdString(current_speech_line->text));
    speech_started = behavior_started;
    text_label.adjustSize();
    text_label.move(x_center-text_label.width()/2, y() - text_label.height());
    text_label.show();
    current_speech_line->play();
}

void Pony::update() {
    int64_t time = QDateTime::currentMSecsSinceEpoch();

    // Check for speech timeout and move text with pony
    if(speech_started+2000 <= time) {
        text_label.hide();
    }else{
        text_label.move(x_center-text_label.width()/2, y() - text_label.height());
    }

    // Check behavior timeout
    if(behavior_started+behavior_duration <= time){
        change_behavior();
    }
    current_behavior->update();
}
