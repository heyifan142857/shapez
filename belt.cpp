#include "belt.h"

Belt::Belt(int direction, QString type, int x, int y, QWidget *parent):
    QWidget(parent), direction(direction), type(type), currentIndex(0)
{
    beltLabel = new QLabel(parent);
    beltLabel->setAlignment(Qt::AlignCenter);
    beltLabel->move(x,y);

    // 加载传送带图片
    for (int i = 0; i < 14; ++i) {
        QPixmap pixmap;
        if (!pixmap.load(QString(":/res/belt/%1_%2.png").arg(type).arg(i))) {
            qWarning() << "Failed to load image:" << QString(":/res/belt/%1_%2.png").arg(type).arg(i);
        }
        QPixmap scaledpixmap = pixmap.scaled(50, 50, Qt::KeepAspectRatio);
        beltImages.push_back(scaledpixmap);
    }

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Belt::updateConveyorBelt);
    timer->start(20);
}

Belt::~Belt(){}

void Belt::updateConveyorBelt()
{
    // 每次定时器超时时，更新显示的图片
    beltLabel->setPixmap(beltImages[currentIndex]);

    // 更新索引，实现图片循环
    currentIndex = (currentIndex + 1) % beltImages.size();
}
