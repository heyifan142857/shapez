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
//#include "item.h"


class Map : public QWidget{
    Q_OBJECT
public:
    Map(int width, int height, QWidget* parent = nullptr);
    ~Map();
    void setTile(int x, int y, Tile &tile);
    Tile getTile(int x, int y) const;
    Tile getTile(std::pair<int,int> pos) const;
    //Tile::Type getTileType(int x, int y) const;
    bool deleteTile(int x, int y);
    int getwidth() const;
    int getheight() const;
    std::pair<int,int> nextPox(int x,int y,Tile &currentTile);
    std::pair<int,int> nextPox(std::pair<int,int> originaPos,Tile &currentTile);
    bool inMap(int x,int y);
    bool inMap(std::pair<int,int> originaPos);
    std::pair<std::pair<int,int>,std::pair<int,int>> cutterOutPox(std::pair<int,int> pos,Tile &cutterTile);
    std::pair<std::pair<int,int>,std::pair<int,int>> cutterOutPox(int x, int y,Tile &cutterTile);

    void moveItems();
    void moveSingleItem(int x,int y,QSet<std::pair<int, int>> &movedItems);
    bool canEnter(int direction, std::pair<int,int> pos);

    void performMining();

    void cutterUpdate();

    void setItem(std::pair<int,int> pos, Item *item);

    void itemToHub(int part1, int part2, int part3, int part4);

private slots:
    void updateAnimationFrame();

public:
    QLabel* questionLabel = nullptr;
    QLabel* countLabel = nullptr;
    QLabel* levelLabel = nullptr;

    int questionLever;

    int current;
    int target;

    //int coins = 0;

    QVector<QVector<Tile*>> tiles;
private:
    int width, height;
    int frameIndex;
    QTimer* animationTimer;

    QList<std::pair<int,int>> miners;
    //QList<Item*> items;
};

#endif // MAP_H
