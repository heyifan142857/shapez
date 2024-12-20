#include "map.h"

Tile::Tile(Type type, const QString& resource, const QString& building):
    type(type), resource(resource), building(building),
    isAnimated(type==Type::Belt ? true : false) {

}

Tile::Tile(Type type, int direction, QString state):
    type(type), direction(direction), state(state), frameIndex(0){
    for (int i = 0; i < 14; ++i) {
        QPixmap pixmap;
        if (!pixmap.load(QString(":/res/belt/%1_%2.png").arg(state).arg(i))) {
            qWarning() << "Failed to load image:" << QString(":/res/belt/%1_%2.png").arg(state).arg(i);
        }
        QPixmap scaledpixmap = pixmap.scaled(50, 50, Qt::KeepAspectRatio);
        images.push_back(scaledpixmap);
    }
}

Map::Map(int width, int height,QWidget* parent) :
    QWidget(parent), width(width), height(height){
    // 假设 width 和 height 分别表示地图的宽度和高度
    tiles.resize(width);  // 先调整外部 QVector 的大小，表示列数
    for (int i = 0; i < width; ++i) {
        tiles[i].resize(height);  // 再调整每个内部 QVector 的大小，表示行数
    }
}

void Map::setTile(int x, int y, const Tile& tile){
    if(x >= tiles.size() || y >= (tiles.empty()?0:tiles[0].size()) ||
        x < 0 || y < 0){
        qWarning() << "Map::setTile x,y is out of range";
    }else{
        tiles[x][y] = tile;
    }
};

Tile Map::getTile(int x, int y) const{
    if(x >= tiles.size() || y >= (tiles.empty()?0:tiles[0].size()) ||
        x < 0 || y < 0){
        qWarning() << "Map::getTile x,y is out of range";
    }
    return tiles[x][y];
};

int Map::getwidth(){
    return width;
}

int Map::getheight(){
    return height;
}








