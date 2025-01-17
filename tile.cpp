#include <QPainter>
#include "tile.h"
#include "item.h"

Tile::Tile(Type type, int direction, const QString& name, std::pair<int,int> size):
    type(type), name(name), direction(direction), label(nullptr), size(size){
    QPixmap pixmap;
    QPixmap scaledpixmap;
    int w = 0;
    int h = 0;
    int f = size.first;
    int s = size.second;

    QTransform transform;
    int angle;
    switch (direction) {
    case NORTH:
        angle = 0;
        break;
    case WEST:
        angle = 270;
        this->size.first = s;
        this->size.second = f;
        break;
    case SOUTH:
        angle = 180;
        break;
    case EAST:
        angle = 90;
        this->size.first = s;
        this->size.second = f;
        break;
    default:
        angle = 0;
        qWarning() << "undefined direction when construct a new Tile";
        break;
    }
    transform.translate(0, 0); // 设置旋转中心点为左上角
    transform.rotate(angle);

    switch (type) {
    case Tile::Type::Building:
        if (!pixmap.load(QString(":/res/buildings/%1.png").arg(name))) {
            qWarning() << "Failed to load image:" << QString(":/res/buildings/%1.png").arg(name);
            QPixmap wrongPixmap(TILESIZE, TILESIZE);
            wrongPixmap.fill(Qt::white);
            image = wrongPixmap;
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
            image = wrongPixmap;
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
        if(name == "circle"){
            image = Item().drawCircle();
        }else if(name == "square"){
            image = Item().drawSquare();
        }else{
            qDebug() << "undefined resource";
        }
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
Tile::Tile(Type type, QString state, int direction):
    type(type), direction(direction), state(state),
    label(nullptr),size(std::make_pair(1,1)){
    if(type != Tile::Type::Belt){
        qDebug() << "Not belt";
    }
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
    transform.translate(0, 0); // 设置旋转中心点为左上角
    transform.rotate(angle);

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

//Hub构造函数
Tile::Tile(Type type, QString name):type(type), name(name),label(nullptr), size(std::make_pair(4,4)){
    QPixmap pixmap;
    if (!pixmap.load(QString(":/res/buildings/hub.png"))) {
        qWarning() << "Failed to load image:" << QString(":/res/buildings/hub.png");
        QPixmap wrongPixmap(TILESIZE, TILESIZE);
        wrongPixmap.fill(Qt::white);
        image = wrongPixmap;
    }
    int w = pixmap.width();
    int h = pixmap.height();
    w = w / 192 * TILESIZE;
    h = h / 192 * TILESIZE;
    QPixmap scaledpixmap = pixmap.scaled(w, h, Qt::KeepAspectRatio);
    image = scaledpixmap;
};

void Tile::changeDirection(){
    QTransform transform;
    int angle = 90;
    transform.translate(0, 0); // 设置旋转中心点为左上角
    transform.rotate(angle);

    int f = size.first;
    int s = size.second;
    size.first = s;
    size.second = f;

    direction = (direction+1)%4;

    if(type == Tile::Type::Belt){
        for (int i = 0; i < 14; ++i) {
            images[i] = images[i].transformed(transform);
        }
    }else{
        image = image.transformed(transform);
    }
}

void Tile::changeState(){
    if(type == Tile::Type::Belt){
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
        transform.translate(0, 0); // 设置旋转中心点为左上角
        transform.rotate(angle);

        if(state == "forward"){
            state = "left";
        }else if(state == "left"){
            state = "right";
        }else if(state == "right"){
            state = "forward";
        }else{
            qDebug() << "fail to rotate";
        }

        for (int i = 0; i < 14; ++i) {
            QPixmap pixmap;
            if (!pixmap.load(QString(":/res/belt/%1_%2.png").arg(state).arg(i))) {
                qWarning() << "Failed to load image:" << QString(":/res/belt/%1_%2.png").arg(state).arg(i);
                continue;  // 忽略加载失败的图像
            }
            QPixmap scaledpixmap = pixmap.scaled(TILESIZE, TILESIZE, Qt::KeepAspectRatio).transformed(transform);
            images[i] = scaledpixmap;
        }
    }
};

Tile::~Tile(){
    if (label) {
        delete label;
        label = nullptr;
    }
    if(father){
        delete father;
        father = nullptr;
    }
    sons.clear();
}

Tile::Tile(const Tile& other) {
    type = other.type;
    direction = other.direction;
    state = other.state;
    name = other.name;
    images = other.images;
    image = other.image;
    label = nullptr;
    size = other.size;
    father = nullptr;
    sons.clear();
    mine = other.mine;
}
