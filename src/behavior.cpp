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
#include <QDebug>

#include <string>
#include <sstream>
#include <unordered_map>
#include <random>
#include <cctype>
#include <cmath>
#include <algorithm>

#include "configwindow.h"
#include "behavior.h"
#include "pony.h"

// These are the variable types for Behavior configuration
const CSVParser::ParseTypes Behavior::OptionTypes {
   {                     "type", QVariant::Type::String },
   {                     "name", QVariant::Type::String },
   {              "probability", QVariant::Type::Double },
   {             "max_duration", QVariant::Type::Double },
   {             "min_duration", QVariant::Type::Double },
   {                    "speed", QVariant::Type::Double },
   {         "right_image_path", QVariant::Type::String },
   {          "left_image_path", QVariant::Type::String },
   {            "movement_type", QVariant::Type::String },
   {          "linked_behavior", QVariant::Type::String },
   {           "speaking_start", QVariant::Type::String },
   {             "speaking_end", QVariant::Type::String },
   {                     "skip", QVariant::Type::Bool   },
   {                   "xcoord", QVariant::Type::Int    },
   {                   "ycoord", QVariant::Type::Int    },
   {         "object_to_follow", QVariant::Type::String },
   {       "auto_select_images", QVariant::Type::Bool   },
   {  "follow_stopped_behavior", QVariant::Type::String },
   {   "follow_moving_behavior", QVariant::Type::String },
   {       "right_image_center", QVariant::Type::Point  },
   {        "left_image_center", QVariant::Type::Point  },
};
/*
  line_type = 0
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
  xcoord = 13  // if not following, then % position of screen
  ycoord = 14  // pixel offset from center of following object
  object_to_follow = 15 // pony or effect?
  auto_select_images = 16 // no idea what it is
  follow_stopped_behavior = 17 // behavior name when not moving; used when following other pony
  follow_moving_behavior = 18 // behavior name when moving; used when following other pony
  right_image_center = 19
  left_image_center = 20
*/

Behavior::Behavior(Pony* parent, const QString filepath, const std::vector<QVariant> &options)
    : path(filepath), parent(parent)
{
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

    type = State::Normal;
    moving = true;

    current_animation = nullptr;
    for(int i = 0; i < 4; i++) {
        animations[i] = nullptr;
    }

    // Read behavior options
    name = options[1].toString().toLower();
    probability = options[2].toFloat();
    duration_max = options[3].toFloat();
    duration_min = options[4].toFloat();
    speed = options[5].toString().toFloat();

    animation_right = options[6].toString();
    animation_left = options[7].toString();

    movement_allowed = movement_map[options[8].toString().toLower().toStdString()];

    skip_normally = false;
    x_coordinate = 0;
    y_coordinate = 0;

    if( options.size() > 9 ) {
        linked_behavior = options[9].toString().toLower();
        starting_line = options[10].toString().toLower();
        ending_line = options[11].toString().toLower();

        skip_normally = (options[12].toString().compare("true",Qt::CaseInsensitive) == 0)?true:false;

        x_coordinate = options[13].toInt();
        y_coordinate = options[14].toInt();
        if( x_coordinate != 0 && y_coordinate != 0) {
            type = State::MovingToPoint;
        }

        follow_object = options[15].toString().toLower();
        if(follow_object != "") {
            type = State::Following;
        }

        if( options.size() > 16 ) {
            follow_stopped_behavior = options[17].toString().toLower();
            follow_moving_behavior = options[18].toString().toLower();

            // Extract left/right image centers
            right_image_center = options[19].toPoint();
            left_image_center = options[20].toPoint();
        }else{
            right_image_center = QPoint(0,0);
            left_image_center = QPoint(0,0);
        }

    }

    state = type;

}

Behavior::Behavior(Behavior &&b)
{
    *this = std::move(b);
    for(int i = 0; i < 4; i++) {
        b.animations[i] = nullptr;
    }
}

Behavior::~Behavior()
{
    for(int i = 0; i < 4; i++) {
        if(animations[i] != nullptr) {
            delete animations[i];
            animations[i] = nullptr;
        }
    }
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
    animations[0] = new QMovie(QString("%1/%2/%3").arg(ConfigWindow::getSetting<QString>("general/pony-directory"), path, animation_left ));
    animations[1] = new QMovie(QString("%1/%2/%3").arg(ConfigWindow::getSetting<QString>("general/pony-directory"), path, animation_right));

    if(!animations[0]->isValid())
        qCritical() << "Pony:"<< path <<"Error opening left animation:"<< animation_left << "for behavior:"<< name;
    if(!animations[1]->isValid())
        qCritical() << "Pony:"<< path <<"Error opening right animation:"<< animation_right << "for behavior:"<< name;

    animations[0]->setCacheMode(QMovie::CacheAll);
    animations[1]->setCacheMode(QMovie::CacheAll);

    // If we do not have the centers of images from configuration, then set them to width/2, height/2
    if(left_image_center.x() == 0 && left_image_center.y() == 0) {
        animations[0]->jumpToFrame(0);
        left_image_center = QPoint(animations[0]->currentImage().width()/2,animations[0]->currentImage().height()/2);
    }
    if(right_image_center.x() == 0 && right_image_center.y() == 0) {
        animations[1]->jumpToFrame(0);
        right_image_center = QPoint(animations[1]->currentImage().width()/2,animations[1]->currentImage().height()/2);
    }

    /* Animations:
       0 - left  / follow_moving left
       1 - right / follow_moving right
       2 - follow_stopped left
       3 - follow_stopped right
    */

    // If we are following or moving to point, load additional animations
    if(state == State::Following || state == State::MovingToPoint){

        // Find moving behavior and get left/right filenames from it
        if(follow_moving_behavior == ""){
            // If we do not have a moveing behavior, use standard left/right animations
        }else if( parent->behaviors.find(follow_moving_behavior) == parent->behaviors.end()) {
            qCritical() << "Pony:"<<parent->name<<"follow moving behavior:"<< follow_moving_behavior << "from:"<< name << "not present.";
        }else{
            const Behavior &moving_behavior = parent->behaviors.at(follow_moving_behavior);
            if(moving_behavior.animation_left == "") { // or animation_right==""
                qCritical() << "Pony:"<<parent->name<<"follow moving behavior:"<< follow_moving_behavior << "animation left from:"<< name << "not present.";
            }else{
                // We are not using the animations declared for this behavior, instead we use the ones specified in follow_moving_behavior
                delete animations[0];
                delete animations[1];
                animations[0] = new QMovie(QString("%1/%2/%3").arg(ConfigWindow::getSetting<QString>("general/pony-directory"), path, moving_behavior.animation_left ));
                animations[1] = new QMovie(QString("%1/%2/%3").arg(ConfigWindow::getSetting<QString>("general/pony-directory"), path, moving_behavior.animation_right));

                // Set centers of the moving animations
                left_image_center = moving_behavior.left_image_center;
                right_image_center = moving_behavior.right_image_center;
            }
        }


        // Find stopped behavior and get left/right filenames from it
        if(follow_stopped_behavior == ""){
            animations[2] = new QMovie(QString("%1/%2/%3").arg(ConfigWindow::getSetting<QString>("general/pony-directory"), path, animation_left ));
            animations[3] = new QMovie(QString("%1/%2/%3").arg(ConfigWindow::getSetting<QString>("general/pony-directory"), path, animation_right));
        }else if( parent->behaviors.find(follow_stopped_behavior) == parent->behaviors.end()) {
            qCritical() << "Pony:"<<parent->name<<"follow stopped behavior:"<< follow_stopped_behavior << "from:"<< name << "not present.";
        }else{
            const Behavior &stopped_behavior = parent->behaviors.at(follow_stopped_behavior);
            if(stopped_behavior.animation_left == "") {
                qCritical() << "Pony:"<<parent->name<<"follow stopped behavior:"<< follow_moving_behavior << "animation left from:"<< name << "not present.";
            }else{
                animations[2] = new QMovie(QString("%1/%2/%3").arg(ConfigWindow::getSetting<QString>("general/pony-directory"), path, stopped_behavior.animation_left ));
                animations[3] = new QMovie(QString("%1/%2/%3").arg(ConfigWindow::getSetting<QString>("general/pony-directory"), path, stopped_behavior.animation_right ));
            }
        }
    }

    // Randomly select movement type from allowed types for this behavior
    if(movement_allowed != Movement::None && movement_allowed != Movement::MouseOver && movement_allowed != Movement::Sleep && movement_allowed != Movement::Dragged){
        std::vector<Behavior::Movement> modes;

        if(movement_allowed & Movement::Horizontal) modes.push_back(Movement::Horizontal);
        if(movement_allowed & Movement::Vertical)   modes.push_back(Movement::Vertical);
        if(movement_allowed & Movement::Diagonal)   modes.push_back(Movement::Diagonal);

        movement = modes[gen()%modes.size()];
    }

    // Randomly select horizontal and vertical direction
    direction_h = gen()%2 == 0? Left : Right;
    direction_v = gen()%2 == 0? Up : Down;

    // Set image center for current direction
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

    current_animation = animations[direction_h<0?0:1];
    current_animation->start();
    width = current_animation->currentImage().size().width();
    height = current_animation->currentImage().size().height();

    parent->update_animation(current_animation);

    // Move window due to change in image center
    parent->move(parent->x_pos-x_center,parent->y_pos-y_center);
}

void Behavior::deinit()
{
    if(current_animation != nullptr) {
        current_animation->stop();
    }

    for(int i = 0; i < 4; i++) {
        if(animations[i] != nullptr) {
            delete animations[i];
            animations[i] = nullptr;
        }
    }

    current_animation = nullptr;
}

void Behavior::change_direction(bool right, bool moving)
{
    current_animation->stop();

    uint8_t animation = right;
    if(state == State::Following || state == State::MovingToPoint){
        if(moving){
            animation = right; // animation 0 or 1 - follow moving behavior
        }else{
            animation = 2 + right; // animation 2 or 3 - follow stopped behavior
        }
    }

    current_animation = animations[animation];
    current_animation->start();
    parent->update_animation(current_animation);
    width = current_animation->currentImage().size().width();
    height = current_animation->currentImage().size().height();
    direction_h = right==true ? Direction::Right : Direction::Left;

    if(right) {
        x_center = right_image_center.x();
        y_center = right_image_center.y();
    }else{
        x_center = left_image_center.x();
        y_center = left_image_center.y();
    }

}

void Behavior::update()
{
    // No need to change position if we can't move
    if(movement == Movement::None) return;

    // Update pony position (so it won't jump when we change desktops or something else unexpected happens)
    // Under X11 the current desktop is (0,0)x(width,height). The desktop on the left is (-width,0)x(0,0),
    // the desktop to the right is (width,0)x(width*2,height), etc
    parent->x_pos = parent->x() + x_center;
    parent->y_pos = parent->y() + y_center;

    QRect screen = QApplication::desktop()->availableGeometry(parent);

    // If we are moving to a destanation point, calculate direction and move there
    if(state == State::Following  || state == State::MovingToPoint) {
        // Check if we are close enough to destanation point
        if((std::abs(destanation_point.x() - parent->x_pos) < 1.5f) && (std::abs(destanation_point.y() - parent->y_pos) < 1.5f)) {
            moving = false;
            change_direction(direction_h==Direction::Right,false);
            return; // We arrived at destanation, don't move anymore

            // TODO: if the centers for stopped and moving are not the same, then
            //       maybe we must move the window to the center of follow_stopped_behavior?
        }

        if(destanation_point.x() == 0 && destanation_point.y() == 0){
            qWarning() << parent->name << "behavior" << name << "is following, but has no target!";
            return;
        }


        float dir_x = destanation_point.x() - parent->x_pos;
        float dir_y = destanation_point.y() - parent->y_pos;

        // Normalize direction vector
        float vec_len = std::sqrt(dir_x*dir_x + dir_y*dir_y);
        dir_x /= vec_len;
        dir_y /= vec_len;

        // TODO: avoidance areas:
        // for each avoidance area:
        //  check if we are inside
        //   if yes, do not change direction, just go, we will leave it eventually
        //  check if we are too close
        //  abs(x - area.left) < min_dist // if we are too close, and we are
        //                                // going in the direction of the area (left,right,up,down)
        //                                // then flip direction
        //   change_direction left, etc


        // Check if we are facing the right irection
        if(dir_x < 0 && direction_h != Direction::Left) {
            moving = true;
            change_direction(false, true);
        }else if(dir_x > 0 && direction_h != Direction::Right) {
            moving = true;
            change_direction(true, true);
        }else if(moving == false) {
            // We were stopped, but are moving now, update the animation
            moving = true;
            change_direction(direction_h==Direction::Right,true);
        }

        // Move only if we are within the screen boundaries
        // Else we may go offscreen when two ponies are following each other
        if((parent->x() >= screen.left()) && (dir_x < 0)) {
            parent->x_pos += dir_x * speed;
        }
        if((parent->x() <= screen.right() - width) && (dir_x > 0)) {
            parent->x_pos += dir_x * speed;
        }

        if((parent->y() >= screen.top()) && (dir_y < 0)){
            parent->y_pos += dir_y * speed;
        }
        if((parent->y() <= screen.bottom() - height) && (dir_y > 0)){
            parent->y_pos += dir_y * speed;
        }

        parent->move(parent->x_pos-x_center,parent->y_pos-y_center);

        return;
    }

    // Normal movement

    // If we are at the screen edge or beyond then reverse the direction of movement if we are not already going in the right direction
    if((parent->x() <= screen.left()) && (direction_h != Direction::Right)) {
        change_direction(true);
        if(movement == Movement::Diagonal) choose_angle();
    }
    if((parent->x() >= screen.right() - width) && (direction_h != Direction::Left)) {
        change_direction(false);
        if(movement == Movement::Diagonal) choose_angle();
    }

    if((parent->y() <= screen.top()) && (direction_v != Direction::Down)){
        direction_v = Direction::Down;
        if(movement == Movement::Diagonal) choose_angle();
    }
    if((parent->y() >= screen.bottom() - height) && (direction_v != Direction::Up)){
        direction_v = Direction::Up;
        if(movement == Movement::Diagonal) choose_angle();
    }

    // Calculate the velocity
    float vel_x = direction_h * speed;
    float vel_y = direction_v * speed;

    // Update posiotion depending on movement type
    if(movement == Movement::Horizontal){
        parent->x_pos += vel_x;
    }
    if(movement == Movement::Vertical){
        parent->y_pos += vel_y;
    }
    if(movement == Movement::Diagonal){
        vel_x = std::sqrt(speed*speed*2) * std::cos(angle);
        vel_y = -std::sqrt(speed*speed*2) * std::sin(angle);
        parent->x_pos += vel_x;
        parent->y_pos += vel_y;
    }

    parent->move(parent->x_pos-x_center,parent->y_pos-y_center);

}

