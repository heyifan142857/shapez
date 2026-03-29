#include "item.h"
#include "tile.h"

#include <QDebug>
#include <QPainter>

namespace {

QString defaultPartColor(int part)
{
    return part == EMPTY ? QString() : QStringLiteral("uncolored");
}

QString normalizedPaintName(const QString &name)
{
    return name.isEmpty() ? QStringLiteral("uncolored") : name;
}

QColor mappedColorInternal(const QString &name)
{
    if (name == "red") {
        return QColor("#FF666A");
    }
    if (name == "blue") {
        return QColor("#66A7FF");
    }
    if (name == "yellow") {
        return QColor("#FCF52A");
    }
    if (name == "green") {
        return QColor("#78FF66");
    }
    if (name == "purple") {
        return QColor("#DD66FF");
    }
    if (name == "orange") {
        return QColor("#FDB34A");
    }
    if (name == "cyan") {
        return QColor("#66FFF7");
    }
    if (name == "white") {
        return QColor("#F6F7FB");
    }

    return QColor("#9EA1A3");
}

QPixmap drawShapePixmapInternal(int type, const QString &colorName, int pixmapSize)
{
    QPixmap pixmap(pixmapSize, pixmapSize);
    pixmap.fill(Qt::transparent);
    if (type == EMPTY) {
        return pixmap;
    }

    const int size = qMax(1, pixmapSize * 16 / TILESIZE);
    const int offset = qMax(1, pixmapSize * 9 / TILESIZE);
    const int penWidth = qMax(2, pixmapSize * 2 / TILESIZE);

    QPen pen(QColor("#404040"));
    pen.setWidth(penWidth);
    const QRectF rectangle(offset, offset, 2 * size, 2 * size);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(pen);
    painter.setBrush(mappedColorInternal(normalizedPaintName(colorName)));

    if (type == SQUARE) {
        painter.drawRect(rectangle);
    } else if (type == CIRCLE) {
        painter.drawEllipse(rectangle);
    } else if (type == DIAMOND) {
        const QPointF top(offset + size, offset);
        const QPointF right(offset + 2 * size, offset + size);
        const QPointF bottom(offset + size, offset + 2 * size);
        const QPointF left(offset, offset + size);

        QPolygonF diamondShape;
        diamondShape << top << right << bottom << left;
        painter.drawPolygon(diamondShape);
    }

    painter.drawLine(offset, offset + size, offset + 2 * size, offset + size);
    painter.drawLine(offset + size, offset, offset + size, offset + 2 * size);
    return pixmap;
}

QPixmap drawDyePixmapInternal(const QString &colorName, int pixmapSize)
{
    QPixmap source;
    if (!source.load(QString(":/res/colors/%1.png").arg(colorName))) {
        QPixmap fallback(pixmapSize, pixmapSize);
        fallback.fill(Qt::transparent);
        return fallback;
    }

    QPixmap pixmap(pixmapSize, pixmapSize);
    pixmap.fill(Qt::transparent);

    const int dyeSize = qMax(1, qRound(pixmapSize * 0.6));
    const QPixmap scaled = source.scaled(dyeSize, dyeSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QPainter painter(&pixmap);
    painter.drawPixmap((pixmapSize - scaled.width()) / 2,
                       (pixmapSize - scaled.height()) / 2,
                       scaled);
    return pixmap;
}

} // namespace

Item::Item(int part1, int part2, int part3, int part4, std::pair<int, int> pos):
    part1(part1),
    part2(part2),
    part3(part3),
    part4(part4),
    part1Color(defaultPartColor(part1)),
    part2Color(defaultPartColor(part2)),
    part3Color(defaultPartColor(part3)),
    part4Color(defaultPartColor(part4)),
    dyeName(""),
    label(nullptr),
    cuttable(true),
    pos(pos)
{
}

Item::Item(QString mine, std::pair<int, int> pos):
    part1(EMPTY),
    part2(EMPTY),
    part3(EMPTY),
    part4(EMPTY),
    part1Color(""),
    part2Color(""),
    part3Color(""),
    part4Color(""),
    dyeName(""),
    label(nullptr),
    cuttable(true),
    pos(pos)
{
    if (mine == "square") {
        part1 = SQUARE;
        part2 = SQUARE;
        part3 = SQUARE;
        part4 = SQUARE;
        part1Color = part2Color = part3Color = part4Color = "uncolored";
    } else if (mine == "circle") {
        part1 = CIRCLE;
        part2 = CIRCLE;
        part3 = CIRCLE;
        part4 = CIRCLE;
        part1Color = part2Color = part3Color = part4Color = "uncolored";
    } else if (mine == "diamond") {
        part1 = DIAMOND;
        part2 = DIAMOND;
        part3 = DIAMOND;
        part4 = DIAMOND;
        part1Color = part2Color = part3Color = part4Color = "uncolored";
    } else if (isDyeName(mine)) {
        cuttable = false;
        dyeName = mine;
    } else {
        qDebug() << "wrong name of mine";
        cuttable = false;
    }
}

Item::Item(const Item &other)
{
    part1 = other.part1;
    part2 = other.part2;
    part3 = other.part3;
    part4 = other.part4;
    part1Color = other.part1Color;
    part2Color = other.part2Color;
    part3Color = other.part3Color;
    part4Color = other.part4Color;
    dyeName = other.dyeName;
    pos = other.pos;
    cuttable = other.cuttable;
    label = nullptr;
}

Item::~Item()
{
    if (label) {
        delete label;
        label = nullptr;
    }
}

bool Item::isDyeName(const QString &name)
{
    return name == "red" ||
           name == "blue" ||
           name == "yellow" ||
           name == "green" ||
           name == "purple" ||
           name == "orange" ||
           name == "cyan" ||
           name == "white";
}

QColor Item::colorForName(const QString &name)
{
    return mappedColorInternal(name);
}

bool Item::isDye() const
{
    return !dyeName.isEmpty();
}

void Item::applyColor(const QString &colorName)
{
    if (isDye()) {
        return;
    }

    const QString normalized = normalizedPaintName(colorName);
    if (part1 != EMPTY) {
        part1Color = normalized;
    }
    if (part2 != EMPTY) {
        part2Color = normalized;
    }
    if (part3 != EMPTY) {
        part3Color = normalized;
    }
    if (part4 != EMPTY) {
        part4Color = normalized;
    }
}

QPixmap Item::getPixmap()
{
    if (isDye()) {
        return drawDyePixmapInternal(dyeName, TILESIZE);
    }

    QPixmap pixmap1;
    QPixmap pixmap2;
    QPixmap pixmap3;
    QPixmap pixmap4;

    auto cutShape = [](QPixmap &pixmap, int type, const QString &colorName, int part) {
        const QRect cropRect((part % 2) * (TILESIZE / 2), (part / 2) * (TILESIZE / 2), TILESIZE / 2, TILESIZE / 2);
        if (type == EMPTY) {
            pixmap = QPixmap(TILESIZE / 2, TILESIZE / 2);
            pixmap.fill(Qt::transparent);
            return;
        }
        pixmap = drawShapePixmapInternal(type, colorName, TILESIZE).copy(cropRect);
    };

    cutShape(pixmap1, part1, part1Color, 0);
    cutShape(pixmap2, part2, part2Color, 1);
    cutShape(pixmap3, part3, part3Color, 2);
    cutShape(pixmap4, part4, part4Color, 3);

    QPixmap combinedPixmap(TILESIZE, TILESIZE);
    combinedPixmap.fill(Qt::transparent);

    QPainter combinedPainter(&combinedPixmap);
    combinedPainter.drawPixmap(0, 0, pixmap1);
    combinedPainter.drawPixmap(TILESIZE / 2, 0, pixmap2);
    combinedPainter.drawPixmap(0, TILESIZE / 2, pixmap3);
    combinedPainter.drawPixmap(TILESIZE / 2, TILESIZE / 2, pixmap4);

    return combinedPixmap;
}

QPixmap Item::drawSquare(const QString &colorName)
{
    return drawShapePixmapInternal(SQUARE, colorName, TILESIZE);
}

QPixmap Item::drawCircle(const QString &colorName)
{
    return drawShapePixmapInternal(CIRCLE, colorName, TILESIZE);
}

QPixmap Item::drawDiamond(const QString &colorName)
{
    return drawShapePixmapInternal(DIAMOND, colorName, TILESIZE);
}

QPixmap Item::drawPixmap(int part1, int part2, int part3, int part4, int pixmapSize)
{
    return drawPixmap(part1, part2, part3, part4, pixmapSize,
                      {QStringLiteral("uncolored"), QStringLiteral("uncolored"),
                       QStringLiteral("uncolored"), QStringLiteral("uncolored")});
}

QPixmap Item::drawPixmap(int part1, int part2, int part3, int part4, int pixmapSize,
                         const std::array<QString, 4> &partColors)
{
    QPixmap pixmap1;
    QPixmap pixmap2;
    QPixmap pixmap3;
    QPixmap pixmap4;

    auto cutShape = [pixmapSize, &partColors](QPixmap &pixmap, int type, int part) {
        const QRect cropRect((part % 2) * (pixmapSize / 2), (part / 2) * (pixmapSize / 2), pixmapSize / 2, pixmapSize / 2);
        if (type == EMPTY) {
            pixmap = QPixmap(pixmapSize / 2, pixmapSize / 2);
            pixmap.fill(Qt::transparent);
            return;
        }
        pixmap = drawShapePixmapInternal(type, partColors[part], pixmapSize).copy(cropRect);
    };

    cutShape(pixmap1, part1, 0);
    cutShape(pixmap2, part2, 1);
    cutShape(pixmap3, part3, 2);
    cutShape(pixmap4, part4, 3);

    QPixmap combinedPixmap(pixmapSize, pixmapSize);
    combinedPixmap.fill(Qt::transparent);

    QPainter combinedPainter(&combinedPixmap);
    combinedPainter.drawPixmap(0, 0, pixmap1);
    combinedPainter.drawPixmap(pixmapSize / 2, 0, pixmap2);
    combinedPainter.drawPixmap(0, pixmapSize / 2, pixmap3);
    combinedPainter.drawPixmap(pixmapSize / 2, pixmapSize / 2, pixmap4);

    return combinedPixmap;
}

bool Item::ableToConbine(Item other)
{
    if (isDye() || other.isDye()) {
        return false;
    }

    return !((part1 != EMPTY && other.part1 != EMPTY) ||
             (part2 != EMPTY && other.part2 != EMPTY) ||
             (part3 != EMPTY && other.part3 != EMPTY) ||
             (part4 != EMPTY && other.part4 != EMPTY));
}

bool Item::isCuttable()
{
    return cuttable;
}

Item *Item::rotateItem()
{
    if (isDye()) {
        return new Item(*this);
    }

    Item *rotated = new Item(part3, part1, part4, part2);
    rotated->part1Color = part3 == EMPTY ? QString() : part3Color;
    rotated->part2Color = part1 == EMPTY ? QString() : part1Color;
    rotated->part3Color = part4 == EMPTY ? QString() : part4Color;
    rotated->part4Color = part2 == EMPTY ? QString() : part2Color;
    rotated->cuttable = cuttable;
    return rotated;
}

Item Item::operator+(const Item &other) const
{
    Item result;
    if (isDye() || other.isDye()) {
        result.cuttable = false;
        return result;
    }

    result.part1 = this->part1 != EMPTY ? this->part1 : other.part1;
    result.part2 = this->part2 != EMPTY ? this->part2 : other.part2;
    result.part3 = this->part3 != EMPTY ? this->part3 : other.part3;
    result.part4 = this->part4 != EMPTY ? this->part4 : other.part4;

    result.part1Color = this->part1 != EMPTY ? this->part1Color : other.part1Color;
    result.part2Color = this->part2 != EMPTY ? this->part2Color : other.part2Color;
    result.part3Color = this->part3 != EMPTY ? this->part3Color : other.part3Color;
    result.part4Color = this->part4 != EMPTY ? this->part4Color : other.part4Color;
    result.cuttable = cuttable || other.cuttable;
    return result;
}

std::pair<Item *, Item *> Item::cutItem(int stragedy)
{
    if (stragedy == 0) {
        Item *item1 = new Item(part1, part2, EMPTY, EMPTY);
        Item *item2 = new Item(EMPTY, EMPTY, part3, part4);
        item1->part1Color = part1 == EMPTY ? QString() : part1Color;
        item1->part2Color = part2 == EMPTY ? QString() : part2Color;
        item2->part3Color = part3 == EMPTY ? QString() : part3Color;
        item2->part4Color = part4 == EMPTY ? QString() : part4Color;
        return std::make_pair(item1, item2);
    }

    if (stragedy == 1) {
        Item *item1 = new Item(part1, EMPTY, part3, EMPTY);
        Item *item2 = new Item(EMPTY, part2, EMPTY, part4);
        item1->part1Color = part1 == EMPTY ? QString() : part1Color;
        item1->part3Color = part3 == EMPTY ? QString() : part3Color;
        item2->part2Color = part2 == EMPTY ? QString() : part2Color;
        item2->part4Color = part4 == EMPTY ? QString() : part4Color;
        return std::make_pair(item1, item2);
    }

    qDebug() << "undefined stragedy";
    Item *item1 = new Item(*this);
    Item *item2 = new Item(*this);
    return std::make_pair(item1, item2);
}
