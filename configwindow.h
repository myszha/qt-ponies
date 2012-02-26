#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H

#include <QMainWindow>
#include <QSignalMapper>
#include <QStandardItemModel>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QMenu>

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

private:
    Ui::ConfigWindow *ui;
    QSignalMapper *signal_mapper;
    QStandardItemModel *list_model;
    QSystemTrayIcon tray_icon;
    QMenu tray_menu;

};

#endif // CONFIGWINDOW_H
