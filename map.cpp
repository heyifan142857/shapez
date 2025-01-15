#include "map.h"
#include <QDebug>

Map::Map(int height, int width, QWidget* parent) :
    QWidget(parent), width(width), height(height) {
    if (width <= 0 || height <= 0) {
        qWarning() << "Invalid map dimensions:" << width << height;
        return;
    }
    tiles.resize(height);
    for (int x = 0; x < height; ++x) {
        tiles[x].resize(width);
        for (int y = 0; y < width; ++y) {
            tiles[x][y] = Tile();  // 初始化 Tile
            tiles[x][y].label = new QLabel(this);
            tiles[x][y].label->setGeometry(y * TILESIZE, x * TILESIZE, TILESIZE, TILESIZE);
        }
    }

    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &Map::updateAnimationFrame);
    animationTimer->start(20); // 每 20 毫秒更新一次帧
}

Map::~Map() {
    delete animationTimer;
}

void Map::setTile(int x, int y, Tile &tile) {
    qDebug() << "setting pos("<<x<<", "<<y<<") a new tile";
    if (x >= tiles.size() || y >= tiles[0].size() || x < 0 || y < 0) {
        qWarning() << "Map::setTile x,y is out of range";
    } else {
        tiles[x][y] = tile;
        tiles[x][y].label = new QLabel(this);
        tiles[x][y].label->setGeometry(y * TILESIZE, x * TILESIZE, TILESIZE, TILESIZE);
        if (tile.type == Tile::Type::Belt && !tile.images.empty()) {
            tiles[x][y].label->setPixmap(tile.images[tile.frameIndex]);
        } else {
            tiles[x][y].label->setPixmap(tile.pixmap);
        }
        qDebug() << "successfully set pos("<<x<< ", " <<y<<") a new tile";
    }
}

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

void Map::updateAnimationFrame() {
    for (int x = 0; x < tiles.size(); ++x) {
        for (int y = 0; y < tiles[x].size(); ++y) {
            Tile& tile = tiles[x][y];
            if (tile.type == Tile::Type::Belt && !tile.images.empty()) {
                tile.frameIndex = (tile.frameIndex + 1) % tile.images.size();  // 循环播放动画
                tile.label->setPixmap(tile.images[tile.frameIndex]);  // 更新 QLabel 的图像
            }
        }
    }
}

