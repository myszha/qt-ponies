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

    uint32_t speed;
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
