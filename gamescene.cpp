#include "gamescene.h"
#include <QPushButton>

Gamescene::Gamescene(QWidget *parent)
    : isPlaceItem(false), currentTile(nullptr), QWidget{parent}
{
    //编辑窗口基本信息
    setFixedSize(1600,900);
    setWindowIcon(QIcon(":/res/icon.ico"));
    setWindowTitle("Shapez");

    qDebug() << "building map";
    map = new Map(18,32,this);
    Tile Hub(Tile::Type::Hub);
    map->setTile(7,14,Hub);
    qDebug() << "successfully build map";

    //建立底部建筑按钮
    QPushButton * beltbtn = new QPushButton();
    beltbtn->setParent(this);
    beltbtn->setFixedSize(70, 70);
    beltbtn->setIconSize(QSize(50,50));
    beltbtn->setIcon(QIcon(":/res/building_icons/belt.png"));
    beltbtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    beltbtn->move(450,810);

    connect(beltbtn, &QPushButton::clicked, this, [this]() {
        if(isPlaceItem && currentTile && currentTile->type == Tile::Type::Belt){
            qDebug() << "cancel placing belt";
            isPlaceItem = false;
            if(currentTile){
                delete currentTile;
                currentTile = nullptr;
            }
        }else{
            qDebug() << "placing belt";
            isPlaceItem = true;
            if(currentTile){
                delete currentTile;
                currentTile = nullptr;
            }
            currentTile = new Tile(Tile::Type::Belt, "forward", NORTH);
        }
    });

    QPushButton * balancerbtn = new QPushButton();
    balancerbtn->setParent(this);
    balancerbtn->setFixedSize(70, 70);
    balancerbtn->setIconSize(QSize(50,50));
    balancerbtn->setIcon(QIcon(":/res/building_icons/balancer.png"));
    balancerbtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    balancerbtn->move(520,810);

    connect(balancerbtn, &QPushButton::clicked, this, [this]() {
        if(isPlaceItem && currentTile && currentTile->name == "balancer"){
            qDebug() << "cancel placing balancer";
            isPlaceItem = false;
            if(currentTile){
                delete currentTile;
                currentTile = nullptr;
            }
        }else{
            qDebug() << "placing balancer";
            isPlaceItem = true;
            if(currentTile){
                delete currentTile;
                currentTile = nullptr;
            }
            currentTile = new Tile(Tile::Type::Building, NORTH, "balancer",std::make_pair(1,2));
        }
    });

    QPushButton * underground_beltbtn = new QPushButton();
    underground_beltbtn->setParent(this);
    underground_beltbtn->setFixedSize(70, 70);
    underground_beltbtn->setIconSize(QSize(50,50));
    underground_beltbtn->setIcon(QIcon(":/res/building_icons/underground_belt.png"));
    underground_beltbtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    underground_beltbtn->move(590,810);

    connect(underground_beltbtn, &QPushButton::clicked, this, [this]() {
        if(isPlaceItem && currentTile && currentTile->name == "underground_belt_entry"){
            qDebug() << "cancel placing underground_belt";
            isPlaceItem = false;
            if(currentTile){
                delete currentTile;
                currentTile = nullptr;
            }
        }else{
            qDebug() << "placing underground_belt";
            isPlaceItem = true;
            if(currentTile){
                delete currentTile;
                currentTile = nullptr;
            }
            currentTile = new Tile(Tile::Type::Building, NORTH, "underground_belt_entry");
        }
    });

    QPushButton * minerbtn = new QPushButton();
    minerbtn->setParent(this);
    minerbtn->setFixedSize(70, 70);
    minerbtn->setIconSize(QSize(50,50));
    minerbtn->setIcon(QIcon(":/res/building_icons/miner.png"));
    minerbtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    minerbtn->move(660,810);

    connect(minerbtn, &QPushButton::clicked, this, [this]() {
        if(isPlaceItem && currentTile && currentTile->name == "miner"){
            qDebug() << "cancel placing miner";
            isPlaceItem = false;
            if(currentTile){
                delete currentTile;
                currentTile = nullptr;
            }
        }else{
            qDebug() << "placing miner";
            isPlaceItem = true;
            if(currentTile){
                delete currentTile;
                currentTile = nullptr;
            }
            currentTile = new Tile(Tile::Type::Building, NORTH, "miner");
        }
    });

    QPushButton * cutterbtn = new QPushButton();
    cutterbtn->setParent(this);
    cutterbtn->setFixedSize(70, 70);
    cutterbtn->setIconSize(QSize(50,50));
    cutterbtn->setIcon(QIcon(":/res/building_icons/cutter.png"));
    cutterbtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    cutterbtn->move(730,810);

    connect(cutterbtn, &QPushButton::clicked, this, [this]() {
        if(isPlaceItem && currentTile && currentTile->name == "cutter"){
            qDebug() << "cancel placing cutter";
            isPlaceItem = false;
            if(currentTile){
                delete currentTile;
                currentTile = nullptr;
            }
        }else{
            qDebug() << "placing cutter";
            isPlaceItem = true;
            if(currentTile){
                delete currentTile;
                currentTile = nullptr;
            }
            currentTile = new Tile(Tile::Type::Building, NORTH, "cutter", std::make_pair(1,2));
        }
    });

    QPushButton * rotaterbtn = new QPushButton();
    rotaterbtn->setParent(this);
    rotaterbtn->setFixedSize(70, 70);
    rotaterbtn->setIconSize(QSize(50,50));
    rotaterbtn->setIcon(QIcon(":/res/building_icons/rotater.png"));
    rotaterbtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    rotaterbtn->move(800,810);

    connect(rotaterbtn, &QPushButton::clicked, this, [this]() {
        if(isPlaceItem && currentTile && currentTile->name == "rotater"){
            qDebug() << "cancel placing rotater";
            isPlaceItem = false;
            if(currentTile){
                delete currentTile;
                currentTile = nullptr;
            }
        }else{
            qDebug() << "placing rotater";
            isPlaceItem = true;
            if(currentTile){
                delete currentTile;
                currentTile = nullptr;
            }
            currentTile = new Tile(Tile::Type::Building, NORTH, "rotater");
        }
    });

    QPushButton * stackerbtn = new QPushButton();
    stackerbtn->setParent(this);
    stackerbtn->setFixedSize(70, 70);
    stackerbtn->setIconSize(QSize(50,50));
    stackerbtn->setIcon(QIcon(":/res/building_icons/stacker.png"));
    stackerbtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    stackerbtn->move(870,810);

    connect(stackerbtn, &QPushButton::clicked, this, [this]() {
        if(isPlaceItem && currentTile && currentTile->name == "stacker"){
            qDebug() << "cancel placing stacker";
            isPlaceItem = false;
            if(currentTile){
                delete currentTile;
                currentTile = nullptr;
            }
        }else{
            qDebug() << "placing stacker";
            isPlaceItem = true;
            if(currentTile){
                delete currentTile;
                currentTile = nullptr;
            }
            currentTile = new Tile(Tile::Type::Building, NORTH, "stacker", std::make_pair(1,2));
        }
    });

    QPushButton * mixerbtn = new QPushButton();
    mixerbtn->setParent(this);
    mixerbtn->setFixedSize(70, 70);
    mixerbtn->setIconSize(QSize(50,50));
    mixerbtn->setIcon(QIcon(":/res/building_icons/mixer.png"));
    mixerbtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    mixerbtn->move(940,810);

    connect(mixerbtn, &QPushButton::clicked, this, [this]() {
        if(isPlaceItem && currentTile && currentTile->name == "mixer"){
            qDebug() << "cancel placing mixer";
            isPlaceItem = false;
            if(currentTile){
                delete currentTile;
                currentTile = nullptr;
            }
        }else{
            qDebug() << "placing mixer";
            isPlaceItem = true;
            if(currentTile){
                delete currentTile;
                currentTile = nullptr;
            }
            currentTile = new Tile(Tile::Type::Building, NORTH, "mixer", std::make_pair(1,2));
        }
    });

    QPushButton * painterbtn = new QPushButton();
    painterbtn->setParent(this);
    painterbtn->setFixedSize(70, 70);
    painterbtn->setIconSize(QSize(50,50));
    painterbtn->setIcon(QIcon(":/res/building_icons/painter.png"));
    painterbtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    painterbtn->move(1010,810);

    connect(painterbtn, &QPushButton::clicked, this, [this]() {
        if(isPlaceItem && currentTile && currentTile->name == "painter"){
            qDebug() << "cancel placing painter";
            isPlaceItem = false;
            if(currentTile){
                delete currentTile;
                currentTile = nullptr;
            }
        }else{
            qDebug() << "placing painter";
            isPlaceItem = true;
            if(currentTile){
                delete currentTile;
                currentTile = nullptr;
            }
            currentTile = new Tile(Tile::Type::Building, NORTH, "painter", std::make_pair(1,2));
        }
    });

    QPushButton * trashbtn = new QPushButton();
    trashbtn->setParent(this);
    trashbtn->setFixedSize(70, 70);
    trashbtn->setIconSize(QSize(50,50));
    trashbtn->setIcon(QIcon(":/res/building_icons/trash.png"));
    trashbtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    trashbtn->move(1080,810);

    connect(trashbtn, &QPushButton::clicked, this, [this]() {
        if(isPlaceItem && currentTile && currentTile->name == "trash"){
            qDebug() << "cancel placing trash";
            isPlaceItem = false;
            if(currentTile){
                delete currentTile;
                currentTile = nullptr;
            }
        }else{
            qDebug() << "placing trash";
            isPlaceItem = true;
            if(currentTile){
                delete currentTile;
                currentTile = nullptr;
            }
            currentTile = new Tile(Tile::Type::Building, NORTH, "trash");
        }
    });



    //测试
    Tile forwardBelt(Tile::Type::Belt, "forward", NORTH);
    Tile forwardBelt2(Tile::Type::Belt, "forward", EAST);
    Tile rightBelt(Tile::Type::Belt, "right", NORTH);
    Tile northMiner(Tile::Type::Building, NORTH, "miner-chainable");
    Tile eastMiner(Tile::Type::Building, EAST, "miner-chainable");
    Tile northCutter(Tile::Type::Building, EAST, "cutter", std::make_pair(1,2));
    map->setTile(0,0,rightBelt);
    map->setTile(1,0,forwardBelt);
    map->setTile(2,0,forwardBelt);
    map->setTile(0,1,forwardBelt2);
    map->setTile(5,5,northMiner);
    map->setTile(5,6,eastMiner);
    map->setTile(2,20,northCutter);
    map->setTile(17,0,forwardBelt);
    //map->deleteTile(2,20);

    map->setGeometry(0, 0, 1600, 900);
}

void Gamescene::mousePressEvent(QMouseEvent *event) {
    if (!isPlaceItem) {
        return;
    }

    int mouseX = event->pos().x();
    int mouseY = event->pos().y();

    int gridX = mouseY / TILESIZE; // 行号（Y 坐标）
    int gridY = mouseX / TILESIZE; // 列号（X 坐标）

    // 检查坐标是否在地图范围内
    if (gridX < 0 || gridX >= map->getheight() || gridY < 0 || gridY >= map->getwidth()) {
        qDebug() << "点击位置超出地图范围";
        return;
    }

    if(map->getTile(gridX, gridY).type != Tile::Type::Empty){
        qDebug() << "pos("<< gridX <<","<< gridY <<") already has a Tile";
        return;
    }

    map->setTile(gridX, gridY, *currentTile);

    isPlaceItem = false;
    delete(currentTile);
    currentTile = nullptr;
}

void Gamescene::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QColor backgroundColor("#ECEEF2");
    painter.fillRect(rect(), backgroundColor);//设置背景色

    int tileSize = TILESIZE;

    //画方格和建筑按钮框
    QColor lineColor("#E3E7EA");
    QPen pen(lineColor);
    pen.setWidth(1);
    painter.setPen(pen);

    for (int y = 0; y < height(); y += tileSize) {
        painter.drawLine(0, y, width(), y);
    }

    for (int x = 0; x < width(); x += tileSize) {
        painter.drawLine(x, 0, x, height());
    }

    QColor rectColor(121, 122, 128, 60);
    painter.setBrush(rectColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(QRect(450, 810, 700, 70), 8, 8);
}


