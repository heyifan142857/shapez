#ifndef MAP_H
#define MAP_H

#include <QWidget>
#include <QString>
#include <QColor>
#include <QPainter>
#include <QBrush>
#include <QGraphicsRectItem>
#include <QTimer>
#include <QLabel>
#include <QHash>
#include <QUuid>
#include <QSharedPointer>
#include <QList>

#include "tile.h"
#include "item.h"


class Map : public QWidget{
    Q_OBJECT
public:
    Map(int width, int height, QWidget* parent = nullptr);
    ~Map();
    void setTile(int x, int y, Tile &tile);
    Tile getTile(int x, int y) const;
    //Tile::Type getTileType(int x, int y) const;
    bool deleteTile(int x, int y);
    int getwidth() const;
    int getheight() const;

    void moveItems();

private slots:
    void updateAnimationFrame();

private:
    QVector<QVector<Tile*>> tiles;
    int width, height;
    int frameIndex;
    QTimer* animationTimer;

    QHash<QUuid, QSharedPointer<Item>> items;
    QList<std::pair<int,int>> miners;
};

#endif // MAP_H
