#include "map.h"
#include <QDebug>

Map::Map(int height, int width, QWidget* parent) :
    QWidget(parent), width(width), height(height), frameIndex(0) {
    if (width <= 0 || height <= 0) {
        qWarning() << "Invalid map dimensions:" << width << height;
        return;
    }
    tiles.resize(height);
    for (int x = 0; x < height; ++x) {
        tiles[x].resize(width);
        for (int y = 0; y < width; ++y) {
            tiles[x][y] = new Tile();  // 初始化 Tile
            tiles[x][y]->label = new QLabel(this);
            tiles[x][y]->label->setGeometry(y * TILESIZE, x * TILESIZE, TILESIZE, TILESIZE);
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
    for (int i = 0; i < tile.size.first; ++i) {
        for (int j = 0; j < tile.size.second; ++j){
            if(tiles[x + i][y + j]->type != Tile::Type::Empty){
                qDebug() << "Cannot deploy here";
                return;
            }
        }
    }
    if (x >= tiles.size() || y >= tiles[0].size() || x < 0 || y < 0) {
        qWarning() << "Map::setTile x,y is out of range";
    } else {
        delete tiles[x][y];
        tiles[x][y] = new Tile(tile);
        tiles[x][y]->label = new QLabel(this);
        tiles[x][y]->label->setGeometry(y * TILESIZE, x * TILESIZE, tile.size.second * TILESIZE, tile.size.first * TILESIZE);
        if (tile.type == Tile::Type::Belt && !tile.images.empty()) {
            if(frameIndex>tiles[x][y]->images.size()){
                qWarning() << "图片缺失";
            }
            tiles[x][y]->label->setPixmap(tile.images[frameIndex]);
        } else {
            tiles[x][y]->label->setPixmap(tile.image);
        }
        tiles[x][y]->label->show();
        qDebug() << "successfully set pos("<<x<< ", " <<y<<") a new tile";
    }

    if (tile.size != std::make_pair(1,1)) {
        qDebug() << "size > 1*1";
        for (int i = 0; i < tile.size.first; ++i) {
            for (int j = 0; j < tile.size.second; ++j) {
                if (i == 0 && j == 0) continue;
                if (x + i < tiles.size() && y + j < tiles[0].size()) {
                    delete tiles[x + i][y + j];
                    tiles[x + i][y + j] = new Tile(tile);
                    tiles[x + i][y + j]->label = new QLabel(this);
                    tiles[x + i][y + j]->father = new std::pair<int, int>(x,y);
                    tiles[x][y]->sons.push_back(std::make_pair(x + i, y + j));
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
    if(tiles[x][y]->father != nullptr){
        return *(tiles[tiles[x][y]->father->first][tiles[x][y]->father->second]);
    }
    return *tiles[x][y];
};

bool Map::deleteTile(int x, int y){
    qDebug() << "Deleting tile at (" << x << ", " << y << ")";
    if(tiles[x][y]->type == Tile::Type::Empty){
        return true;
    }
    if(tiles[x][y]->type == Tile::Type::Hub || tiles[x][y]->type == Tile::Type::Resource){
        qDebug() << "Hub/Resource cannot be destoryed";
        return false;
    }
    if(tiles[x][y]->father != nullptr){
        std::pair<int,int>* father = tiles[x][y]->father;
        for(std::pair<int,int> son:tiles[father->first][father->second]->sons){
            if(tiles[son.first][son.second]->type == Tile::Type::Empty){
                continue;
            }
            delete tiles[son.first][son.second];
            tiles[son.first][son.second] = new Tile();
            tiles[son.first][son.second]->label = new QLabel(this);
            tiles[son.first][son.second]->label->setGeometry(son.second * TILESIZE, son.first * TILESIZE, TILESIZE, TILESIZE);
        }
        delete tiles[father->first][father->second];
        tiles[father->first][father->second] = new Tile();
        tiles[father->first][father->second]->label = new QLabel(this);
        tiles[father->first][father->second]->label->setGeometry(father->second * TILESIZE, father->first * TILESIZE, TILESIZE, TILESIZE);
        return true;
    }else{
        if(tiles[x][y]->size != std::make_pair(1,1)){
            for(std::pair<int,int> son:tiles[x][y]->sons){
                if(tiles[son.first][son.second]->type == Tile::Type::Empty){
                    continue;
                }
                delete tiles[son.first][son.second];
                tiles[son.first][son.second] = new Tile();
                tiles[son.first][son.second]->label = new QLabel(this);
                tiles[son.first][son.second]->label->setGeometry(son.second * TILESIZE, son.first * TILESIZE, TILESIZE, TILESIZE);
            }
        }
        delete tiles[x][y];
        tiles[x][y] = new Tile();
        tiles[x][y]->label = new QLabel(this);
        tiles[x][y]->label->setGeometry(y * TILESIZE, x * TILESIZE, TILESIZE, TILESIZE);
        return true;
    }
};

int Map::getwidth() const{
    return width;
}

int Map::getheight() const{
    return height;
}

void Map::updateAnimationFrame() {
    frameIndex = (frameIndex + 1) % 14;  // 循环播放动画
    for (int x = 0; x < tiles.size(); ++x) {
        for (int y = 0; y < tiles[x].size(); ++y) {
            Tile* tile = tiles[x][y];
            if (tile->type == Tile::Type::Belt && !tile->images.empty()) {
                if(frameIndex > tile->images.size()){
                    qWarning() << "图片缺失";
                }
                tile->label->setPixmap(tile->images[frameIndex]);  // 更新 QLabel 的图像
            }
        }
    }
}


