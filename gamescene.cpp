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
    map = new Map(32,18,this);
    Tile forwardBelt(Tile::Type::Belt, NORTH, "forward");
    map->setTile(0,0,forwardBelt);
    map->setTile(1,0,forwardBelt);
    map->setTile(2,0,forwardBelt);
}

void Gamescene::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 通过 painter 绘制地图
    if(map!=nullptr){
        qDebug() << "Calling paintMap in paintEvent";
        map->paintMap(&painter);
    }
}


