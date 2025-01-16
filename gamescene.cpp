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
    map->setTile(17,0,forwardBelt);

    // QVBoxLayout* layout = new QVBoxLayout(this);
    // layout->addWidget(map);
    // setLayout(layout);
    // 不使用布局管理器
    map->setGeometry(0, 0, 1600, 900);
}

void Gamescene::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QColor backgroundColor("#ECEEF2");
    painter.fillRect(rect(), backgroundColor);//设置背景色

    int tileSize = TILESIZE;

    //画方格
    QColor lineColor("#E3E7EA");
    QPen pen(lineColor);
    pen.setWidth(1);
    painter.setPen(pen);

    for (int y = 0; y < height(); y += tileSize) {
        painter.drawLine(0, y, width(), y);
    }

    for (int x = 0; x < width(); x += tileSize) {
        painter.drawLine(x, 0, x, height());
    }
}


