#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QWidget>
#include <QIcon>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include "Map.h"

class Gamescene : public QWidget
{
    Q_OBJECT
public:
    explicit Gamescene(QWidget *parent = nullptr);

    void setPuzzle();

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void paintEvent(QPaintEvent *) override;

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

    QFont font;

    bool isDragging;

    QTimer* itemMoveTimer;
    QTimer* minerTimer;

signals:
};

#endif // GAMESCENE_H
