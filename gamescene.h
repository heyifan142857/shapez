#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QWidget>
#include <QIcon>
#include <QLabel>
#include <QMouseEvent>
#include "Map.h"

class Gamescene : public QWidget
{
    Q_OBJECT
public:
    explicit Gamescene(QWidget *parent = nullptr);

    void mousePressEvent(QMouseEvent *event) override;

    //void mouseMoveEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void paintEvent(QPaintEvent *) override;

    Map * map = nullptr;

    bool isPlaceItem;
    Tile* currentTile;

private:
    //QLabel *imageLabel;

signals:
};

#endif // GAMESCENE_H
