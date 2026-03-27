#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QWidget>
#include <QIcon>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QSoundEffect>
#include <QVector>
#include <QHash>
#include "map.h"

class Gamescene : public QWidget
{
    Q_OBJECT
public:
    explicit Gamescene(QWidget *parent = nullptr);

    ~Gamescene();

    void setPuzzle();

    bool eventFilter(QObject *watched, QEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

    void paintEvent(QPaintEvent *) override;

    void saveGame(const QString& filename);

    void loadGame(const QString& filename);

    void autoSaveGame(const QString& filename);

    void autoLoadGame(const QString& filename);

    void returnToMainScene();

    Map * map = nullptr;

    bool isPlaceItem;
    Tile* currentTile;

private:
    void clearPlacementSelection();
    void setPlacementTile(Tile *tile);
    void refreshPlacementPreview();
    void updateInterfaceLayout();
    QRect mapViewportRect() const;
    QPoint clampedMapPosition(const QPoint &desiredPosition) const;
    void applyMapViewportMask();
    void panMapBy(const QPoint &delta);
    void zoomMapAt(const QPoint &viewportPos, double zoomDelta);
    QString t(const QString &zhText, const QString &enText) const;
    bool handleMapMousePress(QMouseEvent *event);
    bool handleMapMouseMove(QMouseEvent *event);
    bool handleMapMouseRelease(QMouseEvent *event);
    void populateStartingResources();
    void placeRandomResourceCluster(const QString &resourceName, int rowMin, int rowMax, int colMin, int colMax);
    void updateTexts();
    void beginBeltDrag(const QPoint &startCell);
    void updateBeltDragPath(const QPoint &targetCell);
    void rebuildBeltDragPath();
    void clearBeltDragPath();
    Tile beltTileForPathIndex(int index) const;
    int directionForStep(const QPoint &from, const QPoint &to) const;
    bool isCellAvailableForDraggedBelt(const QPoint &cell) const;
    Tile *createPlacementTile(Tile::Type type, const QString &name, std::pair<int, int> size = std::make_pair(1,1)) const;
    int rememberedDirectionFor(const QString &name, Tile::Type type) const;
    void rememberDirectionForCurrentTile();

    int defaultBeltDirection;
    QHash<QString, int> rememberedBuildingDirections;

    QPushButton * beltbtn;
    QPushButton * balancerbtn;
    QPushButton * underground_beltbtn;
    QPushButton * minerbtn;
    QPushButton * cutterbtn;
    QPushButton * rotaterbtn;
    QPushButton * stackerbtn;
    QPushButton * mixerbtn;
    QPushButton * painterbtn;
    QPushButton * trashbtn;
    QPushButton * backbtn;
    QPushButton * savebtn;
    QPushButton * upgratebtn;

    QSoundEffect ui_clickEffect;

    QFont font;

    bool isDragging;
    bool isPanning = false;
    bool isBeltDragging = false;
    bool hasInitializedMapPosition = false;
    QPoint lastPanGlobalPos;
    QVector<QPoint> beltDragPath;
    QVector<QPoint> activeDraggedBeltCells;
    QString languageCode = "zh-CN";

    QTimer* itemMoveTimer;
    QTimer* minerTimer;
    QTimer* cutterTimer;

    int itemMoveTimerIntervalUpgrate = 400;
    int minerTimerIntervalUpgrate = 1600;
    int cutterTimerIntervalUpgrate = 3200;

    bool itemMoveUpgrate = false;
    bool minerUpgrate = false;
    bool cutterUpgrate = false;

signals:
    void returnToMain();
};

#endif // GAMESCENE_H
