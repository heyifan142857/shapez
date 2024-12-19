#ifndef BELT_H
#define BELT_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include "map.h"
#define NORTH 0
#define SOUTH 1
#define WEST 2
#define EAST 3

class Belt:public QWidget
{
    Q_OBJECT
private:
    int direction;
    QString type;
    int x,y;
    QLabel * beltLabel;
    QVector<QPixmap> beltImages;
    int currentIndex;

public:
    explicit Belt(int direction, QString type, int x, int y, QWidget *parent = nullptr);

    ~Belt();

private slots:
    void updateConveyorBelt();
};

#endif // BELT_H
