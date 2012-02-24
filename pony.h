#ifndef PONY_H
#define PONY_H

#include <QtGui/QMovie>
#include <QtGui/QLabel>
#include <QMainWindow>

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

    int x_center;
    int y_center;
    std::string name;
    std::unordered_map<std::string, Behavior> behaviors;
    std::vector<Behavior*> random_behaviors;
    Behavior* current_behavior;

    std::unordered_map<std::string, Speak> speak_lines;
    std::vector<Speak*> random_speak_lines;

signals:

public slots:
    void update();
    void display_menu(const QPoint &);

private:
    QLabel label;
    QLabel text_label;
    int64_t behavior_started;
    int64_t behavior_duration;
    int64_t speech_started;
    std::random_device rd;
    std::mt19937 gen;
    float total_behavior_probability;
    ConfigWindow *config;

};

#endif // PONY_H
