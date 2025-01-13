#ifndef TILE_H
#define TILE_H

#include <QString>
#include <QPixmap>
#include <QDebug>

//Belt的朝向
#define NORTH 0
#define SOUTH 1
#define WEST 2
#define EAST 3

#define TILESIZE 50

//基类Tile
class Tile {
public:
    enum class Type {
        NONE,
        Empty,
        Belt,
        Resource,
        Building
    };
public:
    Tile(){};

    virtual Type getType() const{
        return Type::NONE;
    };

    virtual QPixmap getImage() const{
        qDebug() << "基类Tile没有图片";
        QPixmap pixmap(192, 192);
        pixmap.fill(Qt::white);
        return pixmap;
    };
};

class Empty: public Tile{
public:
    Empty(){};

    Type getType() const override {
        return Tile::Type::Empty;
    }

    QPixmap getImage() const override{
        qDebug() << "Empty类没有图片";
        QPixmap pixmap(192, 192);
        pixmap.fill(Qt::white);
        return pixmap;
    };
};

class Belt: public Tile{
    int direction; //朝向
    QString category; //传送带的种类
    QVector<QPixmap> images;//存储动态帧
    int frameIndex; //当前帧
public:
    Belt(int direction, QString category);

    Type getType() const override{
        return Tile::Type::Belt;
    }

    QPixmap getImage() const override{
        qDebug() << "Empty类没有图片";
        QPixmap pixmap(TILESIZE, TILESIZE);
        pixmap.fill(Qt::white);
        return pixmap;
    };

    void update(){
        if(!images.empty()){
            frameIndex = (frameIndex+1) % images.size();
        }
    }
};

#endif // TILE_H
