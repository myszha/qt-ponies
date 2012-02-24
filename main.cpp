#include <QtGui/QApplication>
#include <QTimer>

#include <list>

#include "configwindow.h"
#include "pony.h"

void remove_pony()
{

}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());


    QTimer timer;
    timer.setInterval(30);

    ConfigWindow config;

    for(int i=0;i<1;i++){
        for(auto &i: {
            "Rainbow Dash",
            "Rainbow Dash",
            "Rainbow Dash",
            "Derpy",
            "Applejack",
            "Twilight",
            "Rarity",
            "fluttershy",
            "Pinkie Pie",
        }){
            try{
//                config.ponies.emplace_back(i,&config);
                config.ponies.push_back(std::make_shared<Pony>(i,&config));
            }catch (std::exception &e){
            }
        }
    }


    for(auto &i: config.ponies) {
        QObject::connect(&timer, SIGNAL(timeout()), i.get(), SLOT(update()));
    }

    timer.start();

    return a.exec();
}
