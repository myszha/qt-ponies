#include <QDir>

#include <iostream>

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
    action_addponies = new QAction("Add ponies", action_group);
    action_addponies->setCheckable(true);
    action_addponies->setChecked(true);
    action_activeponies = new QAction("Active ponies", action_group);
    action_activeponies->setCheckable(true);
    action_configuration = new QAction("Configuration", action_group);
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

    connect(signal_mapper, SIGNAL(mapped(int)), ui->stackedWidget, SLOT(setCurrentIndex(int)));

    list_model = new QStandardItemModel(this);

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

    connect(ui->listView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(newpony_list_changed(QModelIndex)));
    connect(ui->addpony_button, SIGNAL(clicked()), this, SLOT(add_pony()));

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
}

void ConfigWindow::newpony_list_changed(QModelIndex item)
{
    // Update the UI with information about selected pony
    ui->image_label->setPixmap(item.data(Qt::DecorationRole).value<QIcon>().pixmap(100,100));
    ui->label_ponyname->setText(item.data().toString());
}

void ConfigWindow::add_pony()
{
    // Add pony and save the active pony list to configuration file
    ponies.emplace_back(std::make_shared<Pony>(ui->listView->currentIndex().data().toString().toStdString(), this));
    QObject::connect(&timer, SIGNAL(timeout()), ponies.back().get(), SLOT(update()));

    save_settings();
}

void ConfigWindow::update_active_list()
{
    //todo
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
    settings->clear(); // FIXME:: segfault
    settings->beginWriteArray("loaded-ponies");
    int i=0;
    for(const auto &pony : ponies) {
        settings->setArrayIndex(i);
        settings->setValue("name", QString::fromStdString(pony->directory));
        i++;
    }
    settings->endArray();
}
