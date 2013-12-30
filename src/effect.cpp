/*
 * Qt-ponies - ponies on the desktop
 * Copyright (C) 2013 funeralismatic, XRevan86
 * Copyright (C) 2012-2013 mysha
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

#include <QDateTime>
#include <QDebug>

#include "configwindow.h"
#include "effect.h"
#include "pony.h"

// These are the variable types for Effect configuration
const CSVParser::ParseTypes Effect::OptionTypes {
   {                     "type", QVariant::Type::String },
   {                     "name", QVariant::Type::String },
   {                 "behavior", QVariant::Type::String },
   {              "image_right", QVariant::Type::String },
   {               "image_left", QVariant::Type::String },
   {                 "duration", QVariant::Type::Double },
   {             "repeat_delay", QVariant::Type::Double },
   {           "location_right", QVariant::Type::String },
   {             "center_right", QVariant::Type::String },
   {            "location_left", QVariant::Type::String },
   {              "center_left", QVariant::Type::String },
   {                   "follow", QVariant::Type::Bool   }
};

// Something in Xlib.h makes the above initialization have a syntax error if included before it.
#ifdef Q_WS_X11
 #include <QX11Info>
 #include <X11/Xatom.h>
 #include <X11/Xlib.h>
 #include <X11/extensions/Xfixes.h>
 #include <X11/extensions/shapeconst.h>
#endif

static const std::unordered_map<std::string, Effect::Position> position_map = {
    {"top",             Effect::Position::Top           },
    {"bottom",          Effect::Position::Bottom        },
    {"left",            Effect::Position::Left          },
    {"right",           Effect::Position::Right         },
    {"bottom_right",    Effect::Position::Bottom_Right  },
    {"bottom_left",     Effect::Position::Bottom_Left   },
    {"top_right",       Effect::Position::Top_Right     },
    {"top_left",        Effect::Position::Top_Left      },
    {"center",          Effect::Position::Center        },
    {"any",             Effect::Position::Any           },
    {"any-not_center",  Effect::Position::Any_NotCenter }
};

EffectInstance::EffectInstance(Effect *owner, int64_t started, bool right, QWidget *parent)
    :QMainWindow(parent), time_started(started), label(this), owner(owner)
{
    // Set window properties the same as the pony window
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_ShowWithoutActivating);

#ifdef Q_WS_X11
    // On some window managers dock windows show on all desktops. KWin shows unwanted shadows when using another type of window.
    if(owner->config->getX11_WM() == ConfigWindow::X11_WM_Types::KWin) {
        setAttribute(Qt::WA_X11NetWmWindowTypeDock);
    }
#endif

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

    if(ConfigWindow::getSetting<bool>("general/always-on-top")) {
        windowflags |= Qt::WindowStaysOnTopHint;
    }

#ifdef Q_WS_X11
    if(ConfigWindow::getSetting<bool>("general/bypass-wm")) {
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

    // Set a null input region mask for the event window, so that it does not interfere with mouseover effects.
    XRectangle rect{0,0,0,0};
    XserverRegion shapeRegion = XFixesCreateRegion(QX11Info::display(), &rect, 1);
    XFixesSetWindowShapeRegion(QX11Info::display(), winId(), ShapeInput, 0, 0, shapeRegion);
    XFixesDestroyRegion(QX11Info::display(), shapeRegion);
#endif
    // TODO: add WS_EX_TRANSPARENT extended window style on windows.

#ifdef Q_WS_X11
    // Make sure the effect gets drawn on the same desktop as the pony
    Atom wm_desktop = XInternAtom(QX11Info::display(), "_NET_WM_DESKTOP", False);
    Atom type_ret;
    int fmt_ret;
    unsigned long nitems_ret;
    unsigned long bytes_after_ret;
    int *desktop = NULL;

    if(XGetWindowProperty(QX11Info::display(), owner->parent_pony->window()->winId(), wm_desktop, 0, 1,
                          False, XA_CARDINAL, &type_ret, &fmt_ret,
                          &nitems_ret, &bytes_after_ret, reinterpret_cast<unsigned char **>(&desktop))
       == Success && desktop != NULL) {
       XChangeProperty(QX11Info::display(), window()->winId(), wm_desktop, XA_CARDINAL, 32, PropModeReplace,
                       reinterpret_cast<unsigned char*>(desktop), 1);
       XFree(desktop);
    }
#endif

    // Load animations and verify them
    // TODO: Do we need to change the direction of active effects? Maybe we only need to display the image for the direction at witch it was spawned.
    animation_left = new QMovie(QString("%1/%2/%3").arg(ConfigWindow::getSetting<QString>("general/pony-directory"), owner->path, owner->image_left ));
    animation_right = new QMovie(QString("%1/%2/%3").arg(ConfigWindow::getSetting<QString>("general/pony-directory"), owner->path, owner->image_right));

    if(!animation_left->isValid())
        qCritical() << "Effect:"<< owner->path <<"Error opening left animation:"<< owner->image_left << "for effect:"<< owner->name;
    if(!animation_right->isValid())
        qCritical() << "Effect:"<< owner->path <<"Error opening right animation:"<< owner->image_right << "for behavior:"<< owner->name;

    animation_left->setCacheMode(QMovie::CacheAll);
    animation_right->setCacheMode(QMovie::CacheAll);

    if(right){
        current_animation = animation_right;
    }else{
        current_animation = animation_left;
    }

    current_animation->jumpToFrame(0);
    current_animation->jumpToNextFrame();

    image_width = current_animation->currentImage().width();
    image_height = current_animation->currentImage().height();

    if(right){
        offset = get_location(owner->location_right, owner->center_right);
    }else{
        offset = get_location(owner->location_left, owner->center_left);
    }

    if(ConfigWindow::getSetting<bool>("general/small-ponies")){
        image_width /= 2.0;
        image_height /= 2.0;
        current_animation->setScaledSize(current_animation->currentImage().size() / 2.0);
    } else {
        current_animation->setScaledSize(current_animation->currentImage().size() / 2.0);
    }
    current_animation->start();
    update_animation();
    show();
}

EffectInstance::~EffectInstance()
{
    delete animation_left;
    delete animation_right;
}

void EffectInstance::change_direction(bool right)
{
    if(owner->follow == false) return;

    if(current_animation != nullptr){
        current_animation->stop();
    }

    if(right){
        current_animation = animation_right;
    }else{
        current_animation = animation_left;
    }

    current_animation->jumpToFrame(0);
    current_animation->jumpToNextFrame();

    image_width = current_animation->currentImage().width();
    image_height = current_animation->currentImage().height();

    if(right){
        offset = get_location(owner->location_right, owner->center_right);
    }else{
        offset = get_location(owner->location_left, owner->center_left);
    }

    if(ConfigWindow::getSetting<bool>("general/small-ponies")){
        image_width /= 2.0;
        image_height /= 2.0;
        current_animation->setScaledSize(current_animation->currentImage().size() / 2.0);
    } else {
        current_animation->setScaledSize(current_animation->currentImage().size() / 2.0);
    }
    current_animation->start();
    update_animation();
}

void EffectInstance::update_animation()
{
    label.setMovie(current_animation);
    resize(current_animation->currentImage().size());
    label.resize(current_animation->currentImage().size());
    label.repaint();
}

QPoint EffectInstance::get_location(int location, int centering)
{
    QPoint l;
    QPoint c;

    if(location == Effect::Position::Any){
        std::uniform_int_distribution<> int_dis(0, Effect::Position::Last - 2);
        location = int_dis(owner->parent_pony->gen);
    }else if(location == Effect::Position::Any_NotCenter){
        std::uniform_int_distribution<> int_dis(0, Effect::Position::Last - 3);
        location = int_dis(owner->parent_pony->gen);
    }

    if(centering == Effect::Position::Any){
        std::uniform_int_distribution<> int_dis(0, Effect::Position::Last - 2);
        centering = int_dis(owner->parent_pony->gen);
    }else if(centering == Effect::Position::Any_NotCenter){
        std::uniform_int_distribution<> int_dis(0, Effect::Position::Last - 3);
        centering = int_dis(owner->parent_pony->gen);
    }

    switch(location){
        case Effect::Position::Top_Right: {
            l.setY(0);
            l.setX(owner->parent_pony->width());
            break;
        }
        case Effect::Position::Top_Left: {
            l.setY(0);
            l.setX(0);
            break;
        }
        case Effect::Position::Bottom_Right: {
            l.setY(owner->parent_pony->height());
            l.setX(owner->parent_pony->width());
            break;
        }
        case Effect::Position::Bottom_Left: {
            l.setY(owner->parent_pony->height());
            l.setX(0);
            break;
        }
        case Effect::Position::Top: {
            l.setY(0);
            l.setX(owner->parent_pony->width()/2);
            break;
        }
        case Effect::Position::Bottom: {
            l.setY(owner->parent_pony->height());
            l.setX(owner->parent_pony->width()/2);
            break;
        }
        case Effect::Position::Left: {
            l.setY(owner->parent_pony->height()/2);
            l.setX(0);
            break;
        }
        case Effect::Position::Right: {
            l.setY(owner->parent_pony->height()/2);
            l.setX(owner->parent_pony->width());
            break;
        }
        case Effect::Position::Center: {
            l.setY(owner->parent_pony->height()/2);
            l.setX(owner->parent_pony->width()/2);
            break;
        }
    }

    switch(centering){
        case Effect::Position::Top_Right: {
            c.setY(0);
            c.setX(image_width);
            break;
        }
        case Effect::Position::Top_Left: {
            c.setY(0);
            c.setX(0);
            break;
        }
        case Effect::Position::Bottom_Right: {
            c.setY(image_height);
            c.setX(image_width);
            break;
        }
        case Effect::Position::Bottom_Left: {
            c.setY(image_height);
            c.setX(0);
            break;
        }
        case Effect::Position::Top: {
            c.setY(0);
            c.setX(image_width/2);
            break;
        }
        case Effect::Position::Bottom: {
            c.setY(image_height);
            c.setX(image_width/2);
            break;
        }
        case Effect::Position::Left: {
            c.setY(image_height/2);
            c.setX(0);
            break;
        }
        case Effect::Position::Right: {
            c.setY(image_height/2);
            c.setX(image_width);
            break;
        }
        case Effect::Position::Center: {
            c.setY(image_height/2);
            c.setX(image_width/2);
            break;
        }
    }

    return l - c;
}

Effect::Effect(Pony *parent, ConfigWindow* conf, const QString filepath, const std::vector<QVariant> &options)
    : path(filepath), parent_pony(parent), config(conf)
{
    // TODO: fail not catastrophically
    Q_ASSERT(options.size() == 12);

    name = options[1].toString().toLower();
    behavior = options[2].toString().toLower();
    image_right = options[3].toString().toLower();
    image_left = options[4].toString().toLower();

    duration = options[5].toFloat();
    repeat_delay = options[6].toFloat();

    location_right = position_map.at(options[7].toString().toLower().toStdString());
    center_right = position_map.at(options[8].toString().toLower().toStdString());
    location_left = position_map.at(options[9].toString().toLower().toStdString());
    center_left = position_map.at(options[10].toString().toLower().toStdString());
    follow = options[11].toBool();
}

Effect::~Effect()
{
}

void Effect::update()
{
    if(!running) return;

    // Check if we need to spawn another instance
    if((repeat_delay != 0) && (last_instanced + repeat_delay*1000.0 < QDateTime::currentMSecsSinceEpoch())){
        // repeat_delay = 0 means we spawn only one instance
        new_instance();
    }

    // Delete instances that lasted their full duration
    if(duration != 0){ // Duration = 0 means the effect stays there until its stoped
        for(auto i = instances.begin(); i != instances.end(); ++i){
            if(((*i)->time_started + (int64_t)(duration*1000)) < QDateTime::currentMSecsSinceEpoch()){
                i = instances.erase(i);
            }
        }
    }

    // Update instance positions
    if(follow){
        for(auto &i: instances){
            i->move(parent_pony->pos() + i->offset);
        }
    }
}

void Effect::start()
{
    running = true;

    // Add the first effect instance
    new_instance();

    if(ConfigWindow::getSetting<bool>("general/debug")) {
        qDebug() << "Pony:"<<parent_pony->name<<"effect:"<< name <<"started.";
    }

}

void Effect::stop()
{
    running = false;
    instances.clear();

    if(ConfigWindow::getSetting<bool>("general/debug")) {
        qDebug() << "Pony:"<<parent_pony->name<<"effect:"<< name <<"stoped.";
    }

}

void Effect::change_direction(bool right)
{
    if(!running) return;

    for(auto &i: instances){
        i->change_direction(right);
    }
}

void Effect::new_instance()
{
    instances.push_back(std::make_shared<EffectInstance>(this, QDateTime::currentMSecsSinceEpoch(), parent_pony->current_behavior->direction_h == Behavior::Direction::Right, parent_pony));
    last_instanced = QDateTime::currentMSecsSinceEpoch();

    std::shared_ptr<EffectInstance> &i = instances.back();

    // Move the newly added effect instance to the appropriate position
    i->move(parent_pony->pos() + i->offset);
}
