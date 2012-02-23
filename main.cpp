#include <QtGui/QApplication>
#include <QTimer>

#include <list>

#include "pony.h"

std::list<Pony> ponies;

void remove_pony()
{

}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());


    QTimer timer;
    timer.setInterval(30);

    for(int i=0;i<1;i++){
        for(auto &i: {
            "Rainbow Dash",
            "Derpy",
            "Applejack",
            "Twilight",
            "Rarity",
            "fluttershy",
            "Pinkie Pie",
        }){
            try{
                ponies.emplace_back(i);
            }catch (std::exception &e){
            }
        }
    }


    for(auto &i: ponies) {
        QObject::connect(&timer, SIGNAL(timeout()), &i, SLOT(update()));
    }

    timer.start();

    return a.exec();
}
