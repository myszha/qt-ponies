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

#include "pony.h"

namespace Ui {
    class ConfigWindow;
}

class ConfigWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ConfigWindow(QWidget *parent = 0);
    ~ConfigWindow();


    std::list<std::shared_ptr<Pony>> ponies;
    QTimer timer;

public slots:
    void remove_pony();
    void remove_pony_all();
    void newpony_list_changed(QModelIndex item);
    void add_pony();
    void update_active_list();
    void toggle_window(QSystemTrayIcon::ActivationReason reason);

private:
    void save_settings();

    Ui::ConfigWindow *ui;
    QSignalMapper *signal_mapper;
    QStandardItemModel *list_model;
    QSystemTrayIcon tray_icon;
    QMenu tray_menu;
    QSettings *settings;
    QActionGroup *action_group;
    QAction *action_addponies;
    QAction *action_activeponies;
    QAction *action_configuration;

};

#endif // CONFIGWINDOW_H
