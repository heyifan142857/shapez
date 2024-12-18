#include "gamescene.h"

Gamescene::Gamescene(QWidget *parent)
    : QWidget{parent}
{
    //编辑窗口基本信息
    setFixedSize(1600,900);
    setWindowIcon(QIcon(":/res/icon.ico"));
    setWindowTitle("Shapez");

    Belt *belt = new Belt(NORTH,"forward",0,0,this);
}

void Gamescene::paintEvent(QPaintEvent *){}
