#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QWidget>
#include <QIcon>
#include <QLabel>

class Gamescene : public QWidget
{
    Q_OBJECT
public:
    explicit Gamescene(QWidget *parent = nullptr);

    void paintEvent(QPaintEvent *);

signals:
};

#endif // GAMESCENE_H
