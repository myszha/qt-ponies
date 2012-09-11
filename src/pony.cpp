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

#include <QApplication>
#include <QDesktopWidget>
#include <QString>
#include <QDateTime>
#include <QMenu>
#include <QCheckBox>
#include <QWidgetAction>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include <fstream>
#include <random>
#include <algorithm>
#include <utility>

#include <cmath>

#include "csv_parser.h"
#include "configwindow.h"
#include "pony.h"

#ifdef Q_WS_X11
 #include <QX11Info>
 #include <X11/Xatom.h>
 #include <X11/Xlib.h> // Xlib #defines None as 0L, which conflicts with Behavior::Movement::None
                       // This is why we include it after pony.h
#endif

// TODO: Maybe a configuration option to change window shape?
// connect to current_animation: update() signal and do:
// setMask(current_behavior->current_animation->currentPixmap().mask());
// or maybe there are better ways to do it

// NOTE: QMovie in separate thread?

// FIXME: when ponies are not on top, they (all at once) flicker to top sometimes (on text show?)

Pony::Pony(const QString path, ConfigWindow *config, QWidget *parent) :
    QMainWindow(parent), sleeping(false), in_interaction(false), current_interaction_delay(0), gen(QDateTime::currentMSecsSinceEpoch()), label(this), config(config), dragging(false), mouseover(false)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_ShowWithoutActivating);

    // Disables shadows under the pony window.
    setAttribute(Qt::WA_X11NetWmWindowTypeDock);

#if defined QT_MAC_USE_COCOA && QT_VERSION >= 0x040800
    // Removes shadows that lag behind animation on OS X. QT 4.8+ needed.
    setAttribute(Qt::WA_MacNoShadow, true);
#endif

#ifdef QT_MAC_USE_COCOA
    // On OS X, tool windows are hidden when another program gains focus.
    Qt::WindowFlags windowflags = Qt::FramelessWindowHint;
#else
    Qt::WindowFlags windowflags = Qt::FramelessWindowHint | Qt::Tool;
#endif

    always_on_top = config->getSetting<bool>("general/always-on-top");
    if(always_on_top) {
        windowflags |= Qt::WindowStaysOnTopHint;
    }

#ifdef Q_WS_X11
    if(config->getSetting<bool>("general/bypass-wm")) {
        // Bypass the window manager
        windowflags |= Qt::X11BypassWindowManagerHint;
    }
#endif

    setWindowFlags( windowflags );

#ifdef Q_WS_X11
    // Qt on X11 does not support the skip taskbar/pager window flags, we have to set them ourselves
    // We let Qt initialize the other window properties, which aren't deleted when we replace them with ours
    // (they probably are appended on show())
    Atom window_state = XInternAtom( QX11Info::display(), "_NET_WM_STATE", False );
    Atom window_props[] = {
        XInternAtom( QX11Info::display(), "_NET_WM_STATE_SKIP_TASKBAR", False ),
        XInternAtom( QX11Info::display(), "_NET_WM_STATE_SKIP_PAGER"  , False )
    };

    XChangeProperty( QX11Info::display(), window()->winId(), window_state, XA_ATOM, 32, PropModeReplace, (unsigned char*)&window_props, 2 );
#endif

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(display_menu(const QPoint &)));

    // Setup speech label
    text_label.hide();
    text_label.setAttribute(Qt::WA_ShowWithoutActivating);
    // Disables showing of the toolbar button in some window managers.
    text_label.setAttribute(Qt::WA_X11NetWmWindowTypeDock);
    text_label.setWindowFlags(windowflags);
    text_label.setAlignment(Qt::AlignHCenter);
    text_label.setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
#ifdef Q_WS_X11
    // Same as above
    XChangeProperty( QX11Info::display(), text_label.window()->winId(), window_state, XA_ATOM, 32, PropModeReplace, (unsigned char*)&window_props, 2 );
#endif

    // Initially place the pony randomly on the screen, keeping a 50 pixel border
    x_pos = 50 + gen()%(QApplication::desktop()->availableGeometry(this).width()-100);
    y_pos = 50 + gen()%(QApplication::desktop()->availableGeometry(this).height()-100);

    move(x_pos, y_pos);

    directory = path;

    QFile ifile(QString("%1/%2/pony.ini").arg(ConfigWindow::getSetting<QString>("general/pony-directory"), path));
    if(!ifile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Cannot open pony.ini for pony:"<< path;
        qCritical() << ifile.errorString();
        throw std::exception();
    }

    name = path;

    if( ifile.isOpen() ) {
        QString line;
        QTextStream istr(&ifile);

        while (!istr.atEnd() ) {
            line = istr.readLine();

            if(line[0] != '\'' && !line.isEmpty()) {
                std::vector<QVariant> csv_data;
                CSVParser::ParseLine(csv_data, line, ',');

                // TODO: maybe add a try/catch here, in case of malformed pony.ini lines
                if(csv_data[0] == "Name") {
                    name = csv_data[1].toString(); //Name,"name"
                }
                else if(csv_data[0] == "Behavior") {
                    Behavior b(this, path, csv_data);
                    behaviors.insert({b.name, std::move(b)});
                }
                else if(csv_data[0] == "Effect") {
                    Effect e(this, path, csv_data);
                    effects.insert({e.name, std::move(e)});
                }
                else if(csv_data[0] == "Speak") {
                    std::shared_ptr<Speak> s = std::make_shared<Speak>(this, path, csv_data);
                    speak_lines.insert({s->name, std::move(s)});
                }
            }
        }

        ifile.close();
    }else{
        qCritical() << "Cannot read pony.ini for pony:"<< path;
        throw std::exception();
    }

    if(behaviors.size() == 0) {
        qCritical() << "Pony:"<<name<<"has no defined behaviors.";
        throw std::exception();
    }

    menu = new QMenu(this);
    QAction *sleep_action = new QAction(trUtf8("Sleeping"),menu);
    sleep_action->setCheckable(true);
    connect(sleep_action, SIGNAL(toggled(bool)), this, SLOT(toggle_sleep(bool)));

    menu->addAction(name)->setEnabled(false);
    menu->addSeparator();
    menu->addAction(sleep_action);
    menu->addAction(trUtf8("Remove %1").arg(name), config, SLOT(remove_pony()));
    menu->addAction(trUtf8("Remove every %1").arg(name), config, SLOT(remove_pony_all()));

    // Select behaviour that will can be choosen randomly
    for(auto &i: behaviors) {
        if(i.second.skip_normally == false) {
            random_behaviors.push_back(&i.second);
        }
    }

    if(random_behaviors.size() == 0) {
        qCritical() << "Pony:"<<name<<"has no defined behaviors that can be randomly selected.";
        throw std::exception();
    }


    std::sort(random_behaviors.begin(), random_behaviors.end(), [](const Behavior *val1, const Behavior *val2){ return val1->probability < val2->probability;} );
    total_behavior_probability = 0;
    for(auto &i: random_behaviors) {
        total_behavior_probability += i->probability;
    }

    // Select speech line that will be choosen randomly
    for(auto &i: speak_lines) {
        if(i.second->skip_normally == false) {
            random_speak_lines.push_back(i.second.get());
        }
    }

    // Select behaviors that will be used for sleeping
    for(auto &i: behaviors) {
        if(i.second.movement_allowed == Behavior::Movement::Sleep) {
           sleep_behaviors.push_back(&i.second);
        }
    }

    // Select behaviors that will be used for dragging
    for(auto &i: behaviors) {
        if(i.second.movement_allowed == Behavior::Movement::Dragged) {
           drag_behaviors.push_back(&i.second);
        }
    }

    // Select behaviors that will be used for mouseover
    for(auto &i: behaviors) {
        if(i.second.movement_allowed == Behavior::Movement::MouseOver) {
           mouseover_behaviors.push_back(&i.second);
        }
    }

    current_behavior = nullptr;
    change_behavior();
    this->show();

}

Pony::~Pony()
{
}

void Pony::set_bypass_wm(bool bypass)
{
    Qt::WindowFlags windowflags = windowFlags();

    if(bypass == true) {
        windowflags |= Qt::X11BypassWindowManagerHint;
    }else{
        windowflags ^= Qt::X11BypassWindowManagerHint;
    }

    setWindowFlags( windowflags );
    text_label.setWindowFlags(windowflags);

    // Set window properties for all effect instance windows
    for(auto &i: effects){
        for(auto &j: i.second.instances){
            j->setWindowFlags(windowflags);
        }
    }

    set_on_top(always_on_top);

}

void Pony::set_on_top(bool top)
{
    always_on_top = top;
    Qt::WindowFlags windowflags = windowFlags();
    if(top == true){
        windowflags |= Qt::WindowStaysOnTopHint; // Enable always on top
    }else{
        windowflags ^= Qt::WindowStaysOnTopHint; // Disable always on top
    }

    setWindowFlags(windowflags);
    text_label.setWindowFlags(windowflags);

#ifdef Q_WS_X11
        Atom window_state = XInternAtom( QX11Info::display(), "_NET_WM_STATE", False );
        Atom window_props[] = {
            XInternAtom( QX11Info::display(), "_NET_WM_STATE_SKIP_TASKBAR", False ),
            XInternAtom( QX11Info::display(), "_NET_WM_STATE_SKIP_PAGER"  , False ),
            XInternAtom( QX11Info::display(), "_NET_WM_STATE_ABOVE", False )
        };
        if(top == true){
            // Set the state to always on top, skip taskbar and pager
            XChangeProperty( QX11Info::display(), window()->winId(), window_state, XA_ATOM, 32, PropModeReplace, (unsigned char*)&window_props, 3 );
            XChangeProperty( QX11Info::display(), text_label.window()->winId(), window_state, XA_ATOM, 32, PropModeReplace, (unsigned char*)&window_props, 3 );

            // Set window properties for all effect instance windows
            for(auto &i: effects){
                for(auto &j: i.second.instances){
                    j->setWindowFlags(windowflags);
                    XChangeProperty( QX11Info::display(), j->window()->winId(), window_state, XA_ATOM, 32, PropModeReplace, (unsigned char*)&window_props, 3 );
                    j->show();
                }
            }

        }else{
            // Only set skip pager/taskbar
            XChangeProperty( QX11Info::display(), window()->winId(), window_state, XA_ATOM, 32, PropModeReplace, (unsigned char*)&window_props, 2 );
            XChangeProperty( QX11Info::display(), text_label.window()->winId(), window_state, XA_ATOM, 32, PropModeReplace, (unsigned char*)&window_props, 2 );

            // Set window properties for all effect instance windows
            for(auto &i: effects){
                for(auto &j: i.second.instances){
                    j->setWindowFlags(windowflags);
                    XChangeProperty( QX11Info::display(), j->window()->winId(), window_state, XA_ATOM, 32, PropModeReplace, (unsigned char*)&window_props, 2 );
                    j->show();
                }
            }
        }
#endif
    this->show(); // Refresh the window so the changes apply
    if(text_label.isVisible()){
        text_label.show();
    }
}


std::shared_ptr<Pony> Pony::get_shared_ptr()
{
    return shared_from_this();
}

void Pony::mouseMoveEvent(QMouseEvent* event)
{
    if (dragging) {
        x_pos = event->globalPos().x();
        y_pos = event->globalPos().y();
        QPoint new_pos(event->globalPos().x()-current_behavior->x_center,event->globalPos().y()-current_behavior->y_center);
        move(new_pos);
        event->accept();
    }
}

void Pony::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        dragging = true;
        change_behavior_to(drag_behaviors);
        event->accept();
    }
}

void Pony::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        dragging = false;
        if(mouseover == true){
            change_behavior_to(mouseover_behaviors);
        }else if(sleeping == true) {
            change_behavior_to(sleep_behaviors);
        }else if(!drag_behaviors.empty()){
            change_behavior();
        }
        event->accept();
    }
}

void Pony::enterEvent(QEvent* event)
{
    mouseover = true;
    change_behavior_to(mouseover_behaviors);

    event->accept();
}

void Pony::leaveEvent(QEvent* event)
{
    mouseover = false;
    if(sleeping == true) {
        change_behavior_to(sleep_behaviors);
    }else if(!mouseover_behaviors.empty()){
        change_behavior();
    }
    event->accept();
}

void Pony::toggle_sleep(bool is_asleep)
{
    sleeping = is_asleep;
    if(sleeping == true) {
        change_behavior_to(sleep_behaviors);
    }else{
        change_behavior();
    }
}


void Pony::display_menu(const QPoint &pos)
{
    menu->exec(mapToGlobal(pos));
}

void Pony::update_animation(QMovie* animation)
{
    // FIXME: sometimes animation gets stuck
    // happens on mouse leave
    // behaviors change?, but animation is stopped, even when bouncing off the edge
    // it is updating, because it moves
    // it seems the label does not change to the new animation maybe?

    label.setMovie(animation);
    resize(animation->currentImage().size());
    label.resize(animation->currentImage().size());
    label.repaint();
}

// Change behavior to the specified one
void Pony::change_behavior_to(const QString &new_behavior)
{
    if(behaviors.find(new_behavior) == behaviors.end()) {
        qCritical() << "Pony:"<<name<<"behavior:"<< new_behavior << "does not exist.";
        return;
    }

    old_behavior = current_behavior;

    if(current_behavior != nullptr) {
        current_behavior->deinit();
    }

    current_behavior = &behaviors.at(new_behavior);

    setup_current_behavior();
}

// Change behavior to one randomly selected from supplied list
// Used for changing to dragged/mouseover/sleeping
void Pony::change_behavior_to(const std::vector<Behavior*> &new_behavior)
{
    int size = new_behavior.size();
    if(size > 0){
            in_interaction = false; // We interrupted an interaction if there was one, so stop it
            interaction_delays[current_interaction] = QDateTime::currentMSecsSinceEpoch() + current_interaction_delay;

            std::uniform_int_distribution<> dis(0, new_behavior.size()-1);
            current_behavior->deinit();
            current_behavior = new_behavior.at(dis(gen));
            current_behavior->init();

            if(config->getSetting<bool>("general/debug")) {
                    qDebug() << "Pony:"<<name<<"behavior: "<< current_behavior->name;
            }

    }
}

// Randomly change behavior or follow linked one
void Pony::change_behavior()
{
    old_behavior = current_behavior;

    if(current_behavior != nullptr) {
        current_behavior->deinit();
    }

    follow_object = "";

    // Check if linked behavior is present
    if(current_behavior != nullptr && current_behavior->linked_behavior != "") {
        if( behaviors.find(current_behavior->linked_behavior) == behaviors.end()) {
            qCritical() << "Pony:"<<name<<"linked behavior:"<< current_behavior->linked_behavior<< "from:"<< current_behavior->name << "not present.";
            // TODO: current_behavior = nullptr and change_behavior(), so we can do another behavior if this is not found
        }else{
            current_behavior = &behaviors.at(current_behavior->linked_behavior);
        }
    }else{
        // If linked behavior not present, select random behavior using roulette-wheel selection

        in_interaction = false; // We finished the interaction if there was one
        interaction_delays[current_interaction] = QDateTime::currentMSecsSinceEpoch() + current_interaction_delay;

        float total = 0;
        std::uniform_real_distribution<> dis(0, total_behavior_probability);
        float rnd = dis(gen);
        for(auto &i: random_behaviors){
            total += i->probability;
            if(rnd <= total) {
                current_behavior = i;
                break;
            }
        }

    }

    setup_current_behavior();

}

// Initialize current behavior
void Pony::setup_current_behavior()
{
    if(current_behavior->type == Behavior::State::Following || current_behavior->type == Behavior::State::MovingToPoint) {
        if(current_behavior->type == Behavior::State::Following){
            // Find follow_object (which is not empty, because we checked it while initializing)
            auto found = std::find_if(config->ponies.begin(), config->ponies.end(),
                                          [this](const std::shared_ptr<Pony> &p) {
                                              return p->name.toLower() == current_behavior->follow_object.toLower();
                                          });
            if(found != config->ponies.end()){
                follow_object = (*found)->name;
                // Destanation point = follow object position + x/y_coordinate offset
                current_behavior->state = Behavior::State::Following;
                current_behavior->destanation_point = QPoint((*found)->x_pos + current_behavior->x_coordinate, (*found)->y_pos + current_behavior->y_coordinate);
            }else{
                // If we did not find the targeted pony in active pony list, then set this behavior to normal for the time being
                follow_object = "";
                current_behavior->state = Behavior::State::Normal;
            }
        }

        if(current_behavior->type == Behavior::State::MovingToPoint) {
            current_behavior->destanation_point = QPoint(((float)current_behavior->x_coordinate / 100.0f) * QApplication::desktop()->availableGeometry(this).width(),
                                                        ((float)current_behavior->y_coordinate / 100.0f) * QApplication::desktop()->availableGeometry(this).height());
        }
    }

    int64_t dur_min = std::round(current_behavior->duration_min*1000);
    int64_t dur_max = std::round(current_behavior->duration_max*1000);
    if(dur_min == dur_max) {
        behavior_duration = dur_min;
    }else{
        std::uniform_int_distribution<> dis(dur_min, dur_max);
        behavior_duration = dis(gen);
    }


    if(config->getSetting<bool>("general/debug")) {
            qDebug() << "Pony:"<<name<<"behavior:"<< current_behavior->name <<"for" << behavior_duration << "msec";
    }

    // Update pony position (so it won't jump when we change desktops or something else unexpected happens)
    if(old_behavior != nullptr){
        x_pos = x() + old_behavior->x_center;
        y_pos = y() + old_behavior->y_center;
    }
    behavior_started = QDateTime::currentMSecsSinceEpoch();
    current_behavior->init();

    // Select speech line to display:
    // starting_line for current behavior or random
    // Do not choose random line when following a linked behavior,
    //    instead use the ending_line of the previous behavior if current
    //    behavior does not have a starting line
    // If ending_line is present, use that instead of choosing a new one
    if(speak_lines.size() > 0 && config->getSetting<bool>("speech/enabled")) {
        Speak* current_speech_line = nullptr;

        if(current_behavior->starting_line != ""){
            // If we have a starting_line, use that

            if( speak_lines.find(current_behavior->starting_line) == speak_lines.end()) {
                qWarning() << "Pony:"<<name<<"starting line:"<< current_behavior->starting_line<< "from:"<< current_behavior->name << "not present.";
            }else{
                current_speech_line = speak_lines.at(current_behavior->starting_line).get();
            }            
        }else if(old_behavior != nullptr && old_behavior->ending_line != "" && old_behavior->linked_behavior != current_behavior->name){
            // If we do not have a starting line, and this is a linked behavior, use old behavior's ending line if present
            // old_behavior == nullptr only if we didn't have any previous behaviors (i.e. at startup)

            if( speak_lines.find(old_behavior->ending_line) == speak_lines.end()) {
                qWarning() << "Pony:"<<name<<"ending line:"<< old_behavior->ending_line<< "from:"<< old_behavior->name << "not present.";
            }else{
                current_speech_line = speak_lines.at(old_behavior->ending_line).get();
            }
        }else if(!current_behavior->ending_line.isEmpty() || in_interaction || current_behavior->state == Behavior::State::Following){
            // Don not choose a random line if we have an ending one, or we are in an interaction, or we are following
            return;
        }else if(old_behavior == nullptr || old_behavior->linked_behavior != current_behavior->name) {
            // If we do not have a starting line and this is NOT a linked behavior, then choose one randomly
            // old_behavior == nullptr only if we didn't have any previous behaviors (i.e. at startup)

            std::uniform_real_distribution<> real_dis(0, 100);
            // Speak only with the specified probability
            if((random_speak_lines.size()) > 0 && (real_dis(gen) <= config->getSetting<float>("speech/probability"))) {
                std::uniform_int_distribution<> int_dis(0, random_speak_lines.size()-1);
                current_speech_line = random_speak_lines[int_dis(gen)];
            }
        }

        if(current_speech_line != nullptr) {
            // Show text only if we found a suitable line

            text_label.setText(current_speech_line->text);
            speech_started = behavior_started;
            text_label.adjustSize();
            text_label.move(x_pos-text_label.width()/2, y() - text_label.height());
            text_label.show();
            if(config->getSetting<bool>("sound/enabled")) {
                current_speech_line->play();
            }
        }
    }
}

void Pony::update() {
    int64_t time = QDateTime::currentMSecsSinceEpoch();

    // Check for speech timeout and move text with pony
    if(text_label.isVisible() == true) {
        if(speech_started + config->getSetting<int>("speech/duration") <= time) {
            text_label.hide();
        }else{
            text_label.move(x() + current_behavior->x_center - text_label.width()/2, y() - text_label.height());
        }
    }

    // Check behavior timeout and update if not dragging or asleep
    if(!dragging && !sleeping && !mouseover) {
        if(behavior_started+behavior_duration <= time){
            change_behavior();
        }

        // If we are following anypony, update their position
        if(follow_object != "" && current_behavior->state == Behavior::State::Following){
            auto found = std::find_if(config->ponies.begin(), config->ponies.end(),
                                          [this](const std::shared_ptr<Pony> &p) {
                                              return p->name == follow_object;
                                          });
            if(found != config->ponies.end()){
                current_behavior->destanation_point = QPoint((*found)->x_pos + current_behavior->x_coordinate, (*found)->y_pos + current_behavior->y_coordinate);
            }else{
                // The pony we were following is no longer available
                change_behavior();
            }
        }
        current_behavior->update();
    }

    for(auto &i: effects){
        i.second.update();
    }
}
