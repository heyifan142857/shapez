#ifndef TILE_H
#define TILE_H

#include <QWidget>
#include <QVector>
#include <QString>
#include <QLabel>

#include "item.h"

#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3

#define TILESIZE 50

class Tile {
public:
    enum class Type {
        Empty,
        Hub,
        Belt,
        Resource,
        Color,
        Building
    };

    Tile(): type(Type::Empty), name(""), label(nullptr), direction(NORTH), size(std::make_pair(1,1)){}
    Tile(Type type, int direction, const QString& name, std::pair<int,int> size = std::make_pair(1,1));
    Tile(Type type, QString state, int direction);//Belt构造函数
    Tile(Type type, QString name = "Hub");//Hub构造函数

    void changeDirection();
    void changeState();

    ~Tile();

    Tile(const Tile& other);

    Type type;        // Tile的类型（空白、传送带、资源、建筑）
    int direction; //朝向
    QString state; //传送带的种类
    QString name; // 名称
    QVector<QPixmap> images;//存储动态帧
    QPixmap image;//存储静态图像
    QLabel* label;    // 用于显示图像的 QLabel
    std::pair<int,int> size; //表示大小
    std::pair<int,int>* father = nullptr;
    QVector<std::pair<int,int>> sons;
    Tile* mine = nullptr;//为miner准备的
    Item* item = nullptr;//显示矿物
};

#endif // TILE_H
