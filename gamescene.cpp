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

    connect(beltbtn, &QPushButton::clicked, this, [this]() {
        qDebug() << "placing belt";
        isPlaceItem = true;
        if(currentTile){
            delete currentTile;
            currentTile = nullptr;
        }
        currentTile = new Tile(Tile::Type::Belt, "forward", NORTH);
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
        qDebug() << "该位置已有Tile";
        return;
    }

    map->setTile(gridX, gridY, *currentTile);

    isPlaceItem = false;
    delete(currentTile);
    currentTile = nullptr;

    update();
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


