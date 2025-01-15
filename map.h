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

#include "tile.h"


class Map : public QWidget{
    Q_OBJECT
public:
    Map(int width, int height, QWidget* parent = nullptr);
    ~Map();
    void setTile(int x, int y, Tile &tile);
    Tile getTile(int x, int y) const;
    int getwidth() const;
    int getheight() const;


private slots:
    void updateAnimationFrame();

private:
    QVector<QVector<Tile>> tiles;
    int width, height;
    QTimer* animationTimer;
};

#endif // MAP_H
