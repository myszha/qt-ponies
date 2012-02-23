#ifndef PONY_H
#define PONY_H

#include <QtGui/QMovie>
#include <QtGui/QLabel>
#include <QMainWindow>

#include <string>
#include <unordered_map>
#include <random>

#include "behavior.h"
#include "effect.h"
#include "speak.h"

class Pony : public QMainWindow
{
    Q_OBJECT
public:
    explicit Pony(const std::string path, QWidget *parent = 0);
    ~Pony();

    void change_behavior();
    void update_animation(QMovie* movie);

    int x_center;
    int y_center;
    std::string name;
    std::unordered_map<std::string, Behavior> behaviors;
    std::vector<Behavior*> random_behaviors;
    Behavior* current_behavior;

signals:

public slots:
    void update();
    void display_menu(const QPoint &);

private:
    QLabel label;
    int64_t started;
    int64_t duration;
    std::random_device rd;
    std::mt19937 gen;
    float total_probability;

};

#endif // PONY_H
