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

#include <QApplication>
#include <QDesktopWidget>
#include <QWidget>
#include <QDateTime>

#include <string>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <random>
#include <cctype>
#include <cmath>

#include "behavior.h"
#include "pony.h"

Behavior::Behavior(Pony* parent, const std::string filepath, const std::vector<std::string> &options)
    : path(filepath), parent(parent)
{
/*
        name = 1
        probability = 2
        max_duration = 3
        min_duration = 4
        speed = 5 'specified in pixels per tick of the timer
        right_image_path = 6
        left_image_path = 7
        movement_type = 8
        linked_behavior = 9
        speaking_start = 10
        speaking_end = 11
        skip = 12 'Should we skip this behavior when considering ones to randomly choose (part of an interaction/chain?)
        xcoord = 13  'used when following/moving to a point on the screen.
        ycoord = 14
        object_to_follow = 15
        auto_select_images = 16 // no idea what it is
        follow_stopped_behavior = 17 // no idea what it is
        follow_moving_behavior = 18 // no idea what it is
        right_image_center = 19
        left_image_center = 20
*/

    static std::unordered_map<std::string, Movement> movement_map = {
        {"none", Movement::None},
        {"horizontal_only", Movement::Horizontal},
        {"vertical_only", Movement::Vertical},
        {"horizontal_vertical", Movement::Horizontal_Vertical},
        {"diagonal_only", Movement::Diagonal},
        {"diagonal_horizontal", Movement::Diagonal_Horizontal},
        {"diagonal_vertical", Movement::Diagonal_Vertical},
        {"all", Movement::All},
        {"mouseover", Movement::MouseOver},
        {"sleep", Movement::Sleep},
        {"dragged", Movement::Dragged}
    };

    name = options[1];
    std::istringstream(options[2]) >> probability;
    std::istringstream(options[3]) >> duration_max;
    std::istringstream(options[4]) >> duration_min;
    std::istringstream(options[5]) >> speed;
    animation_right = options[6];
    animation_left = options[7];

    std::string lower(options[8]);
    for(auto &i: lower){ i = std::tolower(i); }
    movement_allowed = movement_map[lower];

    if( options.size() > 9 ) {
        linked_behavior = options[9];
        starting_line = options[10];
        ending_line = options[11];

        std::string lower(options[12]);
        for(auto &i: lower){ i = std::tolower(i); }
        skip_normally = lower == "true"?true:false;

        std::istringstream(options[13]) >> x_coordinate;
        std::istringstream(options[14]) >> y_coordinate;
        follow_object = options[15];

        if( options.size() > 16 ) {
            // Extract lefft/right image centers
            std::string tmp_field;
            int x,y;

            std::istringstream iss(options[19]);

            std::getline(iss,tmp_field,',');
            std::istringstream(tmp_field) >> x;
            std::getline(iss,tmp_field,',');
            std::istringstream(tmp_field) >> y;
            right_image_center = QPoint(x,y);

            std::istringstream iss2(options[20]);

            std::getline(iss2,tmp_field,',');
            std::istringstream(tmp_field) >> x;
            std::getline(iss2,tmp_field,',');
            std::istringstream(tmp_field) >> y;
            left_image_center = QPoint(x,y);
        }else{
            right_image_center = QPoint(0,0);
            left_image_center = QPoint(0,0);
        }

    }

    current_animation = nullptr;
    animations[0] = nullptr;
    animations[1] = nullptr;

    QDesktopWidget *desktop = QApplication::desktop();
    desktop_width = desktop->width();
    desktop_height = desktop->height();

}

Behavior::Behavior(Behavior &&b)
{
    *this = std::move(b);
    b.animations[0] = nullptr;
    b.animations[1] = nullptr;
}

Behavior::~Behavior()
{
    if(animations[0] != nullptr) delete animations[0];
    if(animations[1] != nullptr) delete animations[1];
}

void Behavior::choose_angle()
{
    std::mt19937 gen(QDateTime::currentMSecsSinceEpoch());

    if(direction_v == Direction::Up){
        std::uniform_real_distribution<> dis(15, 50);
        angle = dis(gen) * M_PI / 180.0;
    }
    if(direction_v == Direction::Down){
        std::uniform_real_distribution<> dis(310, 345);
        angle = dis(gen) * M_PI / 180.0;
    }
    if(direction_h == Direction::Left) {
        angle = M_PI - angle;
    }
}

void Behavior::init()
{
    movement = Movement::None;
    std::mt19937 gen(QDateTime::currentMSecsSinceEpoch());

    // Load animations and verify them
    animations[0] = new QMovie(QString::fromStdString("desktop-ponies/" + path + "/" + animation_left ));
    animations[1] = new QMovie(QString::fromStdString("desktop-ponies/" + path + "/" + animation_right));

    if(!animations[0]->isValid())
        std::cerr << "ERROR: Pony: '"<< path <<"' Error opening left animation:'"<< animation_left << "' for behavior: '"<< name << "'."<<std::endl;
    if(!animations[1]->isValid())
        std::cerr << "ERROR: Pony: '"<< path <<"' Error opening right animation:'"<< animation_right << "' for behavior: '"<< name << "'."<<std::endl;

    animations[0]->setCacheMode(QMovie::CacheAll);
    animations[1]->setCacheMode(QMovie::CacheAll);

    // If we do not have the centers of images from configuration, then set them to width/2, height/2
    if(left_image_center.x() == 0 && left_image_center.y() == 0) {
        // TODO: currentImage() is none when animation is not started
        // maybe it can be done by setting the move frame to 1 by hand?
        animations[0]->start();
        left_image_center = QPoint(animations[0]->currentImage().width()/2,animations[0]->currentImage().height()/2);
        animations[0]->stop();
    }
    if(right_image_center.x() == 0 && right_image_center.y() == 0) {
        animations[1]->start();
        right_image_center = QPoint(animations[1]->currentImage().width()/2,animations[1]->currentImage().height()/2);
        animations[1]->start();
    }


    if(movement_allowed != Movement::None && movement_allowed != Movement::MouseOver && movement_allowed != Movement::Sleep && movement_allowed != Movement::Dragged){
        std::vector<Behavior::Movement> modes;

        if(movement_allowed & Movement::Horizontal) modes.push_back(Movement::Horizontal);
        if(movement_allowed & Movement::Vertical)   modes.push_back(Movement::Vertical);
        if(movement_allowed & Movement::Diagonal)   modes.push_back(Movement::Diagonal);

        movement = modes[gen()%modes.size()];
    }

    direction_h = gen()%2 == 0? Left : Right;
    direction_v = gen()%2 == 0? Up : Down;

    if(direction_h == Right) {
        x_center = right_image_center.x();
        y_center = right_image_center.y();
    }else{
        x_center = left_image_center.x();
        y_center = left_image_center.y();
    }


    if(movement == Movement::Diagonal) {
        choose_angle();
    }

    // Move window due to change in image center
    parent->move(parent->x_center-x_center,parent->y_center-y_center);

    current_animation = animations[direction_h<0?0:1];
    current_animation->start();
    width = current_animation->currentImage().size().width();
    height = current_animation->currentImage().size().height();
}

void Behavior::deinit()
{
    current_animation->stop();
    if(animations[0] != nullptr) delete animations[0];
    if(animations[1] != nullptr) delete animations[1];

    animations[0] = nullptr;
    animations[1] = nullptr;
}

void Behavior::update()
{
    // No need to change position if we can't move
    if(movement == Movement::None) return;

    float vel_x = direction_h * speed;
    float vel_y = direction_v * speed;

    if(parent->x() <= 0) {
        current_animation->stop();
        current_animation = animations[1];
        current_animation->start();
        parent->update_animation(current_animation);
        width = current_animation->currentImage().size().width();
        height = current_animation->currentImage().size().height();
        direction_h = Direction::Right;
        x_center = right_image_center.x();
        y_center = right_image_center.y();
        if(movement == Movement::Diagonal) choose_angle();
    }
    if(parent->x() >= desktop_width - width) {
        current_animation->stop();
        current_animation = animations[0];
        current_animation->start();
        parent->update_animation(current_animation);
        width = current_animation->currentImage().size().width();
        height = current_animation->currentImage().size().height();
        direction_h = Direction::Left;
        x_center = left_image_center.x();
        y_center = left_image_center.y();
        if(movement == Movement::Diagonal) choose_angle();
    }

    if(parent->y() <= 0){
        direction_v = Behavior::Direction::Down;
        if(movement == Movement::Diagonal) choose_angle();
    }
    if(parent->y() >= desktop_height - height){
        direction_v = Behavior::Direction::Up;
        if(movement == Movement::Diagonal) choose_angle();
    }

    if(movement == Movement::Horizontal){
        parent->x_center += vel_x;
        parent->move(parent->x_center-x_center,parent->y_center-y_center);
    }
    if(movement == Movement::Vertical){
        parent->y_center += vel_y;
        parent->move(parent->x_center-x_center,parent->y_center-y_center);
    }
    if(movement == Movement::Diagonal){
        vel_x = std::sqrt(speed*speed*2) * std::cos(angle);
        vel_y = -std::sqrt(speed*speed*2) * std::sin(angle);
        parent->x_center += vel_x;
        parent->y_center += vel_y;
        parent->move(parent->x_center-x_center,parent->y_center-y_center);
    }

}

void Behavior::info()
{
std::cout
    <<"name: "<<name<<" "
    <<"speed: "<<speed<<" "
    <<"probability: "<<probability<<" "
    <<"duration_min: "<<duration_min<<" "
    <<"duration_max: "<<duration_max<<" "
    <<"movement_allowed: "<<int(movement_allowed)<<" "
    <<"animation_left: "<<animation_left<<" "
    <<"animation_right: "<<animation_right<<" "
    <<"linked_behavior: "<<linked_behavior<<" "
    <<"starting_line: "<<starting_line<<" "
    <<"ending_line: "<<ending_line<<" "
    <<"skip_normally: "<<skip_normally<<" "
    <<"x_coordinate: "<<x_coordinate<<" "
    <<"y_coordinate: "<<y_coordinate<<" "
    <<"follow_object: "<<follow_object<<std::endl;

}
