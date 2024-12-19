#include "map.h"

Map::Map(int width, int height,QWidget* parent) : QWidget(parent){
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












// void Map::render(QGraphicsScene* scene) {
//     for (int x = 0; x < width; ++x) {
//         for (int y = 0; y < height; ++y) {
//             TileItem* item = new TileItem(tiles[x][y]);
//             item->setPos(x * TILE_SIZE, y * TILE_SIZE);  // 设置位置
//             scene->addItem(item);
//         }
//     }
// }
