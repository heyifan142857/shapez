#ifndef MAP_H
#define MAP_H

#include <QVector>
#include <QGraphicsRectItem>
#include "tile.h"

class Map {
public:
    Map(int width, int height);
    void setTile(int x, int y, const Tile& tile);
    Tile getTile(int x, int y) const;

private:
    QVector<QVector<Tile>> tiles;
};

class TileItem : public QGraphicsRectItem {
public:
    TileItem(const Tile& tile) {
        if (tile.type == Tile::Type::Resource) {
            setBrush(Qt::green); // 假设绿色表示资源
        } else if (tile.type == Tile::Type::Building) {
            setBrush(Qt::blue); // 蓝色表示建筑
        } else {
            setBrush(Qt::gray); // 空白格子
        }
    }
};



#endif // MAP_H
