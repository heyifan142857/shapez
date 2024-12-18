#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QWidget>
#include <QIcon>
#include <QLabel>
#include "belt.h"

class Gamescene : public QWidget
{
    Q_OBJECT
public:
    explicit Gamescene(QWidget *parent = nullptr);

    void paintEvent(QPaintEvent *);

signals:
};

#endif // GAMESCENE_H
