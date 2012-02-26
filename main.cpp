#include <QtGui/QApplication>

#include "configwindow.h"
#include "pony.h"

void remove_pony()
{

}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());
    QCoreApplication::setApplicationName("qt-ponies");

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
            "Big Celestia"
        }){
            try{
//                config.ponies.emplace_back(i,&config);
                config.ponies.push_back(std::make_shared<Pony>(i,&config));
            }catch (std::exception &e){
            }
        }
    }


    for(auto &i: config.ponies) {
        QObject::connect(&config.timer, SIGNAL(timeout()), i.get(), SLOT(update()));
    }

    config.show();

    return a.exec();
}
