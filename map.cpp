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
    if(!deleteTile(x,y)){return;}
    if (x >= tiles.size() || y >= tiles[0].size() || x < 0 || y < 0) {
        qWarning() << "Map::setTile x,y is out of range";
    } else {
        tiles[x][y] = tile;
        tiles[x][y].label = new QLabel(this);
        tiles[x][y].label->setGeometry(y * TILESIZE, x * TILESIZE, tile.size.second * TILESIZE, tile.size.first * TILESIZE);
        if (tile.type == Tile::Type::Belt && !tile.images.empty()) {
            tiles[x][y].label->setPixmap(tile.images[tile.frameIndex]);
        } else {
            tiles[x][y].label->setPixmap(tile.image);
        }
        qDebug() << "successfully set pos("<<x<< ", " <<y<<") a new tile";
    }

    if (tile.size != std::make_pair(1,1)) {
        qDebug() << "size > 1*1";
        for (int i = 0; i < tile.size.first; ++i) {
            for (int j = 0; j < tile.size.second; ++j) {
                if (i == 0 && j == 0) continue;
                if (x + i < tiles.size() && y + j < tiles[0].size()) {
                    tiles[x + i][y + j] = tile;
                    tiles[x + i][y + j].label = new QLabel(this);
                    tiles[x + i][y + j].father = &tiles[x][y];
                    tiles[x][y].sons.push_back(std::make_pair(x + i, y + j));
                }
            }
        }
    }
}

Tile Map::getTile(int x, int y) const{
    if(x >= tiles.size() || y >= (tiles.empty()?0:tiles[0].size()) ||
        x < 0 || y < 0){
        qWarning() << "Map::getTile x,y is out of range";
    }
    if(tiles[x][y].father != nullptr){
        return *(tiles[x][y].father);
    }
    return tiles[x][y];
};

bool Map::deleteTile(int x, int y){
    if(tiles[x][y].type == Tile::Type::Empty){
        return true;
    }
    if(tiles[x][y].type == Tile::Type::Hub){
        qDebug() << "Hub cannot be destoryed";
        return false;
    }
    if(tiles[x][y].father != nullptr){
        Tile father = *tiles[x][y].father;
        for(std::pair<int,int> son:father.sons){
            if(tiles[son.first][son.second].type == Tile::Type::Empty){
                continue;
            }
            tiles[son.first][son.second] = Tile();
            tiles[son.first][son.second].label = new QLabel(this);
            tiles[son.first][son.second].label->setGeometry(son.second * TILESIZE, son.first * TILESIZE, TILESIZE, TILESIZE);
        }
    }
    tiles[x][y] = Tile();
    tiles[x][y].label = new QLabel(this);
    tiles[x][y].label->setGeometry(y * TILESIZE, x * TILESIZE, TILESIZE, TILESIZE);
    return true;
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

