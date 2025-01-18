#include <QDebug>
#include <QMutableListIterator>
#include "map.h"

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
    if(tile.type == Tile::Type::Building && tile.name == "miner"){
        if(tiles[x][y]->type == Tile::Type::Resource){
            tile.mine = tiles[x][y];
            miners.append(std::make_pair(x,y));
            qDebug() << "add a mine to miners";
        }else{
            delete tiles[x][y];
        }
        tiles[x][y] = new Tile(tile);
        tiles[x][y]->label = new QLabel(this);
        tiles[x][y]->label->setGeometry(y * TILESIZE, x * TILESIZE, tile.size.second * TILESIZE, tile.size.first * TILESIZE);
        tiles[x][y]->label->setPixmap(tile.image);
        tiles[x][y]->label->show();
        qDebug() << "successfully set pos("<<x<< ", " <<y<<") a new tile";
        return;
    }
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
    if(tiles[x][y]->type == Tile::Type::Building && tiles[x][y]->name == "miner" && tiles[x][y]->mine){
        Tile* temp = tiles[x][y];
        tiles[x][y] = new Tile(*temp->mine);
        delete temp;
        return true;
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

void Map::moveItems() {
    //todo
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

void Map::performMining(){
    if(!miners.empty()){
        QMutableListIterator<std::pair<int, int>> it(miners);

        while (it.hasNext()) {
            std::pair<int,int> pos = it.next();
            if(tiles[pos.first][pos.second]->type!=Tile::Type::Building || tiles[pos.first][pos.second]->name!="miner"){
                it.remove();
                qDebug() << "delete a invalid miner in list miners";
            }else{
                std::pair<int,int> generatePos = nextPox(pos,*tiles[pos.first][pos.second]);
                if(!inMap(generatePos)){
                    qDebug() << "miner out of map";
                    return;
                }
                if(tiles[generatePos.first][generatePos.second]->type == Tile::Type::Belt && tiles[generatePos.first][generatePos.second]->direction==tiles[pos.first][pos.second]->direction){
                    if(tiles[generatePos.first][generatePos.second]->item == nullptr){
                        QString minename = tiles[pos.first][pos.second]->mine->name;
                        tiles[generatePos.first][generatePos.second]->item = new Item(minename,generatePos);
                        tiles[generatePos.first][generatePos.second]->item->label = new QLabel(this);
                        tiles[generatePos.first][generatePos.second]->item->label->setGeometry(generatePos.second * TILESIZE, generatePos.first * TILESIZE, TILESIZE, TILESIZE);
                        QPixmap minePixmap = tiles[generatePos.first][generatePos.second]->item->getPixmap();
                        tiles[generatePos.first][generatePos.second]->item->label->setPixmap(minePixmap);
                        tiles[generatePos.first][generatePos.second]->item->label->show();
                        //tiles[generatePos.first][generatePos.second]->item->label->raise();
                    }
                }
            }
        }
    }
}

std::pair<int,int> Map::nextPox(int x,int y,Tile &currentTile){
    std::pair<int,int> newPos;
    if(currentTile.type != Tile::Type::Belt){
        switch (currentTile.direction) {
        case NORTH:
            newPos = std::make_pair(x-1,y);
            break;
        case EAST:
            newPos = std::make_pair(x,y+1);
            break;
        case SOUTH:
            newPos = std::make_pair(x+1,y);
            break;
        case WEST:
            newPos = std::make_pair(x,y-1);
            break;
        default:
            newPos = std::make_pair(x, y);
            break;
        }
    }else{
        int beltDirection = currentTile.direction;
        if(currentTile.state == "left"){
            beltDirection = (beltDirection+3)%4;
        }
        if(currentTile.state == "right"){
            beltDirection = (beltDirection+1)%4;
        }

        switch (beltDirection) {
        case NORTH:
            newPos = std::make_pair(x-1,y);
            break;
        case EAST:
            newPos = std::make_pair(x,y+1);
            break;
        case SOUTH:
            newPos = std::make_pair(x+1,y);
            break;
        case WEST:
            newPos = std::make_pair(x,y-1);
            break;
        default:
            newPos = std::make_pair(x, y);
            break;
        }
    }

    return newPos;
}
std::pair<int,int> Map::nextPox(std::pair<int,int> originaPos,Tile &currentTile){
    int x = originaPos.first;
    int y = originaPos.second;

    std::pair<int,int> newPos;
    if(currentTile.type != Tile::Type::Belt){
        switch (currentTile.direction) {
        case NORTH:
            newPos = std::make_pair(x-1,y);
            break;
        case EAST:
            newPos = std::make_pair(x,y+1);
            break;
        case SOUTH:
            newPos = std::make_pair(x+1,y);
            break;
        case WEST:
            newPos = std::make_pair(x,y-1);
            break;
        default:
            newPos = std::make_pair(x, y);
            break;
        }
    }else{
        int beltDirection = currentTile.direction;
        if(currentTile.state == "left"){
            beltDirection = (beltDirection+3)%4;
        }
        if(currentTile.state == "right"){
            beltDirection = (beltDirection+1)%4;
        }

        switch (beltDirection) {
        case NORTH:
            newPos = std::make_pair(x-1,y);
            break;
        case EAST:
            newPos = std::make_pair(x,y+1);
            break;
        case SOUTH:
            newPos = std::make_pair(x+1,y);
            break;
        case WEST:
            newPos = std::make_pair(x,y-1);
            break;
        default:
            newPos = std::make_pair(x, y);
            break;
        }
    }

    return newPos;
}

bool Map::inMap(int x,int y){
    if( x<0 || y<0 || x>=height || y>=width){
        return false;
    }else{
        return true;
    }
}

bool Map::inMap(std::pair<int,int> originaPos){
    int x = originaPos.first;
    int y = originaPos.second;

    if( x<0 || y<0 || x>=height || y>=width){
        return false;
    }else{
        return true;
    }
}


