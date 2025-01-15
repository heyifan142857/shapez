#ifndef TILE_H
#define TILE_H

#include <QWidget>
#include <QVector>
#include <QString>
#include <QLabel>

#define NORTH 0
#define SOUTH 1
#define WEST 2
#define EAST 3

#define TILESIZE 50

class Tile {
public:
    enum class Type {
        Empty,
        Belt,
        Resource,
        Color,
        Building
    };

    Tile(): type(Type::Empty), name(""){}
    Tile(Type type, const QString& name , int direction);
    Tile(Type type, int direction, QString state);

    Type type;        // Tile的类型（空白、传送带、资源、建筑）
    int direction; //朝向
    QString state; //传送带的种类
    QString name; // 名称
    QVector<QPixmap> images;//存储动态帧
    QPixmap image;//存储静态图像
    int frameIndex;   // 当前显示的帧的索引
    QLabel* label;    // 用于显示图像的 QLabel
};

#endif // TILE_H
