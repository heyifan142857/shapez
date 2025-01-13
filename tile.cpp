#include "tile.h"

Belt::Belt(int direction, QString category):direction(direction),category(category),frameIndex(0){
        //根据direction设置旋转角度
        QTransform transform;
        int angle;
        switch (direction) {
        case NORTH:
            angle = 0;
            break;
        case WEST:
            angle = 90;
            break;
        case SOUTH:
            angle = 180;
            break;
        case EAST:
            angle = 270;
            break;
        default:
            angle = 0;
            qWarning() << "undefined direction when construct a new Tile";
            break;
        }
        transform.rotate(angle);

        for (int i = 0; i < 14; ++i) {
            QPixmap pixmap;
            if (!pixmap.load(QString(":/res/belt/%1_%2.png").arg(category).arg(i))) {
                qWarning() << "Failed to load image:" << QString(":/res/belt/%1_%2.png").arg(category).arg(i);
                continue;  // 忽略加载失败的图像
            }
            QPixmap scaledpixmap = pixmap.scaled(TILESIZE, TILESIZE, Qt::KeepAspectRatio).transformed(transform);
            images.push_back(scaledpixmap);
        }
};
