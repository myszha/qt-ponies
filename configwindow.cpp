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
    tray_icon.setIcon(QIcon(":/tray_icon.png"));

    tray_menu.addAction("Open configuration",this,SLOT(show()));
    tray_menu.addAction("Close application",QCoreApplication::instance(),SLOT(quit()));

    tray_icon.setContextMenu(&tray_menu);
    tray_icon.show();

    signal_mapper->setMapping(ui->actionAdd_ponies,0);
    connect(ui->actionAdd_ponies, SIGNAL(triggered()), signal_mapper, SLOT(map()));

    signal_mapper->setMapping(ui->actionActive_ponies,1);
    connect(ui->actionActive_ponies, SIGNAL(triggered()), signal_mapper, SLOT(map()));

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

}

ConfigWindow::~ConfigWindow()
{
    delete ui;
    delete signal_mapper;
    delete list_model;
}

void ConfigWindow::remove_pony()
{
    // Get a pointer to Pony from sender()
    QAction *q = qobject_cast<QAction*>(QObject::sender());
    Pony* p = static_cast<Pony*>(q->parent()->parent()); // QAction->QMenu->QMainWindow(Pony)
    ponies.remove(p->get_shared_ptr());
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
}

void ConfigWindow::newpony_list_changed(QModelIndex item)
{
    ui->image_label->setPixmap(item.data(Qt::DecorationRole).value<QIcon>().pixmap(100,100));
    ui->label_ponyname->setText(item.data().toString());
}

void ConfigWindow::add_pony()
{
    ponies.emplace_back(std::make_shared<Pony>(ui->listView->currentIndex().data().toString().toStdString(), this));
    QObject::connect(&timer, SIGNAL(timeout()), ponies.back().get(), SLOT(update()));
}

void ConfigWindow::update_active_list()
{
    //todo
}
