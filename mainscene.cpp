#include "mainscene.h"
#include <QGuiApplication>
#include <QMessageBox>
#include <QScreen>
#include "configmanager.h"
#include "localization.h"
#include "./ui_mainscene.h"
Mainscene::Mainscene(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainScene)
{
    ui->setupUi(this);

    //编辑窗口信息
    const QRect availableGeometry = QGuiApplication::primaryScreen()
        ? QGuiApplication::primaryScreen()->availableGeometry()
        : QRect(0, 0, 1600, 900);
    const QSize preferredSize(1600, 900);
    resize(preferredSize.boundedTo(availableGeometry.size()));
    setMinimumSize(QSize(qMin(960, availableGeometry.width()), qMin(540, availableGeometry.height())));
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

    ConfigManager config;
    languageCode = config.getLanguage();

    //添加各种按钮
    newbtn = new QPushButton(this);
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

    readbtn = new QPushButton(this);
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

    continuebtn = new QPushButton(this);
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

    steambtn = new QPushButton();
    steambtn->setParent(this);
    steambtn->setIconSize(QSize(600,145));
    steambtn->setIcon(QIcon(":/get_on_steam.png"));
    steambtn->setStyleSheet(
        "QPushButton {"
        " border: none;"
        " background-color: transparent;"
        " padding: 0;"
        " margin: 0;"
        "}"
        );

    githubbtn = new QPushButton();
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
    languagebtn = new QPushButton(this);
    languagebtn->setFixedSize(80, 80);
    languagebtn->setIconSize(QSize(54,54));
    languagebtn->setStyleSheet(
        "QPushButton {"
        " border: none;"
        " background-color: white;"
        " border-radius: 40px;"
        " padding: 0;"
        " margin: 0;"
        "}"
    );
    updateLanguageButtonIcon();
    applyLanguage();
    updateInterfaceLayout();

    QPropertyAnimation *newbtnanimation = new QPropertyAnimation(newbtn, "geometry");
    newbtnanimation->setDuration(100);
    newbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect newbtnGeometry = newbtn->geometry();
    connect(newbtn,&QPushButton::pressed,this,[=](){
        qDebug() << "New Game clicked";
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
                qDebug() << "Main menu is active";
            });
            gamescene->show();
            qDebug() << "Game scene is active";
        });
    });

    QPropertyAnimation *readbtnanimation = new QPropertyAnimation(readbtn, "geometry");
    readbtnanimation->setDuration(100);
    readbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect readbtnGeometry = readbtn->geometry();
    connect(readbtn,&QPushButton::pressed,this,[=](){
        qDebug() << "Load Save clicked";
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
            QString filename = QFileDialog::getOpenFileName(
                this,
                t("选择存档文件", "Select Save File"),
                "saves",
                t("JSON 文件 (*.json)", "JSON Files (*.json)")
            );

            if (!filename.isEmpty()) {
                loadGameAndSwitchToGameScene(filename);
            } else {
                QMessageBox::warning(this, t("警告", "Warning"), t("未选择存档文件", "No save file was selected."));
            }
        });
    });


    QPropertyAnimation *continuebtnanimation = new QPropertyAnimation(continuebtn, "geometry");
    continuebtnanimation->setDuration(100);
    continuebtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect continuebtnGeometry = continuebtn->geometry();
    connect(continuebtn,&QPushButton::pressed,this,[=](){
        qDebug() << "Continue clicked";
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
                QMessageBox::warning(
                    this,
                    t("继续游戏", "Continue"),
                    t("未找到存档文件，请开始新游戏或加载其他存档。", "No save file was found. Start a new game or load another save.")
                );
            }
        });
    });

    connect(steambtn,&QPushButton::clicked,this,[](){
        qDebug() << "Steam icon clicked";
        QUrl url("https://store.steampowered.com/app/1318690/Shapez/");
        QDesktopServices::openUrl(url);
    });

    connect(githubbtn,&QPushButton::clicked,this,[](){
        qDebug() << "GitHub icon clicked";
        QUrl url("https://github.com/heyifan142857/shapez");
        QDesktopServices::openUrl(url);
    });

    connect(languagebtn, &QPushButton::clicked, this, &Mainscene::toggleLanguage);
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
        qDebug() << "Main menu is active";
    });

    qDebug() << "Game scene is active";
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
        qDebug() << "Main menu is active";
    });

    qDebug() << "Game scene is active";
}

Mainscene::~Mainscene()
{
    delete gamescene;
    delete ui;
}

void Mainscene::applyLanguage()
{
    newbtn->setText(t("新游戏", "New Game"));
    readbtn->setText(t("读取存档", "Load Save"));
    continuebtn->setText(t("继续游戏", "Continue"));
    languagebtn->setToolTip(t("切换语言", "Switch Language"));
}

void Mainscene::toggleLanguage()
{
    languageCode = Localization::isEnglish(languageCode) ? "zh-CN" : "en";

    ConfigManager config;
    config.setLanguage(languageCode);

    updateLanguageButtonIcon();
    applyLanguage();
}

QString Mainscene::t(const QString &zhText, const QString &enText) const
{
    return Localization::text(languageCode, zhText, enText);
}

void Mainscene::updateLanguageButtonIcon()
{
    const QString iconPath = Localization::isEnglish(languageCode)
        ? QString(":/res/languages/en.svg")
        : QString(":/res/languages/zh-CN.svg");
    languagebtn->setIcon(QIcon(iconPath));
}

void Mainscene::updateInterfaceLayout()
{
    const qreal scale = qMin(width() / 1600.0, height() / 900.0);
    const int contentWidth = qRound(1600 * scale);
    const int contentHeight = qRound(900 * scale);
    const int offsetX = (width() - contentWidth) / 2;
    const int offsetY = (height() - contentHeight) / 2;

    auto scaleRect = [scale, offsetX, offsetY](int x, int y, int w, int h) {
        return QRect(
            offsetX + qRound(x * scale),
            offsetY + qRound(y * scale),
            qRound(w * scale),
            qRound(h * scale)
        );
    };

    const int mainFontPx = qMax(12, qRound(16 * scale));
    const int continueFontPx = qMax(14, qRound(20 * scale));

    QFont mainFont("Microsoft YaHei", mainFontPx, QFont::Normal);
    newbtn->setFont(mainFont);
    readbtn->setFont(mainFont);

    QFont continueFont("Microsoft YaHei", continueFontPx, QFont::Normal);
    continuebtn->setFont(continueFont);

    newbtn->setGeometry(scaleRect(812, 424, 264, 66));
    readbtn->setGeometry(scaleRect(812, 502, 264, 60));
    continuebtn->setGeometry(scaleRect(524, 424, 264, 144));

    steambtn->setGeometry(scaleRect(500, 725, 600, 145));
    steambtn->setIconSize(steambtn->size());

    githubbtn->setGeometry(scaleRect(40, 780, 80, 80));
    githubbtn->setIconSize(QSize(qRound(54 * scale), qRound(54 * scale)));

    languagebtn->setGeometry(scaleRect(130, 780, 80, 80));
    languagebtn->setIconSize(QSize(qRound(54 * scale), qRound(54 * scale)));
}

void Mainscene::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateInterfaceLayout();
}

void Mainscene::paintEvent(QPaintEvent *){
    QPainter painter(this);

    const qreal scale = qMin(width() / 1600.0, height() / 900.0);
    const int contentWidth = qRound(1600 * scale);
    const int contentHeight = qRound(900 * scale);
    const int offsetX = (width() - contentWidth) / 2;
    const int offsetY = (height() - contentHeight) / 2;

    QPixmap pix;
    pix.load(":/res/mainbackground.png");
    painter.drawPixmap(0,0,this->width(),this->height(),pix);

    QBrush brush(QColor(255,255,255));
    painter.setBrush(brush);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(
        offsetX + qRound(500 * scale),
        offsetY + qRound(350 * scale),
        qRound(600 * scale),
        qRound(300 * scale),
        qRound(10 * scale),
        qRound(10 * scale)
    );

    pix.load(":/res/logo.png");
    const QSize logoSize(qRound(pix.width() * scale), qRound(pix.height() * scale));
    painter.drawPixmap(
        QRect(offsetX + qRound(450 * scale), offsetY + qRound(60 * scale), logoSize.width(), logoSize.height()),
        pix
    );
}
