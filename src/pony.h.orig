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
#include <QHash>

#include <string>
#include <unordered_map>
#include <random>
#include <memory>
#include <vector>

#include "behavior.h"
#include "effect.h"
#include "speak.h"

class ConfigWindow;

namespace std
{
        template <>
        struct hash<QString>
        {
            size_t operator()(const QString& s) const
            {
                return qHash(s);
            }
        };
}

class Pony : public QMainWindow, public std::enable_shared_from_this<Pony>
{
    Q_OBJECT
public:
    explicit Pony(const QString path, ConfigWindow *config, QWidget *parent = 0);
    ~Pony();

    void change_behavior();
    void change_behavior_to(const QString &new_behavior);
    void update_animation(QMovie* movie);
    void set_on_top(bool top);
    void set_bypass_wm(bool bypass);
    std::shared_ptr<Pony> get_shared_ptr();

    float x_pos;
    float y_pos;
    Behavior* current_behavior;
    std::vector<Behavior*> random_behaviors;
    std::unordered_map<QString, Behavior> behaviors;

    std::unordered_map<QString, Effect> effects;

    std::unordered_map<QString, std::shared_ptr<Speak>> speak_lines;
    std::vector<Speak*> random_speak_lines;

    std::vector<Behavior*> sleep_behaviors;
    std::vector<Behavior*> drag_behaviors;

    std::vector<Behavior*> mouseover_behaviors;

    QString name;
    QString directory;

    bool sleeping;

    bool in_interaction;
    std::unordered_map<QString, int64_t> interaction_delays;
    QString current_interaction;
    int current_interaction_delay;

    std::mt19937 gen;

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
    void change_behavior_to(const std::vector<Behavior*> &new_behavior_list);
    void setup_current_behavior();

    QLabel label;
    QLabel text_label;
    Behavior *old_behavior;
    QString follow_object;
    int64_t behavior_started;
    int64_t behavior_duration;
    int64_t speech_started;
    float total_behavior_probability;
    ConfigWindow *config;
    QMenu* menu;
    bool dragging;
    bool mouseover;
    bool always_on_top;

};

inline std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) {
   return os << qPrintable(str);
}

#endif // PONY_H
