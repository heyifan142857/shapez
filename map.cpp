#include "map.h"
#include <QDebug>

Tile::Tile(Type type, const QString& resource, const QString& building):
    type(type), resource(resource), building(building),
    isAnimated(type==Type::Belt ? true : false) {
}

//专门为Belt而写的构造函数，因为只有Belt会动
Tile::Tile(Type type, int direction, QString state):
    type(type), direction(direction), state(state), frameIndex(0),isAnimated(true){
    //根据direction设置旋转角度
    QTransform transform;
    int angle;
    switch (direction) {
    case NORTH:
        angle = 0;
        break;
    case WEST:
        angle = 90;
        break;
    case SOUTH:
        angle = 180;
        break;
    case EAST:
        angle = 270;
        break;
    default:
        angle = 0;
        qWarning() << "undefined direction when construct a new Tile";
        break;
    }
    transform.rotate(angle);

    for (int i = 0; i < 14; ++i) {
        QPixmap pixmap;
        if (!pixmap.load(QString(":/res/belt/%1_%2.png").arg(state).arg(i))) {
            qWarning() << "Failed to load image:" << QString(":/res/belt/%1_%2.png").arg(state).arg(i);
            continue;  // 忽略加载失败的图像
        }
        QPixmap scaledpixmap = pixmap.scaled(TILESIZE, TILESIZE, Qt::KeepAspectRatio).transformed(transform);
        images.push_back(scaledpixmap);
    }
}


Map::Map(int width, int height,QWidget* parent) :
    QWidget(parent), width(width), height(height){
    // 假设 width 和 height 分别表示地图的宽度和高度
    if (width <= 0 || height <= 0) {
        qWarning() << "Invalid map dimensions:" << width << height;
        return;
    }
    tiles.resize(width);
    for (auto& row : tiles) {
        row.resize(height);  // 每一列设置为 height 行，每个元素默认初始化为 Tile()
    }

    // 设置定时器来更新动画
    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &Map::updateAnimationFrame);
    animationTimer->start(20); // 每 20 毫秒更新一次帧
}

Map::~Map() {
    // delete animationTimer;
}

void Map::setTile(int x, int y, Tile tile){
    if(x >= tiles.size() || y >= tiles[0].size() ||
        x < 0 || y < 0){
        qWarning() << "Map::setTile x,y is out of range";
    }else{
        tiles[x][y] = tile;
        qDebug() << "set (" << x << ", " << y << ") a new tile";
        //tile.setLocation();
    }
};

Tile Map::getTile(int x, int y) const{
    if(x >= tiles.size() || y >= (tiles.empty()?0:tiles[0].size()) ||
        x < 0 || y < 0){
        qWarning() << "Map::getTile x,y is out of range";
    }
    return tiles[x][y];
};

int Map::getwidth() const{
    return width;
}

int Map::getheight() const{
    return height;
}

void Map::paintMap(QPainter* painter) {
    qDebug() << "paiting map";
    for (int x = 0; x < tiles.size(); ++x) {
        for (int y = 0; y < tiles[x].size(); ++y) {
            const Tile& tile = tiles[x][y];
            if (tile.type == Tile::Type::Belt) {
                if (tile.isAnimated) {
                    painter->drawPixmap(x * TILESIZE, y * TILESIZE, TILESIZE, TILESIZE, tile.images[tile.frameIndex]);
                } else {
                    painter->drawPixmap(x * TILESIZE, y * TILESIZE, TILESIZE, TILESIZE, tile.pixmap);
                }
            }
        }
    }
}

void Map::updateAnimationFrame(){
    for (int x = 0; x < tiles.size(); ++x) {
        for (int y = 0; y < tiles[x].size(); ++y) {
            Tile& tile = tiles[x][y];
            if (tile.isAnimated) {
                tile.frameIndex = (tile.frameIndex + 1) % tile.images.size();  // 循环播放动画
            }
        }
    }
    update();  // 触发重新绘制
}

