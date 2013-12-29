/*
 * Qt-ponies - ponies on the desktop
 * Copyright (C) 2013 funeralismatic, XRevan86
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

#ifndef EFFECT_H
#define EFFECT_H

#include <QtGui/QMovie>
#include <QtGui/QLabel>
#include <QMainWindow>
#include <QVariant>
#include <QMovie>
#include <QString>
#include <QPoint>

#include <list>
#include <string>
#include <memory>

#include "csv_parser.h"

class Pony;
class ConfigWindow;
class EffectInstance;

class Effect
{
public:
    explicit Effect(Pony* parent, ConfigWindow* conf, const QString filepath, const std::vector<QVariant> &options);
    ~Effect();

    enum Position {
        Top_Right       = 0,
        Top_Left        = 1,
        Bottom_Right    = 2,
        Bottom_Left     = 3,
        Top             = 4,
        Bottom          = 5,
        Left            = 6,
        Right           = 7,
        Center          = 8,
        Any             = 9,
        Any_NotCenter   = 10,
        Last            = 11
    };

    void update();
    void start();
    void stop();
    void change_direction(bool right);

    static const CSVParser::ParseTypes OptionTypes;

    std::list<std::shared_ptr<EffectInstance>> instances;

    QString name;
    QString behavior;

private:
    void new_instance();

    float duration;
    float repeat_delay;
    int64_t last_instanced;
    bool running;

    QString image_left;
    QString image_right;
    QString path;
    Position location_right;
    Position location_left;
    Position center_right;
    Position center_left;
    bool follow;

    Pony* parent_pony;
    ConfigWindow* config;

    friend class EffectInstance;

};

class EffectInstance: public QMainWindow
{
    Q_OBJECT
public:
    explicit EffectInstance(Effect* owner, int64_t started, bool right, QWidget *parent = 0);
    ~EffectInstance();

    void change_direction(bool right);
    void update_animation();

    int64_t time_started;
    QPoint offset;

private:
    QPoint get_location(int location, int centering);

    QMovie* animation_left;
    QMovie* animation_right;
    QMovie* current_animation;

    int image_width;
    int image_height;

    QLabel label;
    Effect* owner;
};


#endif // Effect_H
