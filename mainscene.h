#ifndef MAINSCENE_H
#define MAINSCENE_H

#include <QWidget>
#include <QIcon>
#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QPropertyAnimation>
#include <QTimer>
#include <QMediaPlayer>
#include <QAudioOutput>
#include "gamescene.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainScene;
}
QT_END_NAMESPACE

class Mainscene : public QWidget
{
    Q_OBJECT

public:
    Mainscene(QWidget *parent = nullptr);
    ~Mainscene();

    void paintEvent(QPaintEvent *);

    Gamescene * _gamescene = NULL;


private:
    QMediaPlayer *player;
    QAudioOutput *audioOutput;
    Gamescene *gamescene = nullptr;
private:
    Ui::MainScene *ui;
};
#endif // MAINSCENE_H
