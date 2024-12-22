#include "gamescene.h"
#include "map.h"

Gamescene::Gamescene(QWidget *parent)
    : QWidget{parent}
{
    //编辑窗口基本信息
    setFixedSize(1600,900);
    setWindowIcon(QIcon(":/res/icon.ico"));
    setWindowTitle("Shapez");

    // Belt *belt = new Belt(NORTH,"forward",0,0,this);
    Map *map = new Map(32,18,this);
    Tile forwardBelt(Tile::Type::Belt, NORTH, "forward");
    map->setTile(10,11,forwardBelt);
}

void Gamescene::paintEvent(QPaintEvent *){}
