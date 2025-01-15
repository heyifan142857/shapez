#include "gamescene.h"
#include <QVBoxLayout>

Gamescene::Gamescene(QWidget *parent)
    : QWidget{parent}
{
    //编辑窗口基本信息
    setFixedSize(1600,900);
    setWindowIcon(QIcon(":/res/icon.ico"));
    setWindowTitle("Shapez");

    qDebug() << "building map";
    map = new Map(18,32,this);
    Tile Hub(Tile::Type::Hub);
    map->setTile(7,14,Hub);
    qDebug() << "successfully build map";

    //测试
    Tile forwardBelt(Tile::Type::Belt, "forward", NORTH);
    Tile forwardBelt2(Tile::Type::Belt, "forward", EAST);
    Tile rightBelt(Tile::Type::Belt, "right", NORTH);
    Tile northMiner(Tile::Type::Building, NORTH, "miner-chainable");
    Tile eastMiner(Tile::Type::Building, EAST, "miner-chainable");
    map->setTile(0,0,rightBelt);
    map->setTile(1,0,forwardBelt);
    map->setTile(2,0,forwardBelt);
    map->setTile(0,1,forwardBelt2);
    map->setTile(5,5,northMiner);
    map->setTile(5,6,eastMiner);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(map);
    setLayout(layout);
}

void Gamescene::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
}


