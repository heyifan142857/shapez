#ifndef ITEM_H
#define ITEM_H
#include <QLabel>
#include <QPixmap>
#include <QString>
#include <QColor>
#include <QUuid>
#include <array>


#define EMPTY 0
#define SQUARE 1
#define CIRCLE 2
#define DIAMOND 3

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3
#define NONEDIREECTION 4

class Item
{
public:
    Item():part1(EMPTY),part2(EMPTY),part3(EMPTY),part4(EMPTY),
        label(nullptr),cuttable(true),pos(std::make_pair(0,0)),
        part1Color(""),part2Color(""),part3Color(""),part4Color(""),
        dyeName("") {

    }
    Item(int part1,int part2,int part3,int part4, std::pair<int,int> pos = std::make_pair(0,0));
    Item(QString mine, std::pair<int,int> pos = std::make_pair(0,0));
    Item(const Item& other);
    ~Item();

    static bool isDyeName(const QString &name);
    static QColor colorForName(const QString &name);

    bool isDye() const;
    void applyColor(const QString &colorName);

    QPixmap getPixmap();
    QPixmap drawSquare(const QString &colorName = "uncolored");
    QPixmap drawCircle(const QString &colorName = "uncolored");
    QPixmap drawDiamond(const QString &colorName = "uncolored");
    QPixmap drawPixmap(int part1,int part2,int part3,int part4,int pixmapSize);
    QPixmap drawPixmap(int part1,int part2,int part3,int part4,int pixmapSize,
                       const std::array<QString, 4> &partColors);

    std::pair<Item*,Item*> cutItem(int stragedy);//stragedy = 0,横着切;stragedy = 1,竖着切

    Item* rotateItem(); // 顺时针旋转90°

    bool ableToConbine(Item other);

    bool isCuttable();

    Item operator+(const Item& other) const;

    std::pair<int,int> pos;

    int part1;
    int part2;
    int part3;
    int part4;
    QString part1Color;
    QString part2Color;
    QString part3Color;
    QString part4Color;
    QString dyeName;
    QLabel* label;
private:
    bool cuttable;
};

#endif // ITEM_H
