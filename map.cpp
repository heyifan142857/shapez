#include <QDebug>
#include <QMutableListIterator>
#include <QMouseEvent>
#include "map.h"
#include "configmanager.h"
#include "localization.h"

namespace {

struct BuildingInfoEntry {
    QString zhName;
    QString enName;
    QString zhDescription;
    QString enDescription;
};

bool isInspectableTile(const Tile &tile)
{
    return tile.type == Tile::Type::Hub ||
           tile.type == Tile::Type::Belt ||
           tile.type == Tile::Type::Building;
}

BuildingInfoEntry buildingInfoEntryForTile(const Tile &tile)
{
    if (tile.type == Tile::Type::Hub) {
        return {
            "基地",
            "Hub",
            "接收并统计目标图形，也是整张地图的核心。",
            "Receives target shapes, tracks progress, and acts as the center of your factory."
        };
    }

    if (tile.type == Tile::Type::Belt) {
        return {
            "传送带",
            "Belt",
            "沿朝向运输物品，拖动铺设时可以自动形成转弯。",
            "Moves items forward and can automatically form turns while dragging belts."
        };
    }

    if (tile.name == "balancer") {
        return {
            "平衡器",
            "Balancer",
            "把两路输入平均分配到两路输出，适合整理产线吞吐。",
            "Splits two input flows evenly across two outputs to keep your production lines balanced."
        };
    }

    if (tile.name == "underground_belt_entry") {
        return {
            "地下传送带（入口）",
            "Underground Belt (Entry)",
            "让物品从地下穿过拥挤区域，最远传送4格。放置时若附近有出口会自动显示连接预览。",
            "Sends items underground to cross busy areas. Max range: 4 tiles. Shows a preview line when a matching exit is nearby."
        };
    }

    if (tile.name == "underground_belt_exit") {
        return {
            "地下传送带（出口）",
            "Underground Belt (Exit)",
            "接收来自入口的地下传输物品，并从前方输出。",
            "Receives items sent underground from a matching entry and outputs them forward."
        };
    }

    if (tile.name == "miner") {
        return {
            "开采器",
            "Miner",
            "放在矿物上持续采集资源，并朝前方输出。",
            "Extracts resources from ore tiles continuously and outputs them forward."
        };
    }

    if (tile.name == "cutter") {
        return {
            "切割机",
            "Cutter",
            "把输入图形切成两部分，为更复杂的目标做准备。",
            "Cuts incoming shapes into two parts for more advanced production goals."
        };
    }

    if (tile.name == "rotater") {
        return {
            "旋转器",
            "Rotator",
            "将输入图形顺时针旋转90°后输出，用来调整拼装方向。",
            "Rotates incoming shapes 90° clockwise before output so you can align later processing."
        };
    }

    if (tile.name == "stacker") {
        return {
            "合成器",
            "Stacker",
            "将两路不重叠的图形合成为一个目标产物，是后期多矿物关卡的核心建筑。",
            "Combines two non-overlapping shapes into one target item, making mixed-resource levels possible."
        };
    }

    if (tile.name == "mixer") {
        return {
            "混合器",
            "Mixer",
            "将多路输入整合到同一产线，便于后续加工。<br/><span style='color:red;'>⚠ 功能尚未实现</span>",
            "Merges multiple inputs into one combined line for later processing.<br/><span style='color:red;'>⚠ Not yet implemented</span>"
        };
    }

    if (tile.name == "painter") {
        return {
            "染色器",
            "Painter",
            "给输入图形上色，让产线能够制作彩色目标。<br/><span style='color:red;'>⚠ 功能尚未实现</span>",
            "Applies color to incoming shapes so you can produce painted targets.<br/><span style='color:red;'>⚠ Not yet implemented</span>"
        };
    }

    if (tile.name == "trash") {
        return {
            "垃圾桶",
            "Trash",
            "销毁输入物品，用来清理多余或错误的产物。",
            "Deletes incoming items so you can remove extra or incorrect outputs."
        };
    }

    return {};
}

} // namespace

Map::Map(int height, int width, QWidget* parent) :
    QWidget(parent), width(width), height(height), frameIndex(0) {
    if (width <= 0 || height <= 0) {
        qWarning() << "Invalid map dimensions:" << width << height;
        return;
    }

    setMouseTracking(true);

    tiles.resize(height);
    for (int x = 0; x < height; ++x) {
        tiles[x].resize(width);
        for (int y = 0; y < width; ++y) {
            tiles[x][y] = new Tile();  // 初始化 Tile
            tiles[x][y]->label = new QLabel(this);
            tiles[x][y]->label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
            tiles[x][y]->label->hide();
        }
    }

    blueprintLabel = new QLabel(this);
    blueprintLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    blueprintLabel->setScaledContents(true);
    blueprintLabel->hide();

    buildingInfoLabel = new QLabel(this);
    buildingInfoLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    buildingInfoLabel->setTextFormat(Qt::RichText);
    buildingInfoLabel->setWordWrap(true);
    buildingInfoLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    buildingInfoLabel->setStyleSheet(
        "QLabel {"
        "background-color: rgb(236, 238, 242);"
        "color: #1F2328;"
        "border: 1px solid rgba(31, 35, 40, 40);"
        "border-radius: 10px;"
        "padding: 10px;"
        "}"
    );
    buildingInfoLabel->raise();
    buildingInfoLabel->hide();

    place_beltEffect.setSource(QUrl("qrc:/res/sounds/place_belt.wav"));
    place_beltEffect.setVolume(0.5f);

    place_buildingEffect.setSource(QUrl("qrc:/res/sounds/place_building.wav"));
    place_buildingEffect.setVolume(0.5f);

    current = 0;
    target = 1;
    questionLever = 0;

    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &Map::updateAnimationFrame);
    animationTimer->start(20); // 每 20 毫秒更新一次帧

    buildingInfoTimer = new QTimer(this);
    buildingInfoTimer->setSingleShot(true);
    connect(buildingInfoTimer, &QTimer::timeout, this, &Map::showBuildingInfo);

    questionLabel = new QLabel(this);
    questionLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    questionLabel->setScaledContents(true);
    questionLabel->show();
    questionLabel->raise();

    countLabel = new QLabel(this);
    countLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    countLabel->show();
    countLabel->raise();

    levelLabel = new QLabel(this);
    levelLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    levelLabel->show();
    levelLabel->raise();

    updateLayout();
}

Map::~Map() {
    for (int x = 0; x < tiles.size(); ++x) {
        for (int y = 0; y < tiles[x].size(); ++y) {
            delete tiles[x][y];
            tiles[x][y] = nullptr;
        }
    }
}

void Map::setTile(int x, int y, Tile &tile, bool playSound) {
    hideBuildingInfo();
    qDebug() << "setting pos("<<x<<", "<<y<<") a new tile";
    if (tile.type == Tile::Type::Hub) {
        hudAnchorRow = x + 1;
        hudAnchorColumn = y + 1;
        updateHudGeometry();
        questionLabel->raise();
        countLabel->raise();
        levelLabel->raise();
    }
    if(tile.type == Tile::Type::Building && tile.name == "miner"){
        if(tile.mine == nullptr && tiles[x][y]->type == Tile::Type::Resource){
            tile.mine = new Tile(*tiles[x][y]);
        }
        if (tile.mine != nullptr) {
            miners.append(std::make_pair(x,y));
            qDebug() << "add a mine to miners";
        }
        delete tiles[x][y];
        tiles[x][y] = new Tile(tile);
        tiles[x][y]->label = new QLabel(this);
        tiles[x][y]->label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        updateTileLabel(x, y);

        if (playSound) {
            place_buildingEffect.play();
        }
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
        tiles[x][y]->label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        if (tile.type == Tile::Type::Belt) {
            if (playSound) {
                place_beltEffect.play();
            }
        } else {
            if (playSound) {
                place_buildingEffect.play();
            }
        }
        updateTileLabel(x, y);
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
                    tiles[x + i][y + j]->label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
                    tiles[x + i][y + j]->father = new std::pair<int, int>(x,y);
                    tiles[x][y]->sons.push_back(std::make_pair(x + i, y + j));
                    updateTileLabel(x + i, y + j);
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

Tile Map::getTile(std::pair<int,int> pos) const{
    int x = pos.first;
    int y = pos.second;

    if(x >= tiles.size() || y >= (tiles.empty()?0:tiles[0].size()) ||
        x < 0 || y < 0){
        qWarning() << "Map::getTile x,y is out of range";
    }
    if(tiles[x][y]->father != nullptr){
        return *(tiles[tiles[x][y]->father->first][tiles[x][y]->father->second]);
    }
    return *tiles[x][y];
}

bool Map::deleteTile(int x, int y){
    hideBuildingInfo();
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
        tiles[x][y]->label = new QLabel(this);
        tiles[x][y]->label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        delete temp;
        updateTileLabel(x, y);
        return true;
    }
    if(tiles[x][y]->father != nullptr){
        const std::pair<int,int> fatherPos = *tiles[x][y]->father;
        const QVector<std::pair<int,int>> sons = tiles[fatherPos.first][fatherPos.second]->sons;

        for(const std::pair<int,int> &son : sons){
            if(tiles[son.first][son.second]->type == Tile::Type::Empty){
                continue;
            }
            delete tiles[son.first][son.second];
            tiles[son.first][son.second] = new Tile();
            tiles[son.first][son.second]->label = new QLabel(this);
            tiles[son.first][son.second]->label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
            updateTileLabel(son.first, son.second);
        }
        delete tiles[fatherPos.first][fatherPos.second];
        tiles[fatherPos.first][fatherPos.second] = new Tile();
        tiles[fatherPos.first][fatherPos.second]->label = new QLabel(this);
        tiles[fatherPos.first][fatherPos.second]->label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        updateTileLabel(fatherPos.first, fatherPos.second);
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
                tiles[son.first][son.second]->label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
                updateTileLabel(son.first, son.second);
            }
        }
        delete tiles[x][y];
        tiles[x][y] = new Tile();
        tiles[x][y]->label = new QLabel(this);
        tiles[x][y]->label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        updateTileLabel(x, y);
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
    QSet<std::pair<int, int>> movedItems;
    for (int x = 0; x < height; ++x) {
        for (int y = 0; y < width; ++y) {
            if(movedItems.contains({x, y})){
                continue;
            }
            moveSingleItem(x,y,movedItems);
        }
    }
}

void Map::moveSingleItem(int x,int y,QSet<std::pair<int, int>> &movedItems){
    if(tiles[x][y]->type != Tile::Type::Belt){
        return;
    }
    if(tiles[x][y]->item == nullptr){
        return;
    }
    std::pair<int,int> newPos = nextPox(x,y,*tiles[x][y]);
    if(!inMap(newPos)){
        return;
    }
    Tile *targetTile = tiles[newPos.first][newPos.second];
    if(targetTile->type == Tile::Type::Color || targetTile->type == Tile::Type::Empty || targetTile->type == Tile::Type::Resource){
        return;
    }

    int realDirection = tiles[x][y]->direction;
    if(tiles[x][y]->state == "left"){
        realDirection = (realDirection+3)%4;
    }
    if(tiles[x][y]->state == "right"){
        realDirection = (realDirection+1)%4;
    }

    Item *item = tiles[x][y]->item;
    if(!item){
        return;
    }

    if(!tryInsertItemAt(QPoint(newPos.first, newPos.second), realDirection, item)){
        return;
    }

    tiles[x][y]->item = nullptr;
    if(targetTile->type == Tile::Type::Belt){
        movedItems.insert({newPos.first, newPos.second});
    }
}

bool Map::canEnter(int direction, std::pair<int,int> pos){
    return canInsertItemAt(QPoint(pos.first, pos.second), direction);
}

void Map::updateAnimationFrame() {
    frameIndex = (frameIndex + 1) % 14;  // 循环播放动画
    for (int x = 0; x < tiles.size(); ++x) {
        for (int y = 0; y < tiles[x].size(); ++y) {
            Tile* tile = tiles[x][y];
            if (tile->type == Tile::Type::Belt && !tile->images.empty()) {
                if(frameIndex >= tile->images.size()){
                    qWarning() << "Missing belt animation frame";
                    continue;
                }
                tile->label->setPixmap(scaledPixmapForSize(tile->images[frameIndex], cellPixelSize(tile->size)));  // 更新 QLabel 的图像
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
                    continue;
                }

                QString minename = tiles[pos.first][pos.second]->mine->name;
                Item *minedItem = new Item(minename, generatePos);
                if(!tryInsertItemAt(QPoint(generatePos.first, generatePos.second), tiles[pos.first][pos.second]->direction, minedItem)){
                    delete minedItem;
                }
            }
        }
    }
}

void Map::cutterUpdate(){
    for (int x = 0; x < height; ++x) {
        for (int y = 0; y < width; ++y) {
            Tile *tile = tiles[x][y];
            if(tile->type != Tile::Type::Building || tile->father != nullptr || tile->name != "cutter" || tile->item == nullptr){
                continue;
            }

            std::pair<std::pair<int,int>,std::pair<int,int>> outPositions = cutterOutPox(x, y, *tile);
            const QPoint firstOut(outPositions.first.first, outPositions.first.second);
            const QPoint secondOut(outPositions.second.first, outPositions.second.second);
            if(!canInsertItemAt(firstOut, tile->direction) || !canInsertItemAt(secondOut, tile->direction)){
                continue;
            }

            const QPoint firstRoot = rootCellForCell(firstOut);
            const QPoint secondRoot = rootCellForCell(secondOut);
            if(firstRoot == secondRoot && inMap(firstRoot.x(), firstRoot.y())){
                Tile *sharedRoot = tiles[firstRoot.x()][firstRoot.y()];
                if(sharedRoot->type == Tile::Type::Building){
                    if(sharedRoot->name != "stacker"){
                        continue;
                    }
                    const int freeSlots = (sharedRoot->item == nullptr ? 1 : 0) + (sharedRoot->secondaryItem == nullptr ? 1 : 0);
                    if(freeSlots < 2){
                        continue;
                    }
                }
            }

            int strategy = (tile->direction == WEST || tile->direction == EAST) ? 0 : 1;
            if(!tile->item->isCuttable()){
                continue;
            }

            std::pair<Item*,Item*> items = tile->item->cutItem(strategy);
            if(tile->direction == SOUTH || tile->direction == WEST){
                std::swap(items.first, items.second);
            }

            const bool firstInserted = tryInsertItemAt(firstOut, tile->direction, items.first);
            const bool secondInserted = tryInsertItemAt(secondOut, tile->direction, items.second);
            if(firstInserted && secondInserted){
                delete tile->item;
                tile->item = nullptr;
            }else{
                if(!firstInserted){
                    delete items.first;
                }
                if(!secondInserted){
                    delete items.second;
                }
            }
        }
    }
}

void Map::rotaterUpdate(){
    for (int x = 0; x < height; ++x) {
        for (int y = 0; y < width; ++y) {
            Tile *tile = tiles[x][y];
            if(tile->type != Tile::Type::Building || tile->father != nullptr || tile->name != "rotater" || tile->item == nullptr){
                continue;
            }

            const QPoint outPos(nextPox(x, y, *tile).first, nextPox(x, y, *tile).second);
            Item *rotated = tile->item->rotateItem();
            if(tryInsertItemAt(outPos, tile->direction, rotated)){
                delete tile->item;
                tile->item = nullptr;
            }else{
                delete rotated;
            }
        }
    }
}

void Map::balancerUpdate(){
    for (int x = 0; x < height; ++x) {
        for (int y = 0; y < width; ++y) {
            Tile *tile = tiles[x][y];
            if(tile->type != Tile::Type::Building || tile->father != nullptr || tile->name != "balancer" || tile->item == nullptr){
                continue;
            }

            const QPoint root(x, y);
            const QPoint primary = primaryCellForWideBuilding(root, *tile);
            const QPoint alternate = alternateCellForWideBuilding(root, *tile);
            const QPoint output1(nextPox(primary.x(), primary.y(), *tile).first, nextPox(primary.x(), primary.y(), *tile).second);
            const QPoint output2(nextPox(alternate.x(), alternate.y(), *tile).first, nextPox(alternate.x(), alternate.y(), *tile).second);
            const bool preferFirst = (tile->state != "1");
            const QPoint preferred = preferFirst ? output1 : output2;
            const QPoint fallback = preferFirst ? output2 : output1;

            if(tryInsertItemAt(preferred, tile->direction, tile->item)){
                tile->item = nullptr;
                tile->state = preferFirst ? "1" : "0";
            }else if(tryInsertItemAt(fallback, tile->direction, tile->item)){
                tile->item = nullptr;
            }
        }
    }
}

void Map::stackerUpdate(){
    for (int x = 0; x < height; ++x) {
        for (int y = 0; y < width; ++y) {
            Tile *tile = tiles[x][y];
            if(tile->type != Tile::Type::Building || tile->father != nullptr || tile->name != "stacker"){
                continue;
            }
            if(tile->item == nullptr || tile->secondaryItem == nullptr){
                continue;
            }
            if(!tile->item->ableToConbine(*tile->secondaryItem)){
                continue;
            }

            Item *combined = new Item(*tile->item + *tile->secondaryItem);
            const QPoint root(x, y);
            const QPoint primary = primaryCellForWideBuilding(root, *tile);
            const QPoint outPos(nextPox(primary.x(), primary.y(), *tile).first, nextPox(primary.x(), primary.y(), *tile).second);
            if(tryInsertItemAt(outPos, tile->direction, combined)){
                delete tile->item;
                delete tile->secondaryItem;
                tile->item = nullptr;
                tile->secondaryItem = nullptr;
            }else{
                delete combined;
            }
        }
    }
}

void Map::undergroundBeltUpdate(){
    for (int x = 0; x < height; ++x) {
        for (int y = 0; y < width; ++y) {
            Tile* tile = tiles[x][y];
            if(tile->type != Tile::Type::Building || tile->father != nullptr || tile->name != "underground_belt_exit" || tile->item == nullptr){
                continue;
            }

            const QPoint outPos(nextPox(x, y, *tile).first, nextPox(x, y, *tile).second);
            if(tryInsertItemAt(outPos, tile->direction, tile->item)){
                tile->item = nullptr;
            }
        }
    }

    for (int x = 0; x < height; ++x) {
        for (int y = 0; y < width; ++y) {
            Tile* tile = tiles[x][y];
            if(tile->type != Tile::Type::Building || tile->father != nullptr || tile->name != "underground_belt_entry" || tile->item == nullptr){
                continue;
            }

            QPoint searchPos(x, y);
            for(int step = 1; step <= 4; ++step){
                std::pair<int,int> nextPos = nextPox(searchPos.x(), searchPos.y(), *tile);
                searchPos = QPoint(nextPos.first, nextPos.second);
                if(!inMap(searchPos.x(), searchPos.y())){
                    break;
                }

                Tile *candidate = tiles[searchPos.x()][searchPos.y()];
                if(candidate->type == Tile::Type::Building &&
                   candidate->name == "underground_belt_exit" &&
                   candidate->direction == tile->direction &&
                   candidate->item == nullptr){
                    candidate->item = tile->item;
                    tile->item = nullptr;
                    setBufferedItemPosition(candidate->item, searchPos);
                    break;
                }
            }
        }
    }
}

void Map::setItem(std::pair<int,int> pos, Item *item){
    tiles[pos.first][pos.second]->item = item;
    setBufferedItemPosition(item, QPoint(pos.first, pos.second));
}

void Map::setGoalShape(const std::array<int, 4> &parts){
    goalShape = parts;
}

void Map::itemToHub(int part1, int part2, int part3, int part4){
    int coins = 0;
    QVector<int> parts = {part1,part2,part3,part4};
    for(int part:parts){
        if(part == EMPTY){
            coins+=0;
        }
        if(part == SQUARE){
            coins+=1;
        }
        if(part == CIRCLE){
            coins+=1;
        }
        if(part == DIAMOND){
            coins+=1;
        }
    }
    ConfigManager config;
    config.addGold(coins);

    if(part1 == goalShape[0] &&
       part2 == goalShape[1] &&
       part3 == goalShape[2] &&
       part4 == goalShape[3]){
        current++;
    }
}

void Map::clearMap()
{
    miners.clear();
    clearBlueprint();
    hideBuildingInfo();

    for (int x = 0; x < tiles.size(); ++x) {
        for (int y = 0; y < tiles[x].size(); ++y) {
            delete tiles[x][y];
            tiles[x][y] = new Tile();
            tiles[x][y]->label = new QLabel(this);
            tiles[x][y]->label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
            updateTileLabel(x, y);
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

bool Map::inMap(int x,int y) const{
    if( x<0 || y<0 || x>=height || y>=width){
        return false;
    }else{
        return true;
    }
}

bool Map::inMap(std::pair<int,int> originaPos) const{
    int x = originaPos.first;
    int y = originaPos.second;

    if( x<0 || y<0 || x>=height || y>=width){
        return false;
    }else{
        return true;
    }
}

std::pair<std::pair<int,int>,std::pair<int,int>> Map::cutterOutPox(std::pair<int,int> pos,Tile &cutterTile){
    if(tiles[pos.first][pos.second]->type != Tile::Type::Building || tiles[pos.first][pos.second]->name != "cutter"){
        qDebug() << "not cutter";
        return std::make_pair(pos,pos);
    }
    const QPoint root(pos.first, pos.second);
    const QPoint secondary = secondaryCellForRoot(root, cutterTile);
    const QPoint primary = primaryCellForWideBuilding(root, cutterTile);
    const QPoint alternate = (primary == root) ? secondary : root;

    return std::make_pair(
        nextPox(std::make_pair(primary.x(), primary.y()), cutterTile),
        nextPox(std::make_pair(alternate.x(), alternate.y()), cutterTile));
}

std::pair<std::pair<int,int>,std::pair<int,int>> Map::cutterOutPox(int x, int y,Tile &cutterTile){
    std::pair<int,int> pos = std::make_pair(x,y);
    return cutterOutPox(pos, cutterTile);
}

void Map::mouseMoveEvent(QMouseEvent *event)
{
    updateBlueprintCursor(event->pos());
    updateHoverBuilding(event->pos());
    QWidget::mouseMoveEvent(event);
}

void Map::leaveEvent(QEvent *event)
{
    hideBuildingInfo();
    QWidget::leaveEvent(event);
}

void Map::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.fillRect(rect(), QColor("#ECEEF2"));

    const int tileSize = tilePixelSize();
    QPen pen(QColor("#E3E7EA"));
    pen.setWidth(1);
    painter.setPen(pen);

    for (int row = 0; row <= height; ++row) {
        const int y = row * tileSize;
        painter.drawLine(0, y, width * tileSize, y);
    }

    for (int column = 0; column <= width; ++column) {
        const int x = column * tileSize;
        painter.drawLine(x, 0, x, height * tileSize);
    }

    // Draw underground belt connection preview line
    if (undergroundPreviewStart.x() >= 0 && undergroundPreviewEnd.x() >= 0) {
        const int half = tileSize / 2;
        QPoint startPx(undergroundPreviewStart.y() * tileSize + half,
                       undergroundPreviewStart.x() * tileSize + half);
        QPoint endPx(undergroundPreviewEnd.y() * tileSize + half,
                     undergroundPreviewEnd.x() * tileSize + half);
        QPen dashPen(QColor(236, 238, 242));
        dashPen.setWidth(3);
        dashPen.setStyle(Qt::DashLine);
        painter.setPen(dashPen);
        painter.drawLine(startPx, endPx);
    }
}

void Map::setZoomFactor(double zoomFactor)
{
    currentZoomFactor = qBound(0.5, zoomFactor, 2.5);
    updateLayout();
}

double Map::zoomFactor() const
{
    return currentZoomFactor;
}

int Map::tilePixelSize() const
{
    return qMax(20, qRound(TILESIZE * currentZoomFactor));
}

QPoint Map::gridPositionFromPoint(const QPoint &point) const
{
    const int tileSize = tilePixelSize();
    return QPoint(point.y() / tileSize, point.x() / tileSize);
}

bool Map::canPlaceTile(int x, int y, const Tile &tile) const
{
    if (x < 0 || y < 0) {
        return false;
    }

    if (x + tile.size.first > height || y + tile.size.second > width) {
        return false;
    }

    if (tile.type == Tile::Type::Building && tile.name == "miner") {
        return tiles[x][y]->type == Tile::Type::Resource;
    }

    for (int row = 0; row < tile.size.first; ++row) {
        for (int column = 0; column < tile.size.second; ++column) {
            if (tiles[x + row][y + column]->type != Tile::Type::Empty) {
                return false;
            }
        }
    }

    return true;
}

void Map::setBlueprintTile(const Tile *tile)
{
    if (!tile) {
        clearBlueprint();
        return;
    }

    applyBlueprintPixmap(*tile);
    updateBlueprintCursor(lastBlueprintCursorPos);
    blueprintLabel->show();
    blueprintLabel->raise();
}

void Map::clearBlueprint()
{
    blueprintLabel->hide();
    blueprintLabel->clear();
}

void Map::updateBlueprintCursor(const QPoint &mapPos)
{
    lastBlueprintCursorPos = mapPos;

    if (blueprintLabel->pixmap().isNull()) {
        return;
    }

    const QPoint imageCenterOffset(blueprintLabel->width() / 2, blueprintLabel->height() / 2);
    blueprintLabel->move(mapPos - imageCenterOffset);
    blueprintLabel->raise();
}

void Map::updateLayout()
{
    setFixedSize(width * tilePixelSize(), height * tilePixelSize());

    for (int x = 0; x < tiles.size(); ++x) {
        for (int y = 0; y < tiles[x].size(); ++y) {
            updateTileLabel(x, y);
            if (tiles[x][y]->item != nullptr) {
                updateItemLabel(std::make_pair(x, y));
            }
        }
    }

    updateHudGeometry();
    questionLabel->raise();
    countLabel->raise();
    levelLabel->raise();
    blueprintLabel->raise();
    if (buildingInfoLabel->isVisible()) {
        showBuildingInfo();
    }
    buildingInfoLabel->raise();
    updateBlueprintCursor(lastBlueprintCursorPos);
    update();
}

QSize Map::cellPixelSize(std::pair<int, int> cellSpan) const
{
    return QSize(cellSpan.second * tilePixelSize(), cellSpan.first * tilePixelSize());
}

QRect Map::tileGeometry(int x, int y, std::pair<int, int> cellSpan) const
{
    const QSize size = cellPixelSize(cellSpan);
    return QRect(y * tilePixelSize(), x * tilePixelSize(), size.width(), size.height());
}

void Map::updateTileLabel(int x, int y)
{
    Tile *tile = tiles[x][y];
    if (!tile || !tile->label) {
        return;
    }

    tile->label->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    if (tile->type == Tile::Type::Empty || tile->father != nullptr) {
        tile->label->clear();
        tile->label->hide();
        return;
    }

    tile->label->setGeometry(tileGeometry(x, y, tile->size));
    if (tile->type == Tile::Type::Belt && !tile->images.empty()) {
        const int safeFrameIndex = qMin(frameIndex, tile->images.size() - 1);
        tile->label->setPixmap(scaledPixmapForSize(tile->images[safeFrameIndex], cellPixelSize(tile->size)));
    } else {
        QPixmap displayPixmap = scaledPixmapForSize(tile->image, cellPixelSize(tile->size));
        if (tile->name == "miner" && tile->mine != nullptr) {
            QPixmap composite(cellPixelSize(tile->size));
            composite.fill(Qt::transparent);
            QPainter painter(&composite);
            painter.drawPixmap(0, 0, scaledPixmapForSize(tile->mine->image, cellPixelSize(tile->size)));
            painter.drawPixmap(0, 0, displayPixmap);
            displayPixmap = composite;
        }
        tile->label->setPixmap(displayPixmap);
    }
    tile->label->show();
    tile->label->raise();

    // Keep buffered items visually inside buildings instead of floating above them.
    if (tile->type == Tile::Type::Building && tile->item && tile->item->label) {
        tile->item->label->stackUnder(tile->label);
    }
}

void Map::updateItemLabel(const std::pair<int, int> &pos)
{
    Item *item = tiles[pos.first][pos.second]->item;
    if (!item || !item->label) {
        return;
    }

    item->label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    item->label->setGeometry(tileGeometry(pos.first, pos.second));
    item->label->setPixmap(scaledPixmapForSize(item->getPixmap(), cellPixelSize()));
    item->label->show();

    Tile *tile = tiles[pos.first][pos.second];
    if (tile->type == Tile::Type::Building && tile->label) {
        item->label->stackUnder(tile->label);
    } else {
        item->label->raise();
    }
}

void Map::updateHudGeometry()
{
    const int tileSize = tilePixelSize();
    updateHudFonts();
    questionLabel->setGeometry(
        hudAnchorColumn * tileSize - qRound(tileSize * 0.6),
        hudAnchorRow * tileSize,
        2 * tileSize,
        2 * tileSize
    );
    countLabel->setGeometry(
        hudAnchorColumn * tileSize + qRound(tileSize * 1.2),
        hudAnchorRow * tileSize,
        3 * tileSize,
        2 * tileSize
    );
    levelLabel->setGeometry(
        hudAnchorColumn * tileSize - qRound(tileSize * 0.3),
        (hudAnchorRow - 1) * tileSize + qRound(tileSize * 0.3),
        tileSize,
        tileSize
    );
}

void Map::updateHudFonts()
{
    QFont countFont = countLabel->font();
    countFont.setPixelSize(qMax(12, qRound(tilePixelSize() * 0.56)));
    countLabel->setFont(countFont);

    QFont levelFont = levelLabel->font();
    levelFont.setPixelSize(qMax(10, qRound(tilePixelSize() * 0.36)));
    levelLabel->setFont(levelFont);
}

void Map::updateHoverBuilding(const QPoint &mapPos)
{
    lastHoverCursorPos = mapPos;

    if (buildingInfoLabel->isVisible()) {
        updateBuildingInfoPosition(mapPos);
    }

    const QPoint hoveredCell = gridPositionFromPoint(mapPos);
    if (!inMap(hoveredCell.x(), hoveredCell.y())) {
        hideBuildingInfo();
        return;
    }

    const QPoint rootCell = rootCellForCell(hoveredCell);
    if (!inMap(rootCell.x(), rootCell.y())) {
        hideBuildingInfo();
        return;
    }

    const Tile hoveredTile = getTile(rootCell.x(), rootCell.y());
    if (!isInspectableTile(hoveredTile)) {
        hideBuildingInfo();
        return;
    }

    if (rootCell == hoveredBuildingRootCell) {
        return;
    }

    hoveredBuildingRootCell = rootCell;
    buildingInfoLabel->hide();
    buildingInfoTimer->start(2000);
}

void Map::showBuildingInfo()
{
    if (!inMap(hoveredBuildingRootCell.x(), hoveredBuildingRootCell.y())) {
        hideBuildingInfo();
        return;
    }

    const Tile hoveredTile = getTile(hoveredBuildingRootCell.x(), hoveredBuildingRootCell.y());
    const QString infoText = buildingInfoText(hoveredTile);
    if (infoText.isEmpty()) {
        hideBuildingInfo();
        return;
    }

    buildingInfoLabel->setFixedWidth(qMax(220, qRound(tilePixelSize() * 5.6)));
    buildingInfoLabel->setText(infoText);
    buildingInfoLabel->adjustSize();
    buildingInfoLabel->resize(buildingInfoLabel->width(), buildingInfoLabel->sizeHint().height());
    updateBuildingInfoPosition(lastHoverCursorPos);
    buildingInfoLabel->show();
    buildingInfoLabel->raise();
}

void Map::updateBuildingInfoPosition(const QPoint &mapPos)
{
    if (!buildingInfoLabel) {
        return;
    }

    const QPoint belowCursorOffset(18, 20);
    const QPoint aboveCursorOffset(18, -(buildingInfoLabel->height() + 20));
    QPoint desiredPosition = mapPos + belowCursorOffset;
    const QRect bounds = rect().adjusted(12, 12, -12, -12);

    if (mapPos.y() > rect().center().y() ||
        desiredPosition.y() + buildingInfoLabel->height() > bounds.bottom()) {
        desiredPosition = mapPos + aboveCursorOffset;
    }

    if (desiredPosition.x() + buildingInfoLabel->width() > bounds.right()) {
        desiredPosition.setX(bounds.right() - buildingInfoLabel->width());
    }
    if (desiredPosition.y() + buildingInfoLabel->height() > bounds.bottom()) {
        desiredPosition.setY(bounds.bottom() - buildingInfoLabel->height());
    }

    desiredPosition.setX(qMax(bounds.left(), desiredPosition.x()));
    desiredPosition.setY(qMax(bounds.top(), desiredPosition.y()));
    buildingInfoLabel->move(desiredPosition);
}

QPoint Map::rootCellForCell(const QPoint &cell) const
{
    if (!inMap(cell.x(), cell.y())) {
        return QPoint(-1, -1);
    }

    const Tile *tile = tiles[cell.x()][cell.y()];
    if (tile && tile->father != nullptr) {
        return QPoint(tile->father->first, tile->father->second);
    }

    return cell;
}

QPoint Map::secondaryCellForRoot(const QPoint &root, const Tile &tile) const
{
    if (tile.size == std::make_pair(1, 1)) {
        return root;
    }

    if (tile.direction == NORTH || tile.direction == SOUTH) {
        return QPoint(root.x(), root.y() + 1);
    }

    return QPoint(root.x() + 1, root.y());
}

QPoint Map::primaryCellForWideBuilding(const QPoint &root, const Tile &tile) const
{
    if (tile.size == std::make_pair(1, 1)) {
        return root;
    }

    return (tile.direction == NORTH || tile.direction == EAST)
        ? root
        : secondaryCellForRoot(root, tile);
}

QPoint Map::alternateCellForWideBuilding(const QPoint &root, const Tile &tile) const
{
    if (tile.size == std::make_pair(1, 1)) {
        return root;
    }

    const QPoint primary = primaryCellForWideBuilding(root, tile);
    return primary == root ? secondaryCellForRoot(root, tile) : root;
}

bool Map::isPrimaryInputCellForWideBuilding(const QPoint &destinationCell, const QPoint &root, const Tile &tile) const
{
    return destinationCell == primaryCellForWideBuilding(root, tile);
}

QPoint Map::primaryInputCellForSingleInputWideBuilding(const QPoint &root, const Tile &tile) const
{
    return primaryCellForWideBuilding(root, tile);
}

bool Map::canInsertItemAt(const QPoint &destinationCell, int direction) const
{
    if (!inMap(destinationCell.x(), destinationCell.y())) {
        return false;
    }

    const Tile *tile = tiles[destinationCell.x()][destinationCell.y()];
    if (tile->type == Tile::Type::Empty || tile->type == Tile::Type::Color || tile->type == Tile::Type::Resource) {
        return false;
    }

    if (tile->type == Tile::Type::Belt) {
        return tile->item == nullptr && tile->direction == direction;
    }

    if (tile->type == Tile::Type::Hub) {
        return true;
    }

    const QPoint rootPos = rootCellForCell(destinationCell);
    const Tile *rootTile = tiles[rootPos.x()][rootPos.y()];
    const QPoint secondaryCell = secondaryCellForRoot(rootPos, *rootTile);

    if (rootTile->name == "trash") {
        return true;
    }

    if (direction != rootTile->direction) {
        return false;
    }

    if (rootTile->name == "cutter") {
        return rootTile->item == nullptr && destinationCell == primaryInputCellForSingleInputWideBuilding(rootPos, *rootTile);
    }

    if (rootTile->name == "rotater") {
        return rootTile->item == nullptr && destinationCell == rootPos;
    }

    if (rootTile->name == "balancer") {
        return rootTile->item == nullptr && (destinationCell == rootPos || destinationCell == secondaryCell);
    }

    if (rootTile->name == "underground_belt_entry") {
        return rootTile->item == nullptr && destinationCell == rootPos;
    }

    if (rootTile->name == "stacker") {
        if (destinationCell != rootPos && destinationCell != secondaryCell) {
            return false;
        }
        return isPrimaryInputCellForWideBuilding(destinationCell, rootPos, *rootTile)
            ? rootTile->item == nullptr
            : rootTile->secondaryItem == nullptr;
    }

    return false;
}

bool Map::tryInsertItemAt(const QPoint &destinationCell, int direction, Item *item)
{
    if (!item || !canInsertItemAt(destinationCell, direction)) {
        return false;
    }

    Tile *tile = tiles[destinationCell.x()][destinationCell.y()];
    if (tile->type == Tile::Type::Belt) {
        tile->item = item;
        setBufferedItemPosition(item, destinationCell);
        return true;
    }

    if (tile->type == Tile::Type::Hub) {
        itemToHub(item->part1, item->part2, item->part3, item->part4);
        delete item;
        return true;
    }

    QPoint rootPos = rootCellForCell(destinationCell);
    Tile *rootTile = tiles[rootPos.x()][rootPos.y()];

    if (rootTile->name == "trash") {
        delete item;
        return true;
    }

    if (rootTile->name == "stacker") {
        if (isPrimaryInputCellForWideBuilding(destinationCell, rootPos, *rootTile)) {
            rootTile->item = item;
            setBufferedItemPosition(item, rootPos);
        } else {
            rootTile->secondaryItem = item;
            item->pos = std::make_pair(rootPos.x(), rootPos.y());
            if (item->label) {
                item->label->hide();
            }
        }
        return true;
    }

    rootTile->item = item;
    setBufferedItemPosition(item, rootPos);
    return true;
}

void Map::setBufferedItemPosition(Item *item, const QPoint &cell)
{
    if (!item) {
        return;
    }

    item->pos = std::make_pair(cell.x(), cell.y());
    if (!item->label) {
        item->label = new QLabel(this);
        item->label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    }
    updateItemLabel(item->pos);
}

QString Map::buildingInfoText(const Tile &tile) const
{
    const BuildingInfoEntry entry = buildingInfoEntryForTile(tile);
    if (entry.enName.isEmpty()) {
        return {};
    }

    ConfigManager config;
    const QString languageCode = config.getLanguage();
    const QString localizedName = Localization::text(languageCode, entry.zhName, entry.enName).toHtmlEscaped();
    const QString localizedDescription = Localization::text(languageCode, entry.zhDescription, entry.enDescription);

    // Build speed info line
    QString speedInfo;
    if (tile.type == Tile::Type::Belt) {
        double ips = 1000.0 / currentBeltIntervalMs;
        speedInfo = Localization::text(languageCode,
            QString("速度: %1 格/秒").arg(ips, 0, 'f', 2),
            QString("Speed: %1 tiles/s").arg(ips, 0, 'f', 2));
    } else if (tile.name == "balancer") {
        double ips = 1000.0 / currentBalancerIntervalMs;
        speedInfo = Localization::text(languageCode,
            QString("速度: %1 项/秒").arg(ips, 0, 'f', 2),
            QString("Speed: %1 items/s").arg(ips, 0, 'f', 2));
    } else if (tile.name == "underground_belt_entry" || tile.name == "underground_belt_exit") {
        double ips = 1000.0 / currentUndergroundIntervalMs;
        speedInfo = Localization::text(languageCode,
            QString("速度: %1 项/秒").arg(ips, 0, 'f', 2),
            QString("Speed: %1 items/s").arg(ips, 0, 'f', 2));
    } else if (tile.name == "miner") {
        double ips = 1000.0 / currentMinerIntervalMs;
        speedInfo = Localization::text(languageCode,
            QString("产量: %1 个/秒").arg(ips, 0, 'f', 2),
            QString("Output: %1 items/s").arg(ips, 0, 'f', 2));
    } else if (tile.name == "cutter") {
        double ips = 1000.0 / currentCutterIntervalMs;
        speedInfo = Localization::text(languageCode,
            QString("速度: %1 个/秒").arg(ips, 0, 'f', 2),
            QString("Speed: %1 items/s").arg(ips, 0, 'f', 2));
    } else if (tile.name == "rotater") {
        double ips = 1000.0 / currentRotaterIntervalMs;
        speedInfo = Localization::text(languageCode,
            QString("速度: %1 个/秒").arg(ips, 0, 'f', 2),
            QString("Speed: %1 items/s").arg(ips, 0, 'f', 2));
    } else if (tile.name == "stacker") {
        double ips = 1000.0 / currentStackerIntervalMs;
        speedInfo = Localization::text(languageCode,
            QString("速度: %1 个/秒").arg(ips, 0, 'f', 2),
            QString("Speed: %1 items/s").arg(ips, 0, 'f', 2));
    }

    QString result = QString("<b>%1</b><br/>%2").arg(localizedName, localizedDescription);
    if (!speedInfo.isEmpty()) {
        result += QString("<br/><span style='color:#555;'>%1</span>").arg(speedInfo);
    }
    return result;
}

void Map::hideBuildingInfo()
{
    hoveredBuildingRootCell = QPoint(-1, -1);
    if (buildingInfoTimer) {
        buildingInfoTimer->stop();
    }
    if (buildingInfoLabel) {
        buildingInfoLabel->hide();
        buildingInfoLabel->clear();
    }
}

QPixmap Map::scaledPixmapForSize(const QPixmap &pixmap, const QSize &targetSize) const
{
    if (pixmap.isNull() || targetSize.isEmpty()) {
        return pixmap;
    }

    return pixmap.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QPixmap Map::blueprintPixmapForTile(const Tile &tile) const
{
    QPixmap pixmap;
    QString assetName = tile.name;

    if (tile.type == Tile::Type::Belt) {
        if (tile.state == "left") {
            assetName = "belt_left";
        } else if (tile.state == "right") {
            assetName = "belt_right";
        } else {
            assetName = "belt_top";
        }
    }

    if (!pixmap.load(QString(":/res/blueprints/%1.png").arg(assetName))) {
        qWarning() << "Failed to load blueprint image:" << assetName;
        return {};
    }

    QTransform transform;
    int angle = 0;
    switch (tile.direction) {
    case NORTH:
        angle = 0;
        break;
    case EAST:
        angle = 90;
        break;
    case SOUTH:
        angle = 180;
        break;
    case WEST:
        angle = 270;
        break;
    default:
        break;
    }
    transform.rotate(angle);

    return pixmap.transformed(transform, Qt::SmoothTransformation);
}

void Map::applyBlueprintPixmap(const Tile &tile)
{
    const QPixmap pixmap = blueprintPixmapForTile(tile);
    const QSize targetSize = cellPixelSize(tile.size);
    blueprintLabel->setPixmap(scaledPixmapForSize(pixmap, targetSize));
    blueprintLabel->resize(targetSize);
}
