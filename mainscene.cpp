#include "mainscene.h"
#include <QMessageBox>
#include "./ui_mainscene.h"
Mainscene::Mainscene(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainScene)
{
    ui->setupUi(this);

    //编辑窗口信息
    setFixedSize(1600,900);
    setWindowIcon(QIcon(":/res/icon.ico"));
    setWindowTitle("Shapez");

    //设置背景音乐
    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);
    // connect(player, &QMediaPlayer::positionChanged, this, &MediaExample::positionChanged);
    player->setSource(QUrl("qrc:/res/menu.wav"));
    audioOutput->setVolume(0.3);
    player->play();


    //添加各种按钮
    QPushButton * newbtn = new QPushButton("新游戏",this);
    newbtn->setStyleSheet("QPushButton {"
                          "border-radius: 8px;"
                          "background-color: rgb(79,152,221);"
                          "color: rgb(255, 255, 255);"
                          "border: none;"
                          "}"
                          "QPushButton:hover {"
                          "background-color: rgb(95,160,222);"
                          "}");
    newbtn->move(812,424);
    newbtn->resize(264,66);
    QFont font1("Microsoft YaHei", 16, QFont::Normal);
    newbtn->setFont(font1);

    QPushButton * readbtn = new QPushButton("读取存档",this);
    readbtn->setStyleSheet("QPushButton {"
                           "border-radius: 8px;"
                           "background-color: rgb(79,152,221);"
                           "color: rgb(255, 255, 255);"
                           "border: none;"
                           "}"
                           "QPushButton:hover {"
                           "background-color: rgb(95,160,222);"
                           "}");
    readbtn->move(812,502);
    readbtn->resize(264,60);
    QFont font2("Microsoft YaHei", 16, QFont::Normal);
    readbtn->setFont(font2);

    QPushButton * continuebtn = new QPushButton("继续游戏",this);
    continuebtn->setStyleSheet("QPushButton {"
                               "border-radius: 8px;"
                               "background-color: rgb(97,183,104);"
                               "color: rgb(255, 255, 255);"
                               "border: none;"
                               "}"
                               "QPushButton:hover {"
                               "background-color: rgb(89,180,96);"
                               "}");
    continuebtn->move(524,424);
    continuebtn->resize(264,144);
    QFont font3("Microsoft YaHei", 20, QFont::Normal);
    continuebtn->setFont(font3);

    QPushButton * getonsteambtn = new QPushButton();
    getonsteambtn->setParent(this);
    getonsteambtn->setIconSize(QSize(600,145));
    getonsteambtn->setIcon(QIcon(":/get_on_steam.png"));
    getonsteambtn->setStyleSheet(
        "QPushButton {"
        " border: none;"
        " background-color: transparent;"
        " padding: 0;"
        " margin: 0;"
        "}"
        );
    getonsteambtn->move(500,725);

    QPushButton * githubbtn = new QPushButton();
    githubbtn->setParent(this);
    githubbtn->setFixedSize(80, 80);
    githubbtn->setIconSize(QSize(54,54));
    githubbtn->setIcon(QIcon(":/res/github.png"));
    githubbtn->setStyleSheet(
        "QPushButton {"
        " border: none;"
        " background-color: white;"
        " border-radius: 40px;"
        " padding: 0;"
        " margin: 0;"
        "}"
        );
    githubbtn->move(40,780);

    QPropertyAnimation *newbtnanimation = new QPropertyAnimation(newbtn, "geometry");
    newbtnanimation->setDuration(100);
    newbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect newbtnGeometry = newbtn->geometry();
    connect(newbtn,&QPushButton::pressed,this,[=](){
        qDebug() << "点击新游戏";
        QRect rect = newbtn->geometry();
        newbtnanimation->setStartValue(rect);
        newbtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.025, rect.y() + rect.height() * 0.025,
                                           rect.width() * 0.95, rect.height() * 0.95));
        newbtnanimation->start();
    });
    connect(newbtn,&QPushButton::released,this,[=]() {
        newbtnanimation->setStartValue(newbtn->geometry());
        newbtnanimation->setEndValue(newbtnGeometry);
        newbtnanimation->start();

        QTimer::singleShot(500,this,[=](){
            gamescene = new Gamescene;
            player->pause();
            this->hide();
            gamescene->setGeometry(this->geometry());
            connect(gamescene, &Gamescene::returnToMain, this, [this]() {
                this->show();  // 当 Gamescene 被销毁时，重新显示 Mainscene
                player->play();
                gamescene = nullptr;
                qDebug() << "当前在主界面";
            });
            gamescene->show();
            qDebug() << "当前在游戏界面";
        });
    });

    QPropertyAnimation *readbtnanimation = new QPropertyAnimation(readbtn, "geometry");
    readbtnanimation->setDuration(100);
    readbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect readbtnGeometry = readbtn->geometry();
    connect(readbtn,&QPushButton::pressed,this,[=](){
        qDebug() << "点击读取存档";
        QRect rect = readbtn->geometry();
        readbtnanimation->setStartValue(rect);
        readbtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.025, rect.y() + rect.height() * 0.025,
                                            rect.width() * 0.95, rect.height() * 0.95));
        readbtnanimation->start();
    });
    connect(readbtn,&QPushButton::released,this,[=]() {
        readbtnanimation->setStartValue(readbtn->geometry());
        readbtnanimation->setEndValue(readbtnGeometry);
        readbtnanimation->start();

        QTimer::singleShot(500,this,[=](){
            QString filename = QFileDialog::getOpenFileName(this, tr("选择存档文件"), "saves", tr("JSON 文件 (*.json)"));

            if (!filename.isEmpty()) {
                loadGameAndSwitchToGameScene(filename);
            } else {
                QMessageBox::warning(this, "警告", "未选择存档文件");
            }
        });
    });


    QPropertyAnimation *continuebtnanimation = new QPropertyAnimation(continuebtn, "geometry");
    continuebtnanimation->setDuration(100);
    continuebtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect continuebtnGeometry = continuebtn->geometry();
    connect(continuebtn,&QPushButton::pressed,this,[=](){
        qDebug() << "点击继续游戏";
        QRect rect = continuebtn->geometry();
        continuebtnanimation->setStartValue(rect);
        continuebtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.025, rect.y() + rect.height() * 0.025,
                                                rect.width() * 0.95, rect.height() * 0.95));
        continuebtnanimation->start();
    });
    connect(continuebtn,&QPushButton::released,this,[=]() {
        continuebtnanimation->setStartValue(continuebtn->geometry());
        continuebtnanimation->setEndValue(continuebtnGeometry);
        continuebtnanimation->start();

        QTimer::singleShot(500,this,[=](){
            QString defaultSaveFile = "auto_save/auto_save.json";
            QFile file(defaultSaveFile);

            if (file.exists()) {
                loadGameAndSwitchToGameScene(defaultSaveFile);
            } else {
                QMessageBox::warning(this, "继续游戏", "未找到存档文件，请开始新游戏或加载其他存档。");
            }
        });
    });

    connect(getonsteambtn,&QPushButton::clicked,this,[](){
        qDebug() << "点击get_on_steam图标";
        QUrl url("https://store.steampowered.com/app/1318690/Shapez/");
        QDesktopServices::openUrl(url);
    });

    connect(githubbtn,&QPushButton::clicked,this,[](){
        qDebug() << "点击github图标";
        QUrl url("https://github.com/heyifan142857/shapez");
        QDesktopServices::openUrl(url);
    });
}

void Mainscene::loadGameAndSwitchToGameScene(const QString& filename) {
    gamescene = new Gamescene;

    gamescene->loadGame(filename);

    player->pause();

    this->hide();
    gamescene->setGeometry(this->geometry());
    gamescene->show();

    connect(gamescene, &Gamescene::returnToMain, this, [this]() {
        this->show();
        player->play();
        gamescene = nullptr;
        qDebug() << "当前在主界面";
    });

    qDebug() << "当前在游戏界面";
}

void Mainscene::loadGameAndSwitchToGameScene() {

    gamescene = new Gamescene;

    gamescene->autoLoadGame("auto_save.json");

    player->pause();

    this->hide();
    gamescene->setGeometry(this->geometry());
    gamescene->show();

    connect(gamescene, &Gamescene::returnToMain, this, [this]() {
        this->show();
        player->play();
        gamescene = nullptr;
        qDebug() << "当前在主界面";
    });

    qDebug() << "当前在游戏界面";
}

Mainscene::~Mainscene()
{
    delete gamescene;
    delete ui;
}

void Mainscene::paintEvent(QPaintEvent *){
    QPainter painter(this);

    QPixmap pix;
    pix.load(":/res/mainbackground.png");
    painter.drawPixmap(0,0,this->width(),this->height(),pix);

    QBrush brush(QColor(255,255,255));
    painter.setBrush(brush);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(500,350,600,300,10,10);

    pix.load(":/res/logo.png");
    painter.drawPixmap(450,60,pix);
}
