#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QWidget>
#include <QIcon>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QSoundEffect>
#include "Map.h"

class Gamescene : public QWidget
{
    Q_OBJECT
public:
    explicit Gamescene(QWidget *parent = nullptr);

    ~Gamescene();

    void setPuzzle();

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void paintEvent(QPaintEvent *) override;

    void saveGame(const QString& filename);

    void loadGame(const QString& filename);

    void autoSaveGame(const QString& filename);

    void autoLoadGame(const QString& filename);

    void returnToMainScene();

    void upgrateMine();
    Map * map = nullptr;

    bool isPlaceItem;
    Tile* currentTile;

private:
    //QLabel * test;
    int defaultBeltDirection;

    QPushButton * beltbtn;
    QPushButton * balancerbtn;
    QPushButton * underground_beltbtn;
    QPushButton * minerbtn;
    QPushButton * cutterbtn;
    QPushButton * rotaterbtn;
    QPushButton * stackerbtn;
    QPushButton * mixerbtn;
    QPushButton * painterbtn;
    QPushButton * trashbtn;

    QSoundEffect ui_clickEffect;

    QFont font;

    bool isDragging;

    QTimer* itemMoveTimer;
    QTimer* minerTimer;
    QTimer* cutterTimer;

    int itemMoveTimerIntervalUpgrate = 400;
    int minerTimerIntervalUpgrate = 1600;
    int cutterTimerIntervalUpgrate = 3200;

    bool itemMoveUpgrate = false;
    bool minerUpgrate = false;
    bool cutterUpgrate = false;

signals:
    void returnToMain();
};

#endif // GAMESCENE_H
