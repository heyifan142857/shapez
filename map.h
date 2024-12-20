#ifndef MAP_H
#define MAP_H

#include <QWidget>
#include <QVector>
#include <QString>
#include <QColor>
#include <QBrush>
#include <QGraphicsRectItem>
#include <QDebug>

#define NORTH 0
#define SOUTH 1
#define WEST 2
#define EAST 3

class Tile {
public:
    enum class Type {
        Empty,
        Belt,
        Resource,
        Building
    };

    Tile(): type(Type::Empty), resource(""), building("") {}
    Tile(Type type, const QString& resource = "", const QString& building = "");
    Tile(Type type, int direction, QString state);


    Type type;        // Tile的类型（空白、传送带、资源、建筑）
    int direction; //朝向
    QString state; //传送带的种类
    QString resource; // 资源的类型（如果是资源类型的话）
    QString building; // 建筑的名称（如果是建筑类型的话）
    QVector<QPixmap> images;//存储动态帧
    QPixmap pixmap;//存储静态图像
    bool isAnimated;  // 标记是否需要动画
    int frameIndex;   // 当前显示的帧的索引
};


class Map : public QWidget{
    Q_OBJECT
public:
    Map(int width, int height, QWidget* parent = nullptr);
    void setTile(int x, int y, const Tile& tile);
    Tile getTile(int x, int y) const;
    int getwidth();
    int getheight();

private:
    QVector<QVector<Tile>> tiles;
    int width, height;
};

#endif // MAP_H
