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

#ifndef BEHAVIOR_H
#define BEHAVIOR_H

#include <QMovie>

#include <cstdint>
#include <QString>

class Pony;

class Behavior
{
public:
    Behavior(Pony* parent, const QString filepath, const std::vector<QString> &options);
    Behavior(Behavior &&b);
    ~Behavior();

    void init();
    void deinit();
    void update();
    void info();

    enum Direction { Left = -1, Right = 1, Down = 1, Up = -1, Stand = 0};

    enum Movement {
        None 			= 0,
        Horizontal		= 1 << 1,
        Vertical 		= 1 << 2,
        Horizontal_Vertical 	= Horizontal | Vertical,
        Diagonal 		= 1 << 3,
        Diagonal_Horizontal 	= Diagonal | Horizontal,
        Diagonal_Vertical 	= Diagonal | Vertical,
        All 			= Horizontal | Vertical | Diagonal,
        MouseOver		= 1 << 4,
        Sleep			= 1 << 5,
        Dragged			= 1 << 6
    };

    enum class State {
        Normal,
        Following,
        MovingToPoint
    };

    float speed;
    int x_center;
    int y_center;
    State state;
    State type;
    QMovie* current_animation;
    int width;
    int height;
    uint8_t movement_allowed;
    float duration_min;
    float duration_max;
    float probability;
    QPoint destanation_point;
    QString path;
    QString animation_left;
    QString animation_right;
    QString linked_behavior;
    QString starting_line;
    QString ending_line;
    QString name;
    QString follow_stopped_behavior;
    QString follow_moving_behavior;
    bool skip_normally;
    int32_t x_coordinate;
    int32_t y_coordinate;
    QString follow_object;

    QPoint right_image_center;
    QPoint left_image_center;

    // We do not need follow_stopped centers
    QPoint follow_moving_right_image_center;
    QPoint follow_moving_left_image_center;

private:
    void choose_angle();
    void change_direction(bool right, bool moving = true);

    QMovie* animations[4]; /* 0 - left  / follow_moving left
                              1 - right / follow_moving right
                              2 - follow_stopped left
                              3 - follow_stopped right
                           */
    Pony* parent;
    int direction_h;
    int direction_v;
    int movement;
    bool moving;
    float angle;
};


#endif // BEHAVIOR_H
