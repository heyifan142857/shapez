#include <QDebug>
#include <QMutableListIterator>
#include <QMouseEvent>
#include "map.h"
#include "configmanager.h"
#include "globalupgradedialog.h"
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
            "把输入流平均分配，适合整理产线吞吐。",
            "Splits incoming flow evenly to keep your production lines balanced."
        };
    }

    if (tile.name == "underground_belt_entry") {
        return {
            "地下传送带",
            "Underground Belt",
            "让物品从地下穿过拥挤区域，便于跨越其他线路。",
            "Sends items underground so they can cross busy factory lines."
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
            "将输入图形旋转后输出，用来调整拼装方向。",
            "Rotates incoming shapes before output so you can align later processing."
        };
    }

    if (tile.name == "stacker") {
        return {
            "堆叠器",
            "Stacker",
            "把两路输入组合成堆叠图形，适合更高阶产物。",
            "Combines two inputs into a stacked shape for more advanced recipes."
        };
    }

    if (tile.name == "mixer") {
        return {
            "混合器",
            "Mixer",
            "将多路输入整合到同一产线，便于后续加工。",
            "Merges multiple inputs into one combined line for later processing."
        };
    }

    if (tile.name == "painter") {
        return {
            "染色器",
            "Painter",
            "给输入图形上色，让产线能够制作彩色目标。",
            "Applies color to incoming shapes so you can produce painted targets."
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
    //Tile nextTile = getTile(newPos);
    if(!inMap(newPos) || tiles[newPos.first][newPos.second]->type == Tile::Type::Color || tiles[newPos.first][newPos.second]->type == Tile::Type::Empty || tiles[newPos.first][newPos.second]->type == Tile::Type::Resource){
        return;
    }
    // if(tiles[newPos.first][newPos.second]->father != nullptr && tiles[newPos.first][newPos.second]->type!=Tile::Type::Hub && tiles[newPos.first][newPos.second]->name != "cutter"){
    //     return;
    // }
    if(tiles[newPos.first][newPos.second]->type == Tile::Type::Belt){
        if(tiles[newPos.first][newPos.second]->item != nullptr){
            return;
        }
        int realDirection = tiles[x][y]->direction;
        if(tiles[x][y]->type == Tile::Type::Belt){
            if(tiles[x][y]->state == "left"){
                realDirection = (realDirection+3)%4;
            }
            if(tiles[x][y]->state == "right"){
                realDirection = (realDirection+1)%4;
            }
        }
        if(realDirection != tiles[newPos.first][newPos.second]->direction){
            return;
        }
        Item *item = tiles[x][y]->item;
        tiles[item->pos.first][item->pos.second]->item = nullptr;
        item->pos = newPos;
        tiles[newPos.first][newPos.second]->item = item;
        updateItemLabel(newPos);
        movedItems.insert({newPos.first,newPos.second});
    }else if(tiles[newPos.first][newPos.second]->type == Tile::Type::Building){
        if(tiles[newPos.first][newPos.second]->name == "cutter"){
            if(tiles[newPos.first][newPos.second]->direction == NORTH || tiles[newPos.first][newPos.second]->direction == EAST){
                if(tiles[newPos.first][newPos.second]->father!=nullptr){
                    return;
                }
            }
            if(tiles[newPos.first][newPos.second]->direction == SOUTH || tiles[newPos.first][newPos.second]->direction == WEST){
                if(tiles[newPos.first][newPos.second]->father==nullptr){
                    return;
                }
            }
            int realDirection = tiles[x][y]->direction;
            if(tiles[x][y]->type == Tile::Type::Belt){
                if(tiles[x][y]->state == "left"){
                    realDirection = (realDirection+3)%4;
                }
                if(tiles[x][y]->state == "right"){
                    realDirection = (realDirection+1)%4;
                }
            }
            if(realDirection != tiles[newPos.first][newPos.second]->direction){
                return;
            }else{
                std::pair<std::pair<int,int>,std::pair<int,int>> outPos = cutterOutPox(newPos.first,newPos.second,*tiles[newPos.first][newPos.second]);
                if(canEnter(tiles[newPos.first][newPos.second]->direction, outPos.first)&&canEnter(tiles[newPos.first][newPos.second]->direction, outPos.second)){
                    int stragedy = 0;
                    if(realDirection == WEST || realDirection == EAST){
                        stragedy = 0;
                    }else{
                        stragedy = 1;
                    }
                    ConfigManager config;
                    if(!config.getUpgradeStatus("cut")){
                        if(!tiles[x][y]->item->isCuttable()){
                            return;
                        }
                    }

                    std::pair<Item*,Item*> items = tiles[x][y]->item->cutItem(stragedy);
                    if(realDirection == SOUTH || realDirection == WEST){
                        std::swap(items.first,items.second);
                    }
                    std::pair<std::pair<int,int>,std::pair<int,int>> outItemPos = cutterOutPox(newPos.first,newPos.second,*tiles[newPos.first][newPos.second]);

                    if(tiles[outItemPos.first.first][outItemPos.first.second]->item!=nullptr || tiles[outItemPos.second.first][outItemPos.second.second]->item!=nullptr){
                        return;
                    }

                    delete tiles[x][y]->item;
                    tiles[x][y]->item = nullptr;

                    tiles[outItemPos.first.first][outItemPos.first.second]->item = items.first;
                    tiles[outItemPos.first.first][outItemPos.first.second]->item->pos = outItemPos.first;
                    tiles[outItemPos.first.first][outItemPos.first.second]->item->label = new QLabel(this);
                    tiles[outItemPos.first.first][outItemPos.first.second]->item->label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
                    updateItemLabel(outItemPos.first);

                    tiles[outItemPos.second.first][outItemPos.second.second]->item = items.second;
                    tiles[outItemPos.second.first][outItemPos.second.second]->item->pos = outItemPos.second;
                    tiles[outItemPos.second.first][outItemPos.second.second]->item->label = new QLabel(this);
                    tiles[outItemPos.second.first][outItemPos.second.second]->item->label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
                    updateItemLabel(outItemPos.second);
                    movedItems.insert({outItemPos.first.first,outItemPos.first.second});
                    movedItems.insert({outItemPos.second.first,outItemPos.second.second});
                }else{
                    return;
                }
            }
            //todo
        }else{
            Item *tempItem = tiles[x][y]->item;
            tiles[x][y]->item = nullptr;
            delete tempItem;
        }
    }else if(tiles[newPos.first][newPos.second]->type == Tile::Type::Hub){
        Item *tempItem = tiles[x][y]->item;
        itemToHub(tempItem->part1,tempItem->part2,tempItem->part3,tempItem->part4);
        tiles[x][y]->item = nullptr;
        delete tempItem;
        //todo
    }
}

bool Map::canEnter(int direction, std::pair<int,int> pos){
    if(!inMap(pos) || tiles[pos.first][pos.second]->type == Tile::Type::Color || tiles[pos.first][pos.second]->type == Tile::Type::Empty || tiles[pos.first][pos.second]->type == Tile::Type::Resource){
        return false;
    }
    if(tiles[pos.first][pos.second]->type == Tile::Type::Belt && direction != tiles[pos.first][pos.second]->direction && !(tiles[pos.first][pos.second]->type==Tile::Type::Hub)){
        return false;
    }
    if(tiles[pos.first][pos.second]->type==Tile::Type::Building){
        if(tiles[pos.first][pos.second]->name == "cutter"){
            if(direction != tiles[pos.first][pos.second]->direction && !(tiles[pos.first][pos.second]->type==Tile::Type::Hub)){
                return false;
            }
            if(tiles[pos.first][pos.second]->father != nullptr){
                return false;
            }
        }
    }
    if(tiles[pos.first][pos.second]->item != nullptr){
        return false;
    }
    return true;
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
                    return;
                }
                if(tiles[generatePos.first][generatePos.second]->type == Tile::Type::Belt && tiles[generatePos.first][generatePos.second]->direction==tiles[pos.first][pos.second]->direction){
                    if(tiles[generatePos.first][generatePos.second]->item == nullptr){
                        QString minename = tiles[pos.first][pos.second]->mine->name;
                        tiles[generatePos.first][generatePos.second]->item = new Item(minename,generatePos);
                        tiles[generatePos.first][generatePos.second]->item->label = new QLabel(this);
                        tiles[generatePos.first][generatePos.second]->item->label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
                        updateItemLabel(generatePos);
                        //items.append();
                        //tiles[generatePos.first][generatePos.second]->item->label->setAttribute(Qt::WA_AlwaysStackOnTop, true);;
                    }
                }
                if(tiles[generatePos.first][generatePos.second]->type == Tile::Type::Building
                    && tiles[generatePos.first][generatePos.second]->name == "cutter"){
                    Tile *cutterTile = tiles[generatePos.first][generatePos.second];
                    int realDirection = tiles[pos.first][pos.second]->direction;

                    if(cutterTile->direction != realDirection){
                        continue;
                    }
                    if((cutterTile->direction == NORTH || cutterTile->direction == EAST) && cutterTile->father != nullptr){
                        continue;
                    }
                    if((cutterTile->direction == SOUTH || cutterTile->direction == WEST) && cutterTile->father == nullptr){
                        continue;
                    }

                    std::pair<std::pair<int,int>,std::pair<int,int>> outPos = cutterOutPox(generatePos.first,generatePos.second,*cutterTile);
                    if(!canEnter(cutterTile->direction, outPos.first) || !canEnter(cutterTile->direction, outPos.second)){
                        continue;
                    }
                    if(tiles[outPos.first.first][outPos.first.second]->item != nullptr
                        || tiles[outPos.second.first][outPos.second.second]->item != nullptr){
                        continue;
                    }

                    QString minename = tiles[pos.first][pos.second]->mine->name;
                    Item *minedItem = new Item(minename, generatePos);
                    ConfigManager config;
                    if(!config.getUpgradeStatus("cut") && !minedItem->isCuttable()){
                        delete minedItem;
                        continue;
                    }

                    int stragedy = (realDirection == WEST || realDirection == EAST) ? 0 : 1;
                    std::pair<Item*,Item*> items = minedItem->cutItem(stragedy);
                    delete minedItem;

                    if(realDirection == SOUTH || realDirection == WEST){
                        std::swap(items.first,items.second);
                    }

                    tiles[outPos.first.first][outPos.first.second]->item = items.first;
                    tiles[outPos.first.first][outPos.first.second]->item->pos = outPos.first;
                    tiles[outPos.first.first][outPos.first.second]->item->label = new QLabel(this);
                    tiles[outPos.first.first][outPos.first.second]->item->label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
                    updateItemLabel(outPos.first);

                    tiles[outPos.second.first][outPos.second.second]->item = items.second;
                    tiles[outPos.second.first][outPos.second.second]->item->pos = outPos.second;
                    tiles[outPos.second.first][outPos.second.second]->item->label = new QLabel(this);
                    tiles[outPos.second.first][outPos.second.second]->item->label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
                    updateItemLabel(outPos.second);
                }
            }
        }
    }
}

void Map::cutterUpdate(){
    for (int x = 0; x < height; ++x) {
        for (int y = 0; y < width; ++y) {
            if(tiles[x][y]->type == Tile::Type::Building && tiles[x][y]->name == "cutter" && tiles[x][y]->item != nullptr){
                std::pair<int,int> newPos = nextPox(x,y,*tiles[x][y]);
                //Tile nextTile = getTile(newPos);
                if(!inMap(newPos) || tiles[newPos.first][newPos.second]->type == Tile::Type::Color || tiles[newPos.first][newPos.second]->type == Tile::Type::Empty || tiles[newPos.first][newPos.second]->type == Tile::Type::Resource){
                    continue;
                }
                if(tiles[newPos.first][newPos.second]->father != nullptr && tiles[newPos.first][newPos.second]->type!=Tile::Type::Hub){
                    continue;
                }
                if(tiles[newPos.first][newPos.second]->type == Tile::Type::Belt){
                    if(tiles[newPos.first][newPos.second]->item != nullptr){
                        continue;
                    }
                    int realDirection = tiles[x][y]->direction;
                    if(tiles[x][y]->type == Tile::Type::Belt){
                        if(tiles[x][y]->state == "left"){
                            realDirection = (realDirection+3)%4;
                        }
                        if(tiles[x][y]->state == "right"){
                            realDirection = (realDirection+1)%4;
                        }
                    }
                    if(realDirection != tiles[newPos.first][newPos.second]->direction){
                        continue;
                    }
                    Item *item = tiles[x][y]->item;
                    tiles[item->pos.first][item->pos.second]->item = nullptr;
                    item->pos = newPos;
                    tiles[newPos.first][newPos.second]->item = item;
                    updateItemLabel(newPos);
                }else if(tiles[newPos.first][newPos.second]->type == Tile::Type::Building){
                    if(tiles[newPos.first][newPos.second]->name == "cutter"){
                        int realDirection = tiles[x][y]->direction;
                        if(tiles[x][y]->type == Tile::Type::Belt){
                            if(tiles[x][y]->state == "left"){
                                realDirection = (realDirection+3)%4;
                            }
                            if(tiles[x][y]->state == "right"){
                                realDirection = (realDirection+1)%4;
                            }
                        }
                        if(realDirection != tiles[newPos.first][newPos.second]->direction){
                            continue;
                        }else{
                            std::pair<std::pair<int,int>,std::pair<int,int>> outPos = cutterOutPox(newPos.first,newPos.second,*tiles[newPos.first][newPos.second]);
                            if(canEnter(tiles[newPos.first][newPos.second]->direction, outPos.first)&&canEnter(tiles[newPos.first][newPos.second]->direction, outPos.second)){
                                int stragedy = 0;
                                if(tiles[x][y]->direction == WEST || tiles[x][y]->direction == EAST){
                                    stragedy = 0;
                                }else{
                                    stragedy = 1;
                                }
                                if(!tiles[x][y]->item->isCuttable()){
                                    continue;
                                }
                                std::pair<Item*,Item*> items = tiles[x][y]->item->cutItem(stragedy);
                                if(tiles[x][y]->direction == SOUTH || tiles[x][y]->direction == WEST){
                                    std::swap(items.first,items.second);
                                }
                                std::pair<std::pair<int,int>,std::pair<int,int>> outItemPos = cutterOutPox(x,y,*tiles[x][y]);

                                if(tiles[outItemPos.first.first][outItemPos.first.second]->item!=nullptr || tiles[outItemPos.second.first][outItemPos.second.second]->item!=nullptr){
                                    continue;
                                }

                                delete tiles[x][y]->item;
                                tiles[x][y]->item = nullptr;

                                tiles[outItemPos.first.first][outItemPos.first.second]->item = items.first;
                                tiles[outItemPos.first.first][outItemPos.first.second]->item->pos = outItemPos.first;
                                tiles[outItemPos.first.first][outItemPos.first.second]->item->label = new QLabel(this);
                                tiles[outItemPos.first.first][outItemPos.first.second]->item->label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
                                updateItemLabel(outItemPos.first);

                                tiles[outItemPos.second.first][outItemPos.second.second]->item = items.second;
                                tiles[outItemPos.second.first][outItemPos.second.second]->item->pos = outItemPos.second;
                                tiles[outItemPos.second.first][outItemPos.second.second]->item->label = new QLabel(this);
                                tiles[outItemPos.second.first][outItemPos.second.second]->item->label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
                                updateItemLabel(outItemPos.second);
                            }else{
                                continue;
                            }
                        }
                        //todo
                    }else{
                        Item *tempItem = tiles[x][y]->item;
                        tiles[x][y]->item = nullptr;
                        delete tempItem;
                    }
                }else if(tiles[newPos.first][newPos.second]->type == Tile::Type::Hub){
                    Item *tempItem = tiles[x][y]->item;
                    itemToHub(tempItem->part1,tempItem->part2,tempItem->part3,tempItem->part4);
                    tiles[x][y]->item = nullptr;
                    delete tempItem;
                    //todo
                }
            }
        }
    }
}


void Map::setItem(std::pair<int,int> pos, Item *item){
    tiles[pos.first][pos.second]->item = item;
    tiles[pos.first][pos.second]->item->label = new QLabel(this);
    tiles[pos.first][pos.second]->item->label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    updateItemLabel(pos);
}

void Map::itemToHub(int part1, int part2, int part3, int part4){
    int coins = 0;
    QVector<int> parts = {part1,part2,part3,part4};
    for(int part:parts){
        if(part == EMPTY){
            coins+=3;
        }
        if(part == SQUARE){
            coins+=2;
        }
        if(part == CIRCLE){
            coins+=1;
        }
        if(part == DIAMOND){
            coins+=3;
        }
    }
    ConfigManager config;
    if(config.getUpgradeStatus("mine")){
        coins *= 2;
    }
    config.addGold(coins);

    if(questionLever == 0){
        if(part1==CIRCLE && part2==CIRCLE && part3==CIRCLE && part4==CIRCLE){
            current++;
        }
    }else if(questionLever == 1){
        if(part1==SQUARE && part2==SQUARE && part3==SQUARE && part4==SQUARE){
            current++;
        }
    }else if(questionLever == 2){
        if(part1==SQUARE && part2==EMPTY && part3==SQUARE && part4==EMPTY){
            current++;
        }
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
    std::pair<int,int> pos1 = nextPox(pos,cutterTile);
    std::pair<int,int> pos2 = pos1;
    switch (cutterTile.direction) {
    case NORTH:
        pos2.second++;
        break;
    case EAST:
        pos2.first++;
        break;
    case SOUTH:
        pos2.second--;
        break;
    case WEST:
        pos2.first--;
        break;
    default:
        break;
    }
    return std::make_pair(pos1,pos2);
}

std::pair<std::pair<int,int>,std::pair<int,int>> Map::cutterOutPox(int x, int y,Tile &cutterTile){
    std::pair<int,int> pos = std::make_pair(x,y);

    // if(tiles[pos.first][pos.second]->type != Tile::Type::Building || tiles[pos.first][pos.second]->name != "cutter"){
    //     qDebug() << "not cutter";
    //     return std::make_pair(pos,pos);
    // }
    std::pair<int,int> pos1 = nextPox(pos,cutterTile);
    std::pair<int,int> pos2 = pos1;
    switch (cutterTile.direction) {
    case NORTH:
        pos2.second++;
        break;
    case EAST:
        pos2.first++;
        break;
    case SOUTH:
        pos2.second--;
        break;
    case WEST:
        pos2.first--;
        break;
    default:
        break;
    }
    return std::make_pair(pos1,pos2);
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
    item->label->raise();
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

QString Map::buildingInfoText(const Tile &tile) const
{
    const BuildingInfoEntry entry = buildingInfoEntryForTile(tile);
    if (entry.enName.isEmpty()) {
        return {};
    }

    ConfigManager config;
    const QString languageCode = config.getLanguage();
    const QString localizedName = Localization::text(languageCode, entry.zhName, entry.enName).toHtmlEscaped();
    const QString localizedDescription = Localization::text(languageCode, entry.zhDescription, entry.enDescription).toHtmlEscaped();

    return QString("<b>%1</b><br/>%2").arg(localizedName, localizedDescription);
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
