#ifndef MAP_H
#define MAP_H

#include <QWidget>
#include <QString>
#include <QColor>
#include <QPainter>
#include <QBrush>
#include <QGraphicsRectItem>
#include <QTimer>
#include <QLabel>
#include <QHash>
#include <QUuid>
#include <QSharedPointer>
#include <QList>
#include <QSoundEffect>
#include <QPoint>
#include <QSize>
#include <QEvent>

#include "tile.h"
//#include "item.h"


class Map : public QWidget{
    Q_OBJECT
public:
    Map(int height, int width, QWidget* parent = nullptr);
    ~Map();
    void setTile(int x, int y, Tile &tile, bool playSound = true);
    Tile getTile(int x, int y) const;
    Tile getTile(std::pair<int,int> pos) const;
    //Tile::Type getTileType(int x, int y) const;
    bool deleteTile(int x, int y);
    int getwidth() const;
    int getheight() const;
    std::pair<int,int> nextPox(int x,int y,Tile &currentTile);
    std::pair<int,int> nextPox(std::pair<int,int> originaPos,Tile &currentTile);
    bool inMap(int x,int y) const;
    bool inMap(std::pair<int,int> originaPos) const;
    std::pair<std::pair<int,int>,std::pair<int,int>> cutterOutPox(std::pair<int,int> pos,Tile &cutterTile);
    std::pair<std::pair<int,int>,std::pair<int,int>> cutterOutPox(int x, int y,Tile &cutterTile);

    void moveItems();
    void moveSingleItem(int x,int y,QSet<std::pair<int, int>> &movedItems);
    bool canEnter(int direction, std::pair<int,int> pos);

    void performMining();

    void cutterUpdate();

    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    void setItem(std::pair<int,int> pos, Item *item);

    void itemToHub(int part1, int part2, int part3, int part4);
    void clearMap();

    void setZoomFactor(double zoomFactor);
    double zoomFactor() const;
    int tilePixelSize() const;
    QPoint gridPositionFromPoint(const QPoint &point) const;
    bool canPlaceTile(int x, int y, const Tile &tile) const;

    void setBlueprintTile(const Tile *tile);
    void clearBlueprint();
    void updateBlueprintCursor(const QPoint &mapPos);
    void updateLayout();
    void hideBuildingInfo();

private slots:
    void updateAnimationFrame();

public:
    QLabel* questionLabel = nullptr;
    QLabel* countLabel = nullptr;
    QLabel* levelLabel = nullptr;

    int questionLever;

    int current;
    int target;

    QLabel *blueprintLabel;

    //int coins = 0;

    QVector<QVector<Tile*>> tiles;
private:
    int width, height;
    int frameIndex;
    QTimer* animationTimer;
    QTimer* buildingInfoTimer = nullptr;
    double currentZoomFactor = 1.0;
    QPoint lastBlueprintCursorPos;
    QPoint lastHoverCursorPos;
    QPoint hoveredBuildingRootCell = QPoint(-1, -1);

    QSoundEffect place_beltEffect;

    QSoundEffect place_buildingEffect;

    QList<std::pair<int,int>> miners;
    //QList<Item*> items;
    int hudAnchorRow = 8;
    int hudAnchorColumn = 15;
    QLabel *buildingInfoLabel = nullptr;

    QSize cellPixelSize(std::pair<int, int> cellSpan = std::make_pair(1,1)) const;
    QRect tileGeometry(int x, int y, std::pair<int, int> cellSpan = std::make_pair(1,1)) const;
    void updateTileLabel(int x, int y);
    void updateItemLabel(const std::pair<int, int> &pos);
    void updateHudGeometry();
    void updateHudFonts();
    void updateHoverBuilding(const QPoint &mapPos);
    void showBuildingInfo();
    void updateBuildingInfoPosition(const QPoint &mapPos);
    QPoint rootCellForCell(const QPoint &cell) const;
    QString buildingInfoText(const Tile &tile) const;
    QPixmap scaledPixmapForSize(const QPixmap &pixmap, const QSize &targetSize) const;
    QPixmap blueprintPixmapForTile(const Tile &tile) const;
    void applyBlueprintPixmap(const Tile &tile);

};

#endif // MAP_H
