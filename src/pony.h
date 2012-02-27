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

#ifndef PONY_H
#define PONY_H

#include <QtGui/QMovie>
#include <QtGui/QLabel>
#include <QMainWindow>
#include <QMouseEvent>

#include <string>
#include <unordered_map>
#include <random>
#include <memory>

#include "behavior.h"
#include "effect.h"
#include "speak.h"

class ConfigWindow;

class Pony : public QMainWindow, public std::enable_shared_from_this<Pony>
{
    Q_OBJECT
public:
    explicit Pony(const std::string path, ConfigWindow *config, QWidget *parent = 0);
    ~Pony();

    void change_behavior();
    void update_animation(QMovie* movie);
    std::shared_ptr<Pony> get_shared_ptr();

    float x_center;
    float y_center;
    Behavior* current_behavior;
    std::vector<Behavior*> random_behaviors;
    std::unordered_map<std::string, Behavior> behaviors;

    std::unordered_map<std::string, Speak> speak_lines;
    std::vector<Speak*> random_speak_lines;

    std::vector<Behavior*> sleep_behaviors;
    std::vector<Behavior*> drag_behaviors;

    std::vector<Behavior*> mouseover_behaviors;

    std::string name;
    std::string directory;

signals:

public slots:
    void update();
    void display_menu(const QPoint &);    
    void toggle_sleep(bool is_asleep);

protected:
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);

private:
    std::mt19937 gen;
    QLabel label;
    QLabel text_label;
    int64_t behavior_started;
    int64_t behavior_duration;
    int64_t speech_started;
    float total_behavior_probability;
    ConfigWindow *config;
    QMenu* menu;
    bool dragging;
    bool sleeping;
    bool mouseover;

};

#endif // PONY_H
