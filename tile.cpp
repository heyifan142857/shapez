#include "tile.h"

Tile::Tile(Type type, const QString& name, int direction):
    type(type), name(name), direction(direction){
    QPixmap pixmap;
    QPixmap scaledpixmap;
    int w = 0;
    int h = 0;

    QTransform transform;
    int angle;
    switch (direction) {
    case NORTH:
        angle = 0;
        break;
    case WEST:
        angle = 270;
        break;
    case SOUTH:
        angle = 180;
        break;
    case EAST:
        angle = 90;
        break;
    default:
        angle = 0;
        qWarning() << "undefined direction when construct a new Tile";
        break;
    }
    transform.translate(TILESIZE / 2, TILESIZE / 2); // 设置旋转中心点
    transform.rotate(angle);
    transform.translate(-TILESIZE / 2, -TILESIZE / 2); // 恢复原点

    switch (type) {
    case Tile::Type::Building:
        if (!pixmap.load(QString(":/res/buildings/%1.png").arg(name))) {
            qWarning() << "Failed to load image:" << QString(":/res/buildings/%1.png").arg(name);
            QPixmap wrongPixmap(TILESIZE, TILESIZE);
            wrongPixmap.fill(Qt::white);
            image = scaledpixmap;
            break;
        }
        w = pixmap.width();
        h = pixmap.height();
        w = w / 192 * TILESIZE;
        h = h / 192 * TILESIZE;
        scaledpixmap = pixmap.scaled(w, h, Qt::KeepAspectRatio).transformed(transform);
        image = scaledpixmap;
        break;
    case Tile::Type::Color:
        if (!pixmap.load(QString(":/res/colors/%1.png").arg(name))) {
            qWarning() << "Failed to load image:" << QString(":/res/colors/%1.png").arg(name);
            QPixmap wrongPixmap(TILESIZE, TILESIZE);
            wrongPixmap.fill(Qt::white);
            image = scaledpixmap;
            break;
        }
        w = pixmap.width();
        h = pixmap.height();
        w = w / 192 * TILESIZE;
        h = h / 192 * TILESIZE;
        scaledpixmap = pixmap.scaled(w, h, Qt::KeepAspectRatio).transformed(transform);
        image = scaledpixmap;
        break;
    case Tile::Type::Resource:
        qDebug() << "Tile::Type::Resource unfinish";
        pixmap.fill(Qt::white);
        image = pixmap.scaled(TILESIZE, TILESIZE);
        break;
    default:
        qDebug() << "undefined type";
        QPixmap wrongPixmap(TILESIZE, TILESIZE);
        wrongPixmap.fill(Qt::white);
        image = wrongPixmap;
        break;
    }
}

//专门为Belt而写的构造函数，因为只有Belt会动
Tile::Tile(Type type, int direction, QString state):
    type(type), direction(direction), state(state),
    frameIndex(0),label(nullptr){
    //根据direction设置旋转角度
    QTransform transform;
    int angle;
    switch (direction) {
    case NORTH:
        angle = 0;
        break;
    case WEST:
        angle = 270;
        break;
    case SOUTH:
        angle = 180;
        break;
    case EAST:
        angle = 90;
        break;
    default:
        angle = 0;
        qWarning() << "undefined direction when construct a new Tile";
        break;
    }
    transform.translate(TILESIZE / 2, TILESIZE / 2); // 设置旋转中心点
    transform.rotate(angle);
    transform.translate(-TILESIZE / 2, -TILESIZE / 2); // 恢复原点

    for (int i = 0; i < 14; ++i) {
        QPixmap pixmap;
        if (!pixmap.load(QString(":/res/belt/%1_%2.png").arg(state).arg(i))) {
            qWarning() << "Failed to load image:" << QString(":/res/belt/%1_%2.png").arg(state).arg(i);
            continue;  // 忽略加载失败的图像
        }
        QPixmap scaledpixmap = pixmap.scaled(TILESIZE, TILESIZE, Qt::KeepAspectRatio).transformed(transform);
        images.push_back(scaledpixmap);
    }
}
