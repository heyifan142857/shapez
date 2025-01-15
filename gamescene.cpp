#include "gamescene.h"
#include <QVBoxLayout>
#include "map.h"

Gamescene::Gamescene(QWidget *parent)
    : QWidget{parent}
{
    //编辑窗口基本信息
    setFixedSize(1600,900);
    setWindowIcon(QIcon(":/res/icon.ico"));
    setWindowTitle("Shapez");

    // Belt *belt = new Belt(NORTH,"forward",0,0,this);
    map = new Map(18,32,this);
    Tile forwardBelt(Tile::Type::Belt, NORTH, "forward");
    Tile forwardBelt2(Tile::Type::Belt, EAST, "forward");
    Tile rightBelt(Tile::Type::Belt, NORTH, "right");
    map->setTile(0,0,rightBelt);
    map->setTile(1,0,forwardBelt);
    map->setTile(2,0,forwardBelt);
    map->setTile(0,1,forwardBelt2);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(map);
    setLayout(layout);
}

void Gamescene::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
}


