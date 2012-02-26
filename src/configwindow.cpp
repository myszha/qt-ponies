#include <QDir>

#include <iostream>
#include <algorithm>

#include "configwindow.h"
#include "ui_configwindow.h"



ConfigWindow::ConfigWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ConfigWindow)
{
    signal_mapper = new QSignalMapper();

    ui->setupUi(this);

    // Setup tray icon and menu
    tray_icon.setIcon(QIcon(":/icons/res/tray_icon.png"));

    tray_menu.addAction("Open configuration",this,SLOT(show()));
    tray_menu.addAction("Close application",QCoreApplication::instance(),SLOT(quit()));

    connect(&tray_icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(toggle_window(QSystemTrayIcon::ActivationReason)));
    tray_icon.setContextMenu(&tray_menu);
    tray_icon.show();

    // Setup the toolbar buttons
    action_group = new QActionGroup(ui->toolBar);
    action_addponies = new QAction(QIcon(":/icons/res/add_icon.png"), "Add ponies", action_group);
    action_addponies->setCheckable(true);
    action_addponies->setChecked(true);
    action_activeponies = new QAction(QIcon(":/icons/res/active_icon.png"), "Active ponies", action_group);
    action_activeponies->setCheckable(true);
    action_configuration = new QAction(QIcon(":/icons/res/settings.png"), "Configuration", action_group);
    action_configuration->setCheckable(true);

    signal_mapper->setMapping(action_addponies,0);
    connect(action_addponies, SIGNAL(triggered()), signal_mapper, SLOT(map()));
    ui->toolBar->addAction(action_addponies);

    signal_mapper->setMapping(action_activeponies,1);
    connect(action_activeponies, SIGNAL(triggered()), signal_mapper, SLOT(map()));
    ui->toolBar->addAction(action_activeponies);

    signal_mapper->setMapping(action_configuration,2);
    connect(action_configuration, SIGNAL(triggered()), signal_mapper, SLOT(map()));
    ui->toolBar->addAction(action_configuration);

    ui->toolBar->setIconSize(QSize(100,100));

    connect(signal_mapper, SIGNAL(mapped(int)), ui->stackedWidget, SLOT(setCurrentIndex(int)));

    list_model = new QStandardItemModel(this);
    active_list_model = new QStandardItemModel(this);

    QDir dir("desktop-ponies");
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    // Get names of all the pony directories
    for(auto &i: dir.entryList()) {
        QDir pony_dir(dir);
        pony_dir.cd(i);
        if(pony_dir.exists("pony.ini")) {
            QStandardItem *item = new QStandardItem(QIcon(pony_dir.absoluteFilePath("icon.png")),i);
            list_model->appendRow(item);
        }
    }

    ui->listView->setIconSize(QSize(100,100));
    ui->listView->setModel(list_model);
    ui->listView->setAlternatingRowColors(true);

    ui->active_list->setIconSize(QSize(100,100));
    ui->active_list->setModel(active_list_model);
    ui->active_list->setAlternatingRowColors(true);

    connect(ui->listView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(newpony_list_changed(QModelIndex)));
    connect(ui->addpony_button, SIGNAL(clicked()), this, SLOT(add_pony()));
    connect(ui->removepony_button, SIGNAL(clicked()), this, SLOT(remove_pony_activelist()));

    // Start update timer
    timer.setInterval(30);
    timer.start();

    // Load settings
    settings = new QSettings("config.ini",QSettings::IniFormat);
    int size = settings->beginReadArray("loaded-ponies");
    for(int i=0; i< size; i++) {
        settings->setArrayIndex(i);
        ponies.emplace_back(std::make_shared<Pony>(settings->value("name").toString().toStdString(), this));
        QObject::connect(&timer, SIGNAL(timeout()), ponies.back().get(), SLOT(update()));
    }
    settings->endArray();
    list_model->sort(0);

    update_active_list();
}

ConfigWindow::~ConfigWindow()
{
    delete ui;
    delete signal_mapper;
    delete list_model;
    delete settings;
    delete action_group;
}

void ConfigWindow::remove_pony()
{
    // Get a pointer to Pony from sender()
    QAction *q = qobject_cast<QAction*>(QObject::sender());
    Pony* p = static_cast<Pony*>(q->parent()->parent()); // QAction->QMenu->QMainWindow(Pony)
    ponies.remove(p->get_shared_ptr());

    save_settings();
    update_active_list();
}

void ConfigWindow::remove_pony_all()
{
    // Get a pointer to Pony from sender()
    QAction *q = qobject_cast<QAction*>(QObject::sender());
    Pony* p = static_cast<Pony*>(q->parent()->parent()); // QAction->QMenu->QMainWindow(Pony)
    std::string pony_name(p->name); // We must copy the name, because it will be deleted
    ponies.remove_if([&pony_name](const std::shared_ptr<Pony> &pony){
        return pony->name == pony_name;
    });

    save_settings();
    update_active_list();
}

void ConfigWindow::remove_pony_activelist()
{
    // Check if we have selected an item
    if(ui->active_list->currentIndex().isValid() == false) return;

    // Get the name from active list
    std::string name = ui->active_list->currentIndex().data().toString().toStdString();

    // Find first occurance of pony name
    auto occurance = std::find_if(ponies.begin(), ponies.end(),
                                                             [&name](const std::shared_ptr<Pony> &p)
                                     {
                                         return p->directory == name;
                                     });
    // If found, remove
    if(occurance != ponies.end()) {
        ponies.erase(occurance);
        save_settings();
        update_active_list();
    }
}

void ConfigWindow::newpony_list_changed(QModelIndex item)
{
    // Update the UI with information about selected pony
    ui->image_label->setPixmap(item.data(Qt::DecorationRole).value<QIcon>().pixmap(100,100));
    ui->label_ponyname->setText(item.data().toString());
}

void ConfigWindow::add_pony()
{
    // Check if we have selected an item
    if(ui->listView->currentIndex().isValid() == false) return;

    // Add pony and save the active pony list to configuration file
    ponies.emplace_back(std::make_shared<Pony>(ui->listView->currentIndex().data().toString().toStdString(), this));
    QObject::connect(&timer, SIGNAL(timeout()), ponies.back().get(), SLOT(update()));

    save_settings();
    update_active_list();
}

void ConfigWindow::update_active_list()
{
    active_list_model->clear();
    for(auto &i: ponies) {
        QStandardItem *item = new QStandardItem(QIcon("desktop-ponies/" + QString::fromStdString(i->directory) +  "/icon.png"),QString::fromStdString(i->directory));
        active_list_model->appendRow(item);
    }
    active_list_model->sort(0);
}

void ConfigWindow::toggle_window(QSystemTrayIcon::ActivationReason reason)
{
    // Toogle the configuration window's visibility
    if(reason == QSystemTrayIcon::DoubleClick) {
        if(this->isVisible() == true)
        {
            hide();
        }else{
            show();
        }
    }
}

void ConfigWindow::save_settings()
{
    settings->clear();
    settings->beginWriteArray("loaded-ponies");
    int i=0;
    for(const auto &pony : ponies) {
        settings->setArrayIndex(i);
        settings->setValue("name", QString::fromStdString(pony->directory));
        i++;
    }
    settings->endArray();
}
