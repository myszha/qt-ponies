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
#include <string>

class Pony;

class Behavior
{
public:
    Behavior(Pony* parent, const std::string filepath, const std::vector<std::string> &options);
    Behavior(Behavior &&b) noexcept;
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

    float speed;
    int width;
    int height;
    QMovie* current_animation;
    uint8_t movement_allowed;
    int x_center;
    int y_center;
    float duration_min;
    float duration_max;
    float probability;
    std::string path;
    std::string animation_left;
    std::string animation_right;
    std::string linked_behavior;
    std::string starting_line;
    std::string ending_line;
    std::string name;
    bool skip_normally;
    uint32_t x_coordinate;
    uint32_t y_coordinate;
    std::string follow_object;

    QPoint right_image_center;
    QPoint left_image_center;

private:
    void choose_angle();

    QMovie* animations[2];
    Pony* parent;
    int direction_h;
    int direction_v;
    int movement;
    float angle;

    int desktop_width;
    int desktop_height;
};


#endif // BEHAVIOR_H
