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

#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H

#include <QMainWindow>
#include <QSignalMapper>
#include <QStandardItemModel>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QSettings>
#include <QAction>
#include <QActionGroup>

#include <memory>
#include <vector>
#include <iostream>
#include <unordered_map>

#include "pony.h"
#include "interaction.h"

namespace Ui {
    class ConfigWindow;
}

namespace std {
    template <>
    struct hash<pair<QString, QString> >
    {
        size_t operator()(const pair<QString, QString> &p) const
        {
            // Maybe change it so it gives the same hash for <p1,p2> and <p2,p1>
            return qHash(QPair<QString, QString>(p.first, p.second));
        }
    };
}

class ConfigWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ConfigWindow(QWidget *parent = 0);
    ~ConfigWindow();


    std::list<std::shared_ptr<Pony>> ponies;
    QTimer update_timer;
    QTimer interaction_timer;

    static const std::unordered_map<QString, const QVariant> config_defaults;

    template <typename T>
    static T getSetting(const QString& name, const QSettings &settings = QSettings()) {
        QString key(name);
        if(settings.group() != "") {
            // We currently are in a group - append the group name to the key
            key = QString("%1/%2").arg(settings.group(), name);
        }
        if(config_defaults.find(key) != config_defaults.end()){
            // There is a default for that option in config_defaults, use it
            return settings.value(name, config_defaults.at(key)).value<T>();
        }else{
            // No default, use empty QVariant
            return settings.value(name).value<T>();
        }
    }


public slots:
    void remove_pony();
    void remove_pony_all();

private slots:
    void remove_pony_activelist();
    void newpony_list_changed(QModelIndex item);
    void add_pony();
    void update_active_list();
    void toggle_window(QSystemTrayIcon::ActivationReason reason);
    void save_settings();
    void load_settings();
    void lettertab_changed(int index);
    void change_ponydata_directory();
    void update_interactions();

private:
    void reload_available_ponies();
    void update_distances();

    std::vector<Interaction> interactions;

    std::unordered_map<std::pair<QString, QString>, float> distances;

    Ui::ConfigWindow *ui;
    QSignalMapper *signal_mapper;
    QStandardItemModel *list_model;
    QStandardItemModel *active_list_model;
    QSystemTrayIcon tray_icon;
    QMenu tray_menu;
    QActionGroup *action_group;
    QAction *action_addponies;
    QAction *action_activeponies;
    QAction *action_configuration;

};

#endif // CONFIGWINDOW_H
