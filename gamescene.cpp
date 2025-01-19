#include "gamescene.h"
#include "item.h"
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QFont>
#include <QFontDatabase>
#include <QSoundEffect>
#include <QDialog>
#include <QMessageBox>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDir>
#include <QInputDialog>

class UpgradeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpgradeDialog(bool itemMoveUpgrate, bool minerUpgrate ,bool cutterUpgrate ,QWidget *parent = nullptr) : QDialog(parent)
    {
        setWindowTitle("选择升级");

        QVBoxLayout *layout = new QVBoxLayout(this);

        // 升级传送带
        if(!itemMoveUpgrate){
            QPushButton *upgradeButton1 = new QPushButton("升级传送带", this);
            connect(upgradeButton1, &QPushButton::clicked, this, [this]() {
                selectedOption = 1;
                accept();
            });
            layout->addWidget(upgradeButton1);
        }

        // 升级传送带
        if(!minerUpgrate){
            QPushButton *upgradeButton2 = new QPushButton("升级开采器", this);
            connect(upgradeButton2, &QPushButton::clicked, this, [this]() {
                selectedOption = 2;
                accept();
            });
            layout->addWidget(upgradeButton2);
        }

        //升级切割机
        if(!cutterUpgrate){
            QPushButton *upgradeButton3 = new QPushButton("升级切割机", this);
            connect(upgradeButton3, &QPushButton::clicked, this, [this]() {
                selectedOption = 3;
                accept();
            });
            layout->addWidget(upgradeButton3);
        }

        setLayout(layout);
    }

    int getSelectedOption() const {
        return selectedOption;
    }

private:
    int selectedOption = 0;
};

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

    Tile ResSquare = Tile(Tile::Type::Resource,NORTH,"square");
    map->setTile(17,0,ResSquare);
    map->setTile(16,0,ResSquare);
    map->setTile(17,1,ResSquare);

    Tile ResCircle = Tile(Tile::Type::Resource,NORTH,"circle");
    map->setTile(0,0,ResCircle);
    map->setTile(0,1,ResCircle);
    map->setTile(1,0,ResCircle);
    qDebug() << "successfully build map";

    //编辑问题
    map->current = 0;
    map->target = 0;
    map->questionLever = 0;
    setPuzzle();

    int fontId = QFontDatabase::addApplicationFont(":/res/font/ComicNeue-Bold.ttf");
    if (fontId == -1) {
        qDebug() << "Failed to load font!";
    }
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (fontFamilies.isEmpty()) {
        qDebug() << "No font families found!";
    }
    QString fontFamily = fontFamilies.at(0);
    qDebug() << "Loaded font family:" << fontFamily;
    font.setPointSize(28);
    font.setFamily(fontFamily);
    map->countLabel->setFont(font);
    font.setPointSize(18);
    map->levelLabel->setFont(font);

    // test = new QLabel(this);
    // //QPixmap combinedPixmap = Item(SQUARE,SQUARE,SQUARE,SQUARE).getPixmap();
    // QPixmap combinedPixmap = Item(CIRCLE,CIRCLE,CIRCLE,CIRCLE).getPixmap();
    // test->setPixmap(combinedPixmap);

    isDragging = false;
    defaultBeltDirection = NORTH;

    //计时器
    itemMoveTimer = new QTimer(this);
    connect(itemMoveTimer, &QTimer::timeout, this, [this]() {
        map->moveItems();
        //qDebug() << "moving mining";
        map->countLabel->setText(QString("%1\n/%2").arg(map->current).arg(map->target));
        map->levelLabel->setText(QString("%1").arg(map->questionLever+1));
        map->levelLabel->raise();
        if(map->current >= map->target){
            map->questionLever++;
            setPuzzle();
            UpgradeDialog dialog(itemMoveUpgrate, minerUpgrate ,cutterUpgrate,this);
            if (dialog.exec() == QDialog::Accepted) {
                // 获取用户选择的选项
                int selectedOption = dialog.getSelectedOption();
                if (selectedOption == 1) {
                    itemMoveUpgrate = true;
                    itemMoveTimer->setInterval(itemMoveTimerIntervalUpgrate);
                    QMessageBox::information(this, "升级结果", "你选择了升级传送带！");
                } else if (selectedOption == 2) {
                    minerUpgrate = true;
                    minerTimer->setInterval(minerTimerIntervalUpgrate);
                    QMessageBox::information(this, "升级结果", "你选择了升级开采器！");
                } else if (selectedOption == 3){
                    cutterUpgrate = true;
                    cutterTimer->setInterval(cutterTimerIntervalUpgrate);
                    QMessageBox::information(this, "升级结果", "你选择了升级切割机！");
                }else{
                    QMessageBox::information(this, "升级结果", "未选择任何选项！");
                }
            } else {
                QMessageBox::information(this, "升级结果", "用户未选择升级选项！");
            }
        }
    });
    itemMoveTimer->start(800);

    minerTimer = new QTimer(this);
    connect(minerTimer, &QTimer::timeout, this, [this]() {
        map->performMining();
        //qDebug() << "perform mining";
    });
    minerTimer->start(3200);

    cutterTimer = new QTimer(this);
    connect(cutterTimer, &QTimer::timeout, this, [this]() {
        map->cutterUpdate();
    });
    cutterTimer->start(6400);


    QSoundEffect level_completeEffect;
    level_completeEffect.setSource(QUrl::fromLocalFile(":/res/sounds/level_complete.wav"));
    level_completeEffect.setVolume(0.5f);

    QSoundEffect destroy_buildingEffect;
    destroy_buildingEffect.setSource(QUrl::fromLocalFile(":/res/sounds/destroy_building.wav"));
    destroy_buildingEffect.setVolume(0.5f);

    QSoundEffect place_beltEffect;
    place_beltEffect.setSource(QUrl::fromLocalFile(":/res/sounds/place_belt.wav"));
    place_beltEffect.setVolume(0.5f);

    QSoundEffect place_buildingEffect;
    place_buildingEffect.setSource(QUrl::fromLocalFile(":/res/sounds/place_building.wav"));
    place_buildingEffect.setVolume(0.5f);

    QSoundEffect ui_clickEffect;
    ui_clickEffect.setSource(QUrl::fromLocalFile(":/res/sounds/ui_click.wav"));
    ui_clickEffect.setVolume(0.5f);


    //建立底部建筑按钮
    beltbtn = new QPushButton();
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
            currentTile = new Tile(Tile::Type::Belt, "forward", defaultBeltDirection);
        }
    });

    QPropertyAnimation *beltbtnanimation = new QPropertyAnimation(beltbtn, "geometry");
    beltbtnanimation->setDuration(100);
    beltbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect beltbtnGeometry = beltbtn->geometry();
    connect(beltbtn,&QPushButton::pressed,this,[=](){
        QRect rect = beltbtn->geometry();
        beltbtnanimation->setStartValue(rect);
        beltbtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.05, rect.y() + rect.height() * 0.05,
                                            rect.width() * 0.9, rect.height() * 0.9));
        beltbtnanimation->start();
    });
    connect(beltbtn,&QPushButton::released,this,[=]() {
        beltbtnanimation->setStartValue(beltbtn->geometry());
        beltbtnanimation->setEndValue(beltbtnGeometry);
        beltbtnanimation->start();
    });

    balancerbtn = new QPushButton();
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

    QPropertyAnimation *balancerbtnanimation = new QPropertyAnimation(balancerbtn, "geometry");
    balancerbtnanimation->setDuration(100);
    balancerbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect balancerbtnGeometry = balancerbtn->geometry();
    connect(balancerbtn,&QPushButton::pressed,this,[=](){
        QRect rect = balancerbtn->geometry();
        balancerbtnanimation->setStartValue(rect);
        balancerbtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.025, rect.y() + rect.height() * 0.025,
                                                rect.width() * 0.95, rect.height() * 0.95));
        balancerbtnanimation->start();
    });
    connect(balancerbtn,&QPushButton::released,this,[=]() {
        balancerbtnanimation->setStartValue(balancerbtn->geometry());
        balancerbtnanimation->setEndValue(balancerbtnGeometry);
        balancerbtnanimation->start();
    });

    underground_beltbtn = new QPushButton();
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

    QPropertyAnimation *underground_beltbtnanimation = new QPropertyAnimation(underground_beltbtn, "geometry");
    underground_beltbtnanimation->setDuration(100);
    underground_beltbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect underground_beltbtnGeometry = underground_beltbtn->geometry();
    connect(underground_beltbtn,&QPushButton::pressed,this,[=](){
        QRect rect = underground_beltbtn->geometry();
        underground_beltbtnanimation->setStartValue(rect);
        underground_beltbtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.025, rect.y() + rect.height() * 0.025,
                                                        rect.width() * 0.95, rect.height() * 0.95));
        underground_beltbtnanimation->start();
    });
    connect(underground_beltbtn,&QPushButton::released,this,[=]() {
        underground_beltbtnanimation->setStartValue(underground_beltbtn->geometry());
        underground_beltbtnanimation->setEndValue(underground_beltbtnGeometry);
        underground_beltbtnanimation->start();
    });

    minerbtn = new QPushButton();
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

    QPropertyAnimation *minerbtnanimation = new QPropertyAnimation(minerbtn, "geometry");
    minerbtnanimation->setDuration(100);
    minerbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect minerbtnGeometry = minerbtn->geometry();
    connect(minerbtn,&QPushButton::pressed,this,[=](){
        QRect rect = minerbtn->geometry();
        minerbtnanimation->setStartValue(rect);
        minerbtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.025, rect.y() + rect.height() * 0.025,
                                             rect.width() * 0.95, rect.height() * 0.95));
        minerbtnanimation->start();
    });
    connect(minerbtn,&QPushButton::released,this,[=]() {
        minerbtnanimation->setStartValue(minerbtn->geometry());
        minerbtnanimation->setEndValue(minerbtnGeometry);
        minerbtnanimation->start();
    });

    cutterbtn = new QPushButton();
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

    QPropertyAnimation * cutterbtnanimation = new QPropertyAnimation(cutterbtn, "geometry");
    cutterbtnanimation->setDuration(100);
    cutterbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect cutterbtnGeometry = cutterbtn->geometry();
    connect(cutterbtn,&QPushButton::pressed,this,[=](){
        QRect rect = cutterbtn->geometry();
        cutterbtnanimation->setStartValue(rect);
        cutterbtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.025, rect.y() + rect.height() * 0.025,
                                              rect.width() * 0.95, rect.height() * 0.95));
        cutterbtnanimation->start();
    });
    connect(cutterbtn,&QPushButton::released,this,[=]() {
        cutterbtnanimation->setStartValue(cutterbtn->geometry());
        cutterbtnanimation->setEndValue(cutterbtnGeometry);
        cutterbtnanimation->start();
    });

    rotaterbtn = new QPushButton();
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

    QPropertyAnimation *rotaterbtnanimation = new QPropertyAnimation(rotaterbtn, "geometry");
    rotaterbtnanimation->setDuration(100);
    rotaterbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect rotaterbtnGeometry = rotaterbtn->geometry();
    connect(rotaterbtn,&QPushButton::pressed,this,[=](){
        QRect rect = rotaterbtn->geometry();
        rotaterbtnanimation->setStartValue(rect);
        rotaterbtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.025, rect.y() + rect.height() * 0.025,
                                               rect.width() * 0.95, rect.height() * 0.95));
        rotaterbtnanimation->start();
    });
    connect(rotaterbtn,&QPushButton::released,this,[=]() {
        rotaterbtnanimation->setStartValue(rotaterbtn->geometry());
        rotaterbtnanimation->setEndValue(rotaterbtnGeometry);
        rotaterbtnanimation->start();
    });

    stackerbtn = new QPushButton();
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

    QPropertyAnimation *stackerbtnanimation = new QPropertyAnimation(stackerbtn, "geometry");
    stackerbtnanimation->setDuration(100);
    stackerbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect stackerbtnGeometry = stackerbtn->geometry();
    connect(stackerbtn,&QPushButton::pressed,this,[=](){
        QRect rect = stackerbtn->geometry();
        stackerbtnanimation->setStartValue(rect);
        stackerbtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.025, rect.y() + rect.height() * 0.025,
                                               rect.width() * 0.95, rect.height() * 0.95));
        stackerbtnanimation->start();
    });
    connect(stackerbtn,&QPushButton::released,this,[=]() {
        stackerbtnanimation->setStartValue(stackerbtn->geometry());
        stackerbtnanimation->setEndValue(stackerbtnGeometry);
        stackerbtnanimation->start();
    });

    mixerbtn = new QPushButton();
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

    QPropertyAnimation *mixerbtnanimation = new QPropertyAnimation(mixerbtn, "geometry");
    mixerbtnanimation->setDuration(100);
    mixerbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect mixerbtnGeometry = mixerbtn->geometry();
    connect(mixerbtn,&QPushButton::pressed,this,[=](){
        QRect rect = mixerbtn->geometry();
        mixerbtnanimation->setStartValue(rect);
        mixerbtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.025, rect.y() + rect.height() * 0.025,
                                             rect.width() * 0.95, rect.height() * 0.95));
        mixerbtnanimation->start();
    });
    connect(mixerbtn,&QPushButton::released,this,[=]() {
        mixerbtnanimation->setStartValue(mixerbtn->geometry());
        mixerbtnanimation->setEndValue(mixerbtnGeometry);
        mixerbtnanimation->start();
    });

    painterbtn = new QPushButton();
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

    QPropertyAnimation *painterbtnanimation = new QPropertyAnimation(painterbtn, "geometry");
    painterbtnanimation->setDuration(100);
    painterbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect painterbtnGeometry = painterbtn->geometry();
    connect(painterbtn,&QPushButton::pressed,this,[=](){
        QRect rect = painterbtn->geometry();
        painterbtnanimation->setStartValue(rect);
        painterbtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.025, rect.y() + rect.height() * 0.025,
                                               rect.width() * 0.95, rect.height() * 0.95));
        painterbtnanimation->start();
    });
    connect(painterbtn,&QPushButton::released,this,[=]() {
        painterbtnanimation->setStartValue(painterbtn->geometry());
        painterbtnanimation->setEndValue(painterbtnGeometry);
        painterbtnanimation->start();
    });

    trashbtn = new QPushButton();
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

    QPropertyAnimation *trashbtnanimation = new QPropertyAnimation(trashbtn, "geometry");
    trashbtnanimation->setDuration(100);
    trashbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect trashbtnGeometry = trashbtn->geometry();
    connect(trashbtn,&QPushButton::pressed,this,[=](){
        QRect rect = trashbtn->geometry();
        trashbtnanimation->setStartValue(rect);
        trashbtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.025, rect.y() + rect.height() * 0.025,
                                             rect.width() * 0.95, rect.height() * 0.95));
        trashbtnanimation->start();
    });
    connect(trashbtn,&QPushButton::released,this,[=]() {
        trashbtnanimation->setStartValue(trashbtn->geometry());
        trashbtnanimation->setEndValue(trashbtnGeometry);
        trashbtnanimation->start();
    });

    QPushButton* backbtn = new QPushButton();
    backbtn->setParent(this);
    backbtn->setFixedSize(50, 50);
    backbtn->setIconSize(QSize(30,30));
    backbtn->setIcon(QIcon(":/res/settings_menu_exit.png"));
    backbtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    backbtn->move(1525,65);

    connect(backbtn, &QPushButton::clicked, this, &Gamescene::returnToMainScene);

    QPushButton* savebtn = new QPushButton();
    savebtn->setParent(this);
    savebtn->setFixedSize(50, 50);
    savebtn->setIconSize(QSize(30,30));
    savebtn->setIcon(QIcon(":/res/save.png"));
    savebtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    savebtn->move(1475,65);

    connect(savebtn, &QPushButton::clicked, this, [this](){
        bool ok;
        QString filename = QInputDialog::getText(this, "输入文件名", "请输入存档文件名:", QLineEdit::Normal, "save.json", &ok);

        if (ok && !filename.isEmpty()) {
            if (!filename.endsWith(".json")) {
                filename += ".json";
            }
            saveGame(filename);
            autoSaveGame("auto_save.json");
        } else {
            QMessageBox::warning(this, "警告", "未输入文件名，保存取消");
        }
        //saveGame("save.json");
    });


    //测试
    // Tile forwardBelt(Tile::Type::Belt, "forward", NORTH);
    // Tile forwardBelt2(Tile::Type::Belt, "forward", EAST);
    // Tile rightBelt(Tile::Type::Belt, "right", NORTH);
    // Tile northMiner(Tile::Type::Building, NORTH, "miner-chainable");
    // Tile eastMiner(Tile::Type::Building, EAST, "miner-chainable");
    // Tile northCutter(Tile::Type::Building, EAST, "cutter", std::make_pair(1,2));
    // map->setTile(0,0,rightBelt);
    // map->setTile(1,0,forwardBelt);
    // map->setTile(2,0,forwardBelt);
    // map->setTile(0,1,forwardBelt2);
    // map->setTile(5,5,northMiner);
    // map->setTile(5,6,eastMiner);
    // map->setTile(2,20,northCutter);
    // map->setTile(17,0,forwardBelt);
    //map->deleteTile(2,20);

    map->setGeometry(0, 0, 1600, 900);
}

void Gamescene::setPuzzle(){
    map->current = 0;
    if(map->questionLever == 0){
        map->target = 20;
        QPixmap level_1 = Item().drawPixmap(CIRCLE,CIRCLE,CIRCLE,CIRCLE,2*TILESIZE);
        //QPixmap level_1 = Item().drawCircle();
        map->questionLabel->setPixmap(level_1);
    }else if(map->questionLever == 1){
        map->target = 30;
        QPixmap level_2 = Item().drawPixmap(SQUARE,SQUARE,SQUARE,SQUARE,2*TILESIZE);
        map->questionLabel->setPixmap(level_2);
    }else if(map->questionLever == 2){
        map->target = 50;
        QPixmap level_3 = Item().drawPixmap(SQUARE,EMPTY,SQUARE,EMPTY,2*TILESIZE);
        map->questionLabel->setPixmap(level_3);
    }else{
        map->questionLabel->clear();
        map->countLabel->hide();
        map->levelLabel->hide();
    }
    map->questionLabel->raise();
    map->countLabel->raise();
}

void Gamescene::mousePressEvent(QMouseEvent *event) {
    int mouseX = event->pos().x();
    int mouseY = event->pos().y();

    int gridX = mouseY / TILESIZE; // 行号（Y 坐标）
    int gridY = mouseX / TILESIZE; // 列号（X 坐标）
    if (event->button() == Qt::LeftButton) {
        if (!isPlaceItem) {
            return;
        }
        // 检查坐标是否在地图范围内
        if (gridX < 0 || gridX >= map->getheight() || gridY < 0 || gridY >= map->getwidth()) {
            qDebug() << "点击位置超出地图范围";
            return;
        }

        if(map->getTile(gridX, gridY).type != Tile::Type::Empty && !(map->getTile(gridX, gridY).type == Tile::Type::Resource && currentTile->type == Tile::Type::Building && currentTile->name == "miner")){
            qDebug() << "pos("<< gridX <<","<< gridY <<") already has a Tile";
            return;
        }

        map->setTile(gridX, gridY, *currentTile);
        isDragging = true;

        isPlaceItem = false;
        delete(currentTile);
        currentTile = nullptr;
    }
    if (event->button() == Qt::RightButton) {
        if (isPlaceItem) {
            isPlaceItem = false;
            delete(currentTile);
            currentTile = nullptr;
        }else{
            if (gridX < 0 || gridX >= map->getheight() || gridY < 0 || gridY >= map->getwidth()) {
                qDebug() << "点击位置超出地图范围";
                return;
            }
            map->deleteTile(gridX, gridY);
        }
    }
}

void Gamescene::mouseMoveEvent(QMouseEvent *event) {
    // if (isDragging && isPlaceItem && currentTile && currentTile->type == Tile::Type::Belt) {
    //     int mouseX = event->pos().x();
    //     int mouseY = event->pos().y();

    //     int gridX = mouseY / TILESIZE;
    //     int gridY = mouseX / TILESIZE;

    //     if (gridX < 0 || gridX >= map->getheight() || gridY < 0 || gridY >= map->getwidth()) {
    //         return;
    //     }

    //     if(map->getTile(gridX, gridY).type != Tile::Type::Empty){
    //         return;
    //     }

    //     map->setTile(gridX, gridY, *currentTile);
    //     qDebug() << "dragging";
    // }
}

void Gamescene::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isDragging = false;  // 结束拖动
    }
}

void Gamescene::keyPressEvent(QKeyEvent *event){
    if (isPlaceItem && currentTile) {
        if (event->key() == Qt::Key_R) {
            qDebug() << "press R";
            currentTile->changeDirection();
            if(currentTile->type == Tile::Type::Belt){
                defaultBeltDirection = (defaultBeltDirection+1)%4;
            }
        }
        if (event->key() == Qt::Key_T) {
            qDebug() << "press T";
            currentTile->changeState();
        }
    }
    if (event->key() == Qt::Key_1) {
        beltbtn->click();
    }
    if (event->key() == Qt::Key_2) {
        balancerbtn->click();
    }
    if (event->key() == Qt::Key_3) {
        underground_beltbtn->click();
    }
    if (event->key() == Qt::Key_4) {
        minerbtn->click();
    }
    if (event->key() == Qt::Key_5) {
        cutterbtn->click();
    }
    if (event->key() == Qt::Key_6) {
        rotaterbtn->click();
    }
    if (event->key() == Qt::Key_7) {
        stackerbtn->click();
    }
    if (event->key() == Qt::Key_8) {
        mixerbtn->click();
    }
    if (event->key() == Qt::Key_9) {
        painterbtn->click();
    }
    if (event->key() == Qt::Key_0) {
        trashbtn->click();
    }
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

void Gamescene::saveGame(const QString& filename) {
    QDir dir;
    QString savePath = dir.currentPath() + "/saves/" + filename;

    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法打开文件进行保存:" << filename;
        return;
    }

    QJsonObject gameState;
    gameState["questionLever"] = map->questionLever;
    gameState["current"] = map->current;
    gameState["target"] = map->target;

    QJsonObject upgrades;
    upgrades["itemMoveUpgrate"] = itemMoveUpgrate;
    upgrades["minerUpgrate"] = minerUpgrate;
    upgrades["cutterUpgrate"] = cutterUpgrate;
    gameState["upgrades"] = upgrades;

    QJsonObject timers;
    timers["itemMoveTimerIntervalUpgrate"] = itemMoveTimerIntervalUpgrate;
    timers["minerTimerIntervalUpgrate"] = minerTimerIntervalUpgrate;
    timers["cutterTimerIntervalUpgrate"] = cutterTimerIntervalUpgrate;
    gameState["timers"] = timers;

    QJsonArray mapTiles;
    for (int x = 0; x < map->getheight(); ++x) {
        for (int y = 0; y < map->getwidth(); ++y) {
            Tile tile = map->getTile(x, y);
            if (tile.type != Tile::Type::Empty) {
                QJsonObject tileObject;
                tileObject["x"] = x;
                tileObject["y"] = y;
                tileObject["type"] = static_cast<int>(tile.type);
                tileObject["direction"] = tile.direction;
                tileObject["state"] = tile.state;
                tileObject["name"] = tile.name;

                QJsonObject sizeObject;
                sizeObject["first"] = tile.size.first;
                sizeObject["second"] = tile.size.second;
                tileObject["size"] = sizeObject;

                if (tile.item) {
                    QJsonObject itemObject;
                    itemObject["part1"] = tile.item->part1;
                    itemObject["part2"] = tile.item->part2;
                    itemObject["part3"] = tile.item->part3;
                    itemObject["part4"] = tile.item->part4;

                    QJsonObject posObject;
                    posObject["first"] = tile.item->pos.first;
                    posObject["second"] = tile.item->pos.second;
                    itemObject["pos"] = posObject;

                    tileObject["item"] = itemObject;
                }
                mapTiles.append(tileObject);
            }
        }
    }
    gameState["map"] = mapTiles;

    QJsonDocument doc(gameState);
    file.write(doc.toJson());
    file.close();
    qDebug() << "游戏已保存到:" << filename;
}

void Gamescene::loadGame(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开文件进行加载:" << filename;
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject gameState = doc.object();

    map->questionLever = gameState["questionLever"].toInt();
    map->target = gameState["target"].toInt();
    setPuzzle();
    map->current = gameState["current"].toInt();

    QJsonObject upgrades = gameState["upgrades"].toObject();
    itemMoveUpgrate = upgrades["itemMoveUpgrate"].toBool();
    minerUpgrate = upgrades["minerUpgrate"].toBool();
    cutterUpgrate = upgrades["cutterUpgrate"].toBool();

    QJsonObject timers = gameState["timers"].toObject();
    itemMoveTimerIntervalUpgrate = timers["itemMoveTimerIntervalUpgrate"].toInt();
    minerTimerIntervalUpgrate = timers["minerTimerIntervalUpgrate"].toInt();
    cutterTimerIntervalUpgrate = timers["cutterTimerIntervalUpgrate"].toInt();

    QJsonArray mapTiles = gameState["map"].toArray();
    for (const QJsonValue& tileValue : mapTiles) {
        QJsonObject tileObject = tileValue.toObject();
        int x = tileObject["x"].toInt();
        int y = tileObject["y"].toInt();
        Tile::Type type = static_cast<Tile::Type>(tileObject["type"].toInt());
        int direction = tileObject["direction"].toInt();
        QString state = tileObject["state"].toString();
        QString name = tileObject["name"].toString();

        // 读取 size
        QJsonObject sizeObject = tileObject["size"].toObject();
        std::pair<int, int> size = std::make_pair(sizeObject["first"].toInt(), sizeObject["second"].toInt());

        if (type == Tile::Type::Belt) {
            Tile tile(type, state, direction);
            tile.size = size;  // 恢复 size
            if (tileObject.contains("item")) {
                QJsonObject itemObject = tileObject["item"].toObject();
                Item* item = new Item(
                    itemObject["part1"].toInt(),
                    itemObject["part2"].toInt(),
                    itemObject["part3"].toInt(),
                    itemObject["part4"].toInt()
                    );

                // 恢复 pos
                QJsonObject posObject = itemObject["pos"].toObject();
                item->pos = std::make_pair(posObject["first"].toInt(), posObject["second"].toInt());

                tile.item = item;
            }
            map->setTile(x, y, tile);
        } else {
            Tile tile(type, direction, name, size);  // 恢复 size
            if (tileObject.contains("item")) {
                QJsonObject itemObject = tileObject["item"].toObject();
                Item* item = new Item(
                    itemObject["part1"].toInt(),
                    itemObject["part2"].toInt(),
                    itemObject["part3"].toInt(),
                    itemObject["part4"].toInt()
                    );

                // 恢复 pos
                QJsonObject posObject = itemObject["pos"].toObject();
                item->pos = std::make_pair(posObject["first"].toInt(), posObject["second"].toInt());

                tile.item = item;
            }
            map->setTile(x, y, tile);
        }
    }

    file.close();
    qDebug() << "游戏已从" << filename << "加载";
}

void Gamescene::autoSaveGame(const QString& filename) {
    QDir dir;
    QString savePath = dir.currentPath() + "/auto_save/" + filename;

    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法打开文件进行保存:" << filename;
        return;
    }

    QJsonObject gameState;
    gameState["questionLever"] = map->questionLever;
    gameState["current"] = map->current;
    gameState["target"] = map->target;

    QJsonObject upgrades;
    upgrades["itemMoveUpgrate"] = itemMoveUpgrate;
    upgrades["minerUpgrate"] = minerUpgrate;
    upgrades["cutterUpgrate"] = cutterUpgrate;
    gameState["upgrades"] = upgrades;

    QJsonObject timers;
    timers["itemMoveTimerIntervalUpgrate"] = itemMoveTimerIntervalUpgrate;
    timers["minerTimerIntervalUpgrate"] = minerTimerIntervalUpgrate;
    timers["cutterTimerIntervalUpgrate"] = cutterTimerIntervalUpgrate;
    gameState["timers"] = timers;

    QJsonArray mapTiles;
    for (int x = 0; x < map->getheight(); ++x) {
        for (int y = 0; y < map->getwidth(); ++y) {
            Tile tile = map->getTile(x, y);
            if (tile.type != Tile::Type::Empty) {
                QJsonObject tileObject;
                tileObject["x"] = x;
                tileObject["y"] = y;
                tileObject["type"] = static_cast<int>(tile.type);
                tileObject["direction"] = tile.direction;
                tileObject["state"] = tile.state;
                tileObject["name"] = tile.name;

                QJsonObject sizeObject;
                sizeObject["first"] = tile.size.first;
                sizeObject["second"] = tile.size.second;
                tileObject["size"] = sizeObject;

                if (tile.item) {
                    QJsonObject itemObject;
                    itemObject["part1"] = tile.item->part1;
                    itemObject["part2"] = tile.item->part2;
                    itemObject["part3"] = tile.item->part3;
                    itemObject["part4"] = tile.item->part4;

                    QJsonObject posObject;
                    posObject["first"] = tile.item->pos.first;
                    posObject["second"] = tile.item->pos.second;
                    itemObject["pos"] = posObject;

                    tileObject["item"] = itemObject;
                }
                mapTiles.append(tileObject);
            }
        }
    }
    gameState["map"] = mapTiles;

    QJsonDocument doc(gameState);
    file.write(doc.toJson());
    file.close();
    qDebug() << "游戏已保存到:" << filename;
}

void Gamescene::autoLoadGame(const QString& filename) {
    {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "无法打开文件进行加载:" << filename;
            return;
        }

        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject gameState = doc.object();

        map->questionLever = gameState["questionLever"].toInt();
        map->target = gameState["target"].toInt();
        setPuzzle();
        map->current = gameState["current"].toInt();

        QJsonObject upgrades = gameState["upgrades"].toObject();
        itemMoveUpgrate = upgrades["itemMoveUpgrate"].toBool();
        minerUpgrate = upgrades["minerUpgrate"].toBool();
        cutterUpgrate = upgrades["cutterUpgrate"].toBool();

        QJsonObject timers = gameState["timers"].toObject();
        itemMoveTimerIntervalUpgrate = timers["itemMoveTimerIntervalUpgrate"].toInt();
        minerTimerIntervalUpgrate = timers["minerTimerIntervalUpgrate"].toInt();
        cutterTimerIntervalUpgrate = timers["cutterTimerIntervalUpgrate"].toInt();

        QJsonArray mapTiles = gameState["map"].toArray();
        for (const QJsonValue& tileValue : mapTiles) {
            QJsonObject tileObject = tileValue.toObject();
            int x = tileObject["x"].toInt();
            int y = tileObject["y"].toInt();
            Tile::Type type = static_cast<Tile::Type>(tileObject["type"].toInt());
            int direction = tileObject["direction"].toInt();
            QString state = tileObject["state"].toString();
            QString name = tileObject["name"].toString();

            // 读取 size
            QJsonObject sizeObject = tileObject["size"].toObject();
            std::pair<int, int> size = std::make_pair(sizeObject["first"].toInt(), sizeObject["second"].toInt());

            if (type == Tile::Type::Belt) {
                Tile tile(type, state, direction);
                tile.size = size;  // 恢复 size
                if (tileObject.contains("item")) {
                    QJsonObject itemObject = tileObject["item"].toObject();
                    Item* item = new Item(
                        itemObject["part1"].toInt(),
                        itemObject["part2"].toInt(),
                        itemObject["part3"].toInt(),
                        itemObject["part4"].toInt()
                        );

                    // 恢复 pos
                    QJsonObject posObject = itemObject["pos"].toObject();
                    item->pos = std::make_pair(posObject["first"].toInt(), posObject["second"].toInt());

                    tile.item = item;
                }
                map->setTile(x, y, tile);
            } else {
                Tile tile(type, direction, name, size);  // 恢复 size
                if (tileObject.contains("item")) {
                    QJsonObject itemObject = tileObject["item"].toObject();
                    Item* item = new Item(
                        itemObject["part1"].toInt(),
                        itemObject["part2"].toInt(),
                        itemObject["part3"].toInt(),
                        itemObject["part4"].toInt()
                        );

                    // 恢复 pos
                    QJsonObject posObject = itemObject["pos"].toObject();
                    item->pos = std::make_pair(posObject["first"].toInt(), posObject["second"].toInt());

                    tile.item = item;
                }
                map->setTile(x, y, tile);
            }
        }

        file.close();
        qDebug() << "游戏已从" << filename << "加载";
    }
}

Gamescene::~Gamescene() {

}

void Gamescene::returnToMainScene() {
    emit returnToMain();
    this->close();
    this->deleteLater();
}

#include "gamescene.moc"
