#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H

#include <QMainWindow>
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

public slots:
    void remove_pony();
    void remove_pony_all();

private:
    Ui::ConfigWindow *ui;
};

#endif // CONFIGWINDOW_H
