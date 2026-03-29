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
#include <array>
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
    void placeRandomColorCluster(const QString &colorName);
    void updateTexts();
    QString upgradeOverviewText() const;
    void showUpgradeOverview();
    void beginBeltDrag(const QPoint &startCell);
    void updateBeltDragPath(const QPoint &targetCell);
    void rebuildBeltDragPath();
    void clearBeltDragPath();
    Tile beltTileForPathIndex(int index) const;
    int directionForStep(const QPoint &from, const QPoint &to) const;
    bool isCellAvailableForDraggedBelt(const QPoint &cell) const;
    bool canConnectDraggedBeltToCell(const QPoint &from, const QPoint &target) const;
    Tile *createPlacementTile(Tile::Type type, const QString &name, std::pair<int, int> size = std::make_pair(1,1)) const;
    int rememberedDirectionFor(const QString &name, Tile::Type type) const;
    void rememberDirectionForCurrentTile();
    std::array<int, 4> generatedGoalForLevel(int levelIndex) const;
    std::array<QString, 4> generatedGoalColorsForLevel(int levelIndex, const std::array<int, 4> &parts) const;
    void refreshProgressLabels();
    bool applyUpgradeOption(int optionId);
    void tryAdvanceLevel();
    void skipCurrentLevelForDebug();

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
    QPushButton * debugNextLevelBtn = nullptr;

    QSoundEffect ui_clickEffect;

    QFont font;

    bool isDragging;
    bool isPanning = false;
    bool isBeltDragging = false;
    bool hasInitializedMapPosition = false;
    QPoint lastPanGlobalPos;
    QPoint beltDragConnectionTarget = QPoint(-1, -1);
    QVector<QPoint> beltDragPath;
    QVector<QPoint> activeDraggedBeltCells;
    QString languageCode = "zh-CN";

    QTimer* itemMoveTimer;
    QTimer* balancerTimer;
    QTimer* undergroundTimer;
    QTimer* minerTimer;
    QTimer* cutterTimer;
    QTimer* rotaterTimer;
    QTimer* stackerTimer;
    QTimer* mixerTimer;
    QTimer* painterTimer;

    // Tier-based upgrade system (0 = default)
    int itemMoveTier = 0;
    int balancerTier = 0;
    int undergroundTier = 0;
    int minerTier    = 0;
    int cutterTier   = 0;
    int rotaterTier  = 0;
    int stackerTier  = 0;
    int mixerTier    = 0;
    int painterTier  = 0;
    int pendingUpgradePoints = 0;
    std::array<int, 4> currentGoalParts = {EMPTY, EMPTY, EMPTY, EMPTY};
    std::array<QString, 4> currentGoalColors = {"", "", "", ""};

    // Timer intervals per tier
    static constexpr int kBeltIntervals[6]       = {800, 640, 520, 420, 330, 250};
    static constexpr int kBalancerIntervals[6]   = {860, 700, 560, 450, 350, 270};
    static constexpr int kUndergroundIntervals[6]= {1200, 980, 800, 650, 520, 400};
    static constexpr int kMinerIntervals[6]      = {3200, 2500, 1900, 1450, 1050, 750};
    static constexpr int kCutterIntervals[6]     = {8000, 6200, 4800, 3600, 2700, 2000};
    static constexpr int kRotaterIntervals[6]    = {1600, 1300, 1050, 850, 680, 520};
    static constexpr int kStackerIntervals[6]    = {2600, 2100, 1700, 1360, 1080, 860};
    static constexpr int kMixerIntervals[6]      = {2600, 2100, 1700, 1360, 1080, 860};
    static constexpr int kPainterIntervals[6]    = {2600, 2100, 1700, 1360, 1080, 860};

    void applyTimerTiers();
    void updateUndergroundBeltPreview(const QPoint &gridCell);
    bool undergroundConnectionForDirection(const QPoint &gridCell, int direction, QString &resolvedName,
                                          QPoint &previewStart, QPoint &previewEnd) const;
    Tile resolvedUndergroundPlacementTile(const QPoint &gridCell, QPoint *previewStart = nullptr,
                                          QPoint *previewEnd = nullptr) const;

signals:
    void returnToMain();
};

#endif // GAMESCENE_H
