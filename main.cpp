#include "mainscene.h"
#include "map.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    gameMap _gameMap(120, 80);
    Mainscene w;
    w.show();
    qDebug() << "当前在主界面";
    return a.exec();
}
