#include "item.h"
#include "tile.h"
#include <QPainter>
#include <QColor>
#include <QDebug>

Item::Item(int part1,int part2,int part3,int part4,std::pair<int,int> pos):
    part1(part1),part2(part2),part3(part3),part4(part4),
    pos(pos),label(nullptr),cuttable(true){

}

Item::Item(QString mine,std::pair<int,int> pos):label(nullptr),pos(pos){

    if(mine == "square"){
        cuttable = true;
        part1 = SQUARE;
        part2 = SQUARE;
        part3 = SQUARE;
        part4 = SQUARE;
    }else if(mine == "circle"){
        cuttable = false;
        part1 = CIRCLE;
        part2 = CIRCLE;
        part3 = CIRCLE;
        part4 = CIRCLE;
    }else{
        qDebug() << "wrong name of mine";
        cuttable = false;
        part1 = EMPTY;
        part2 = EMPTY;
        part3 = EMPTY;
        part4 = EMPTY;
    }
}

Item::Item(const Item& other){
    part1 = other.part1;
    part2 = other.part2;
    part3 = other.part3;
    part4 = other.part4;
    pos = other.pos;
    cuttable = other.cuttable;

    label = nullptr;
}

Item::~Item(){
    if(label){
        delete label;
        label = nullptr;
    }
}

QPixmap Item::getPixmap() {
    int size = 16; // 每个小部分的大小

    QPixmap circle(TILESIZE, TILESIZE);
    circle.fill(Qt::transparent);
    QPixmap square(TILESIZE, TILESIZE);
    square.fill(Qt::transparent);
    QPen pen(Qt::black);
    pen.setWidth(2);
    QRectF rectangle(9, 9, 2*size, 2*size);

    QPainter painter1(&circle);
    painter1.setRenderHint(QPainter::Antialiasing);
    painter1.setPen(pen);
    painter1.setBrush(QColor("#9EA1A3"));
    painter1.drawEllipse(rectangle);
    painter1.drawLine(9, size+9, 2*size+9, size+9);
    painter1.drawLine(size+9, 9, size+9, 2*size+9);

    QPainter painter2(&square);
    painter2.setRenderHint(QPainter::Antialiasing);
    painter2.setPen(pen);
    painter2.setBrush(QColor("#9EA1A3"));
    painter2.drawRect(rectangle);
    painter2.drawLine(9, size+9, 2*size+9, size+9);
    painter2.drawLine(size+9, 9, size+9, 2*size+9);

    QPixmap pixmap1;
    QPixmap pixmap2;
    QPixmap pixmap3;
    QPixmap pixmap4;

    auto cutShape = [=](QPixmap &pixmap, int type ,int part) {
        QRect cropRect((part%2)*(TILESIZE/2), (part/2)*(TILESIZE/2), (TILESIZE/2), (TILESIZE/2));
        if (type == SQUARE){
            pixmap = square.copy(cropRect);
        }
        if (type == CIRCLE){
            pixmap = circle.copy(cropRect);
        }
        if (type == EMPTY){
            pixmap.scaled((TILESIZE/2),(TILESIZE/2));
            pixmap.fill(Qt::transparent);
        }
    };

    cutShape(pixmap1,part1,0);
    cutShape(pixmap2,part2,1);
    cutShape(pixmap3,part3,2);
    cutShape(pixmap4,part4,3);

    // 创建组合图像
    QPixmap combinedPixmap(TILESIZE, TILESIZE);
    combinedPixmap.fill(Qt::transparent);

    QPainter combinedPainter(&combinedPixmap);
    combinedPainter.drawPixmap(0, 0, pixmap1); // 左上角
    combinedPainter.drawPixmap(TILESIZE/2, 0, pixmap2); // 右上角
    combinedPainter.drawPixmap(0, TILESIZE/2, pixmap3); // 左下角
    combinedPainter.drawPixmap(TILESIZE/2, TILESIZE/2, pixmap4); // 右下角

    return combinedPixmap;
}

QPixmap Item::drawSquare(){
    int size = 16;

    QPixmap square(TILESIZE, TILESIZE);
    square.fill(Qt::transparent);
    QPen pen(Qt::black);
    pen.setWidth(2);
    QRectF rectangle(9, 9, 2*size, 2*size);

    QPainter painter(&square);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(pen);
    painter.setBrush(QColor("#9EA1A3"));
    painter.drawRect(rectangle);
    painter.drawLine(9, size+9, 2*size+9, size+9);
    painter.drawLine(size+9, 9, size+9, 2*size+9);

    return square;
}
QPixmap Item::drawCircle(){
    int size = 16;

    QPixmap circle(TILESIZE, TILESIZE);
    circle.fill(Qt::transparent);
    QPen pen(Qt::black);
    pen.setWidth(2);
    QRectF rectangle(9, 9, 2*size, 2*size);

    QPainter painter(&circle);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(pen);
    painter.setBrush(QColor("#9EA1A3"));
    painter.drawEllipse(rectangle);
    painter.drawLine(9, size+9, 2*size+9, size+9);
    painter.drawLine(size+9, 9, size+9, 2*size+9);

    return circle;
}

QPixmap drawPixmap(int part1, int part2, int part3, int part4, int pixmapSize) {
    const int IMAGE_SIZE = pixmapSize;
    const int PART_SIZE = IMAGE_SIZE / 2;
    const int BASE_SIZE = 16;
    const int BASE_OFFSET = 9;

    int size = pixmapSize / BASE_SIZE * 16;
    int offset = pixmapSize / BASE_SIZE * 9;

    auto drawShape = [&](QPixmap &pixmap, bool isCircle) {
        QPen pen(Qt::black);
        pen.setWidth(2);
        QRectF rectangle(offset, offset, size, size);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(pen);
        painter.setBrush(QColor("#9EA1A3"));

        if (isCircle) {
            painter.drawEllipse(rectangle);
        } else {
            painter.drawRect(rectangle);
        }

        painter.drawLine(offset, size / 2 + offset, size + offset, size / 2 + offset);
        painter.drawLine(size / 2 + offset, offset, size / 2 + offset, size + offset);
    };

    QPixmap circle(IMAGE_SIZE, IMAGE_SIZE);
    circle.fill(Qt::transparent);
    drawShape(circle, true);

    QPixmap square(IMAGE_SIZE, IMAGE_SIZE);
    square.fill(Qt::transparent);
    drawShape(square, false);

    auto cutShape = [=](const QPixmap &source, int part) {
        QRect cropRect((part % 2) * PART_SIZE, (part / 2) * PART_SIZE, PART_SIZE, PART_SIZE);
        return source.copy(cropRect);
    };

    QPixmap pixmap1 = (part1 == EMPTY) ? QPixmap(PART_SIZE, PART_SIZE) : cutShape(part1 == CIRCLE ? circle : square, 0);
    QPixmap pixmap2 = (part2 == EMPTY) ? QPixmap(PART_SIZE, PART_SIZE) : cutShape(part2 == CIRCLE ? circle : square, 1);
    QPixmap pixmap3 = (part3 == EMPTY) ? QPixmap(PART_SIZE, PART_SIZE) : cutShape(part3 == CIRCLE ? circle : square, 2);
    QPixmap pixmap4 = (part4 == EMPTY) ? QPixmap(PART_SIZE, PART_SIZE) : cutShape(part4 == CIRCLE ? circle : square, 3);

    QPixmap combinedPixmap(IMAGE_SIZE, IMAGE_SIZE);
    combinedPixmap.fill(Qt::transparent);

    QPainter combinedPainter(&combinedPixmap);
    combinedPainter.drawPixmap(0, 0, pixmap1); // 左上角
    combinedPainter.drawPixmap(PART_SIZE, 0, pixmap2); // 右上角
    combinedPainter.drawPixmap(0, PART_SIZE, pixmap3); // 左下角
    combinedPainter.drawPixmap(PART_SIZE, PART_SIZE, pixmap4); // 右下角

    return combinedPixmap;
}

bool Item::ableToConbine(Item other){
    if((part1!=EMPTY&&other.part1!=EMPTY)||(part2!=EMPTY&&other.part2!=EMPTY)||(part3!=EMPTY&&other.part3!=EMPTY)||(part4!=EMPTY&&other.part4!=EMPTY)){
        return false;
    }else{
        return true;
    }
}

bool Item::isCuttable(){
    return cuttable;
};

Item Item::operator+(const Item& other) const {
    Item result;
    result.part1 = this->part1 + other.part1;
    result.part2 = this->part2 + other.part2;
    result.part3 = this->part3 + other.part3;
    result.part4 = this->part4 + other.part4;
    return result;
}

//stragedy = 0,横着切;stragedy = 1,竖着切
std::pair<Item*,Item*> Item::cutItem(int stragedy){
    if(stragedy == 0){
        Item* item1 = new Item(part1,part2,EMPTY,EMPTY);
        Item* item2 = new Item(EMPTY,EMPTY,part3,part4);
        return std::make_pair(item1,item2);
    }else if(stragedy == 1){
        Item* item1 = new Item(part1,EMPTY,part3,EMPTY);
        Item* item2 = new Item(EMPTY,part2,EMPTY,part4);
        return std::make_pair(item1,item2);
    }else{
        qDebug() << "undefined stragedy";
        Item* item1 = new Item(*this);
        Item* item2 = new Item(*this);
        return std::make_pair(item1,item2);
    }
}
