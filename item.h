#ifndef ITEM_H
#define ITEM_H
#include <QLabel>
#include <QPixmap>

#define EMPTY 0
#define SQUARE 1
#define CIRCLE 2

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3
#define NONEDIREECTION 4

class Item
{
public:
    Item():part1(EMPTY),part2(EMPTY),part3(EMPTY),part4(EMPTY),
        label(nullptr),speed(0),direction(NONEDIREECTION),cuttable(false){}
    Item(int part1,int part2,int part3,int part4);
    ~Item();

    QPixmap getPixmap();
    QPixmap drawSquare();
    QPixmap drawCircle();

    bool ableToConbine(Item other);

    Item operator+(const Item& other) const;

    int part1;
    int part2;
    int part3;
    int part4;
    QLabel* label;
private:
    int speed;
    int direction;
    bool cuttable;
};

#endif // ITEM_H
