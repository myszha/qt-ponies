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

Pony::Pony(const std::string path, QWidget *parent) :
    QMainWindow(parent), label(this), gen(rd())
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);
#ifdef Q_WS_X11
    setWindowFlags(Qt::X11BypassWindowManagerHint);
#endif

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(display_menu(const QPoint &)));

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
    total_probability = 0;
    for(auto &i: random_behaviors) {
        total_probability += i->probability;
    }

    current_behavior = nullptr;
    change_behavior();
    this->show();

}

Pony::~Pony()
{
}

void Pony::display_menu(const QPoint &pos)
{
    QMenu *menu = new QMenu(this);
    menu->addAction("Remove pony", this, SLOT(test_slot()));
    menu->exec(mapToGlobal(pos));
}

void Pony::update_animation(QMovie* animation)
{
    label.setMovie(animation);
    resize(animation->currentImage().size());
    label.resize(animation->currentImage().size());
    label.repaint();
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
        std::uniform_real_distribution<> dis(0, total_probability);
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
        duration = dur_min;
    }else{
        std::uniform_int_distribution<> dis(dur_min, dur_max);
        duration = dis(gen);
    }


    //current_behavior->info();
    std::cout << "Pony: '"<<name<<"' behavior: '"<< current_behavior->name <<"' for " << duration << "msec" <<std::endl;

    started = QDateTime::currentMSecsSinceEpoch();
    current_behavior->init();
    update_animation(current_behavior->current_animation);

}

void Pony::update() {
    if(started+duration <= QDateTime::currentMSecsSinceEpoch()){
        change_behavior();
    }
    current_behavior->update();
}
