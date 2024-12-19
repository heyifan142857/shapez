#ifndef MAP_H
#define MAP_H

#include <QWidget>
#include <QVector>
#include <QString>
#include <QColor>
#include <QBrush>
#include <QGraphicsRectItem>
#include <QDebug>

class Tile {
public:
    enum class Type {
        Empty,
        Resource,
        Building
    };

    Tile()
        : type(Type::Empty), resource(""), building("") {}

    Tile(Type type, const QString& resource = "", const QString& building = "")
        : type(type), resource(resource), building(building) {}

    Type type;        // Tile的类型（空白、资源、建筑）
    QString resource; // 资源的类型（如果是资源类型的话）
    QString building; // 建筑的名称（如果是建筑类型的话）
};


class Map : public QWidget{
    Q_OBJECT
public:
    Map(int width, int height,QWidget* parent = nullptr);
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
