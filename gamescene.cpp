#include "gamescene.h"
#include "item.h"
#include <QGuiApplication>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QFont>
#include <QFontDatabase>
#include <QSoundEffect>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDir>
#include <QFileInfo>
#include <QInputDialog>
#include <QRandomGenerator>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QScreen>
#include <QCursor>
#include <QHelpEvent>
#include <QProgressBar>
#include <QScrollArea>
#include <QSet>
#include <QToolTip>
#include <algorithm>
#include "configmanager.h"
#include "localization.h"

namespace {
constexpr int kDefaultItemMoveTimerInterval = 800;
constexpr int kDefaultBalancerTimerInterval = 860;
constexpr int kDefaultUndergroundTimerInterval = 1200;
constexpr int kDefaultMinerTimerInterval = 3200;
constexpr int kDefaultCutterTimerInterval = 8000;
constexpr int kDefaultRotaterTimerInterval = 1600;
constexpr int kDefaultStackerTimerInterval = 2600;
constexpr int kDefaultMixerTimerInterval = 2600;
constexpr int kDefaultPainterTimerInterval = 2600;
constexpr int kMaxUpgradeTier = 5;

using GoalParts = std::array<int, 4>;
using GoalColors = std::array<QString, 4>;

double itemsPerSecondForInterval(int intervalMs)
{
    return 1000.0 / intervalMs;
}

GoalParts fullGoal(int type)
{
    return {type, type, type, type};
}

GoalColors defaultGoalColorsForParts(const GoalParts &parts, const QString &colorName = QStringLiteral("uncolored"))
{
    GoalColors colors = {"", "", "", ""};
    for (int index = 0; index < 4; ++index) {
        if (parts[index] != EMPTY) {
            colors[index] = colorName;
        }
    }
    return colors;
}

int distinctTypeCount(const GoalParts &goal)
{
    QSet<int> types;
    for (int part : goal) {
        if (part != EMPTY) {
            types.insert(part);
        }
    }
    return types.size();
}

void shuffleVector(QVector<int> &values, QRandomGenerator &generator)
{
    for (int index = values.size() - 1; index > 0; --index) {
        values.swapItemsAt(index, generator.bounded(index + 1));
    }
}

Tile::Type mineTileTypeForName(const QString &name)
{
    return Item::isDyeName(name) ? Tile::Type::Color : Tile::Type::Resource;
}

QString normalizedLegacyDyeName(const QString &name)
{
    if (name == "yellow") {
        return QStringLiteral("green");
    }
    if (name == "orange") {
        return QStringLiteral("yellow");
    }
    if (name == "green") {
        return QStringLiteral("cyan");
    }
    return name;
}

QJsonObject serializeItem(const Item *item)
{
    QJsonObject itemObject;
    if (!item) {
        return itemObject;
    }

    itemObject["part1"] = item->part1;
    itemObject["part2"] = item->part2;
    itemObject["part3"] = item->part3;
    itemObject["part4"] = item->part4;
    itemObject["part1Color"] = item->part1Color;
    itemObject["part2Color"] = item->part2Color;
    itemObject["part3Color"] = item->part3Color;
    itemObject["part4Color"] = item->part4Color;
    itemObject["dyeName"] = item->dyeName;

    QJsonObject posObject;
    posObject["first"] = item->pos.first;
    posObject["second"] = item->pos.second;
    itemObject["pos"] = posObject;
    return itemObject;
}

Item *deserializeItem(const QJsonObject &itemObject)
{
    const QString dyeName = normalizedLegacyDyeName(itemObject["dyeName"].toString());
    Item *item = nullptr;
    if (!dyeName.isEmpty()) {
        item = new Item(dyeName);
    } else {
        item = new Item(
            itemObject["part1"].toInt(),
            itemObject["part2"].toInt(),
            itemObject["part3"].toInt(),
            itemObject["part4"].toInt()
        );
    }

    if (itemObject.contains("part1Color")) {
        item->part1Color = normalizedLegacyDyeName(itemObject["part1Color"].toString());
        item->part2Color = normalizedLegacyDyeName(itemObject["part2Color"].toString());
        item->part3Color = normalizedLegacyDyeName(itemObject["part3Color"].toString());
        item->part4Color = normalizedLegacyDyeName(itemObject["part4Color"].toString());
    }
    item->dyeName = dyeName;

    const QJsonObject posObject = itemObject["pos"].toObject();
    item->pos = std::make_pair(posObject["first"].toInt(), posObject["second"].toInt());
    return item;
}

struct UpgradeDialogEntry
{
    int optionId = 0;
    int currentTier = 0;
    QString iconPath;
    QString displayName;
    QString description;
    QString effectTooltip;
};

QSize boundedUpgradeDialogSize(const QSize &desiredSize)
{
    const QRect availableGeometry = QGuiApplication::primaryScreen()
        ? QGuiApplication::primaryScreen()->availableGeometry()
        : QRect(0, 0, 1600, 900);
    return QSize(qMax(560, availableGeometry.width() - 80),
                 qMax(420, availableGeometry.height() - 80)).boundedTo(desiredSize);
}

QSize boundedUpgradeDialogMinimum()
{
    const QRect availableGeometry = QGuiApplication::primaryScreen()
        ? QGuiApplication::primaryScreen()->availableGeometry()
        : QRect(0, 0, 1600, 900);
    return QSize(qMin(680, qMax(560, availableGeometry.width() - 80)),
                 qMin(480, qMax(420, availableGeometry.height() - 80)));
}

QString upgradePanelStyleSheet()
{
    return QStringLiteral(
        "QFrame#upgradeCard {"
        "background: #f7f9fc;"
        "border: 1px solid #d6deeb;"
        "border-radius: 14px;"
        "}"
        "QProgressBar {"
        "border: 1px solid #c7d2e5;"
        "border-radius: 10px;"
        "background: #eef3fb;"
        "color: #23324d;"
        "font-weight: 600;"
        "min-height: 20px;"
        "text-align: center;"
        "}"
        "QProgressBar::chunk {"
        "background-color: #5a8cff;"
        "border-radius: 9px;"
        "}"
        "QPushButton#upgradePlusButton {"
        "background: #23324d;"
        "border: none;"
        "border-radius: 14px;"
        "color: white;"
        "font-size: 22px;"
        "font-weight: 700;"
        "padding-bottom: 2px;"
        "}"
        "QPushButton#upgradePlusButton:hover {"
        "background: #35507a;"
        "}"
        "QPushButton#upgradePlusButton:disabled {"
        "background: #d7ddea;"
        "color: #7f8ba1;"
        "}"
    );
}
}

class UpgradeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpgradeDialog(const QString &languageCode,
                           const QVector<UpgradeDialogEntry> &entries,
                           int availablePoints,
                           QWidget *parent = nullptr)
        : QDialog(parent), languageCode(languageCode), availablePoints(availablePoints)
    {
        setWindowTitle(text("选择升级", "Choose Upgrade"));
        setModal(true);
        resize(boundedUpgradeDialogSize(QSize(920, 720)));
        setMinimumSize(boundedUpgradeDialogMinimum());
        setStyleSheet(upgradePanelStyleSheet());

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(20, 20, 20, 20);
        layout->setSpacing(12);

        QLabel *hint = new QLabel(text("完成关卡！选择一项升级：", "Level complete! Choose an upgrade:"), this);
        hint->setStyleSheet("font-size: 20px; font-weight: 700; color: #22324b;");
        layout->addWidget(hint);

        QLabel *subHint = new QLabel(
            text("左侧查看建筑与说明，右侧查看等级进度。将鼠标悬停在 ➕ 或进度条上可以预览升级效果。",
                 "Check the building and description on the left, then review the tier progress on the right. Hover the ➕ button or progress bar to preview the upgrade effect."),
            this);
        subHint->setWordWrap(true);
        subHint->setStyleSheet("color: #55657f;");
        layout->addWidget(subHint);

        pointsLabel = new QLabel(this);
        pointsLabel->setStyleSheet("color: #55657f; background: #edf4ff; border-radius: 10px; padding: 10px 12px;");
        layout->addWidget(pointsLabel);

        statusLabel = new QLabel(this);
        statusLabel->setWordWrap(true);
        statusLabel->setStyleSheet("color: #55657f; background: #edf4ff; border-radius: 10px; padding: 10px 12px;");
        layout->addWidget(statusLabel);

        QScrollArea *scrollArea = new QScrollArea(this);
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::NoFrame);

        QWidget *content = new QWidget(scrollArea);
        QVBoxLayout *contentLayout = new QVBoxLayout(content);
        contentLayout->setContentsMargins(0, 0, 0, 0);
        contentLayout->setSpacing(10);

        bool hasAvailableUpgrade = false;
        for (const UpgradeDialogEntry &entry : entries) {
            QFrame *card = new QFrame(content);
            card->setObjectName("upgradeCard");
            card->setToolTip(entry.effectTooltip);

            QHBoxLayout *rowLayout = new QHBoxLayout(card);
            rowLayout->setContentsMargins(16, 14, 16, 14);
            rowLayout->setSpacing(14);

            QLabel *iconLabel = new QLabel(card);
            iconLabel->setFixedSize(56, 56);
            const QPixmap iconPixmap(entry.iconPath);
            iconLabel->setPixmap(iconPixmap.scaled(iconLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            iconLabel->setAlignment(Qt::AlignCenter);
            rowLayout->addWidget(iconLabel, 0, Qt::AlignTop);

            QVBoxLayout *textLayout = new QVBoxLayout();
            textLayout->setSpacing(4);

            QLabel *nameLabel = new QLabel(QString("%1 (Lv.%2)").arg(entry.displayName).arg(entry.currentTier), card);
            nameLabel->setStyleSheet("font-size: 16px; font-weight: 700; color: #22324b;");
            textLayout->addWidget(nameLabel);

            QLabel *descriptionLabel = new QLabel(entry.description, card);
            descriptionLabel->setWordWrap(true);
            descriptionLabel->setStyleSheet("color: #55657f; line-height: 1.35em;");
            textLayout->addWidget(descriptionLabel);
            textLayout->addStretch();
            rowLayout->addLayout(textLayout, 1);

            QVBoxLayout *progressLayout = new QVBoxLayout();
            progressLayout->setSpacing(8);

            QProgressBar *progressBar = new QProgressBar(card);
            progressBar->setRange(0, kMaxUpgradeTier);
            progressBar->setValue(entry.currentTier);
            progressBar->setFormat(text("Lv.%v / Lv.%m", "Lv.%v / Lv.%m"));
            progressBar->setTextVisible(true);
            progressBar->setMinimumWidth(220);
            progressBar->setToolTip(entry.effectTooltip);
            progressLayout->addWidget(progressBar);

            QLabel *effectHint = new QLabel(text("悬停查看升级效果", "Hover to preview the upgrade effect"), card);
            effectHint->setStyleSheet("color: #70819c; font-size: 12px;");
            progressLayout->addWidget(effectHint);

            rowLayout->addLayout(progressLayout, 0);

            QPushButton *btn = new QPushButton(QStringLiteral("➕"), card);
            btn->setObjectName("upgradePlusButton");
            btn->setFixedSize(48, 48);
            btn->setCursor(Qt::PointingHandCursor);
            btn->setToolTip(entry.effectTooltip);

            const bool canUpgrade = entry.currentTier < kMaxUpgradeTier;
            btn->setEnabled(canUpgrade && availablePoints > 0);
            if (canUpgrade) {
                hasAvailableUpgrade = true;
                const int rowIndex = rows.size();
                rows.append({btn, progressBar, nameLabel, entry.displayName, entry.optionId, entry.currentTier, entry.effectTooltip});
                connect(btn, &QPushButton::clicked, this, [this, rowIndex]() {
                    if (this->availablePoints <= 0 || rowIndex < 0 || rowIndex >= rows.size()) {
                        return;
                    }

                    UpgradeRowState &row = rows[rowIndex];
                    if (row.currentTier >= kMaxUpgradeTier) {
                        return;
                    }

                    selectedOptions.append(row.optionId);
                    this->availablePoints--;
                    row.currentTier = qMin(row.currentTier + 1, kMaxUpgradeTier);
                    row.progressBar->setValue(row.currentTier);
                    row.nameLabel->setText(QString("%1 (Lv.%2)").arg(row.displayName).arg(row.currentTier));

                    const QString updatedTooltip = text(
                        QString("已累计升级：%1 当前 Lv.%2").arg(row.displayName).arg(row.currentTier),
                        QString("Queued upgrade: %1 is now Lv.%2").arg(row.displayName).arg(row.currentTier));
                    row.progressBar->setToolTip(updatedTooltip);
                    row.button->setToolTip(updatedTooltip);

                    refreshUi();
                });
            }

            rowLayout->addWidget(btn, 0, Qt::AlignCenter);
            contentLayout->addWidget(card);
        }
        contentLayout->addStretch();

        scrollArea->setWidget(content);
        layout->addWidget(scrollArea, 1);

        if (!hasAvailableUpgrade) {
            QLabel *maxLabel = new QLabel(
                text("所有升级已达最高等级，进度条会继续保留显示。",
                     "All upgrades are already at max level, and the filled progress bars remain visible."),
                this);
            maxLabel->setWordWrap(true);
            maxLabel->setStyleSheet("color: #7a5a00; background: #fff4d6; border-radius: 10px; padding: 10px 12px;");
            layout->addWidget(maxLabel);
            statusLabel->setText(text(
                "这一关没有可选升级了，直接关闭窗口继续。",
                "There are no remaining upgrades to pick this level, so you can close this window and continue."));
        }

        QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
        closeButton = buttonBox->addButton(text("暂不升级", "Skip for Now"), QDialogButtonBox::RejectRole);
        closeButton->setCursor(Qt::PointingHandCursor);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(buttonBox);

        refreshUi();
    }

    QVector<int> getSelectedOptions() const {
        return selectedOptions;
    }

private:
    struct UpgradeRowState {
        QPushButton *button = nullptr;
        QProgressBar *progressBar = nullptr;
        QLabel *nameLabel = nullptr;
        QString displayName;
        int optionId = 0;
        int currentTier = 0;
        QString effectTooltip;
    };

    QString text(const QString &zhText, const QString &enText) const {
        return Localization::text(languageCode, zhText, enText);
    }

    void refreshUi()
    {
        if (pointsLabel) {
            pointsLabel->setText(text(
                QString("当前可用升级点：%1").arg(availablePoints),
                QString("Available upgrade points: %1").arg(availablePoints)));
        }

        if (statusLabel) {
            if (selectedOptions.isEmpty()) {
                statusLabel->setText(text(
                    "你可以现在花掉升级点，也可以直接关闭窗口稍后再补点。",
                    "You can spend upgrade points now, or close this window and use them later."));
            } else {
                statusLabel->setText(text(
                    QString("已累计分配 %1 点升级。你可以继续补点，或直接关闭窗口继续游戏。").arg(selectedOptions.size()),
                    QString("%1 upgrade point(s) have been queued. You can keep spending or close this window to continue.").arg(selectedOptions.size())));
            }
        }

        for (UpgradeRowState &row : rows) {
            if (row.button) {
                row.button->setEnabled(availablePoints > 0 && row.currentTier < kMaxUpgradeTier);
            }
        }

        if (closeButton) {
            closeButton->setText(selectedOptions.isEmpty()
                ? text("暂不升级", "Skip for Now")
                : text("关闭并继续", "Close and Continue"));
        }
    }

    QString languageCode;
    int availablePoints = 0;
    QLabel *pointsLabel = nullptr;
    QLabel *statusLabel = nullptr;
    QPushButton *closeButton = nullptr;
    QVector<UpgradeRowState> rows;
    QVector<int> selectedOptions;
};

class UpgradeOverviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpgradeOverviewDialog(const QString &languageCode,
                                   const QVector<UpgradeDialogEntry> &entries,
                                   int availablePoints,
                                   QWidget *parent = nullptr)
        : QDialog(parent), languageCode(languageCode), availablePoints(availablePoints)
    {
        setWindowTitle(text("升级概览", "Upgrade Overview"));
        resize(boundedUpgradeDialogSize(QSize(860, 620)));
        setMinimumSize(boundedUpgradeDialogMinimum());
        setStyleSheet(upgradePanelStyleSheet());

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(20, 20, 20, 20);
        layout->setSpacing(12);

        QLabel *titleLabel = new QLabel(text("当前升级统计", "Current Upgrade Stats"), this);
        titleLabel->setStyleSheet("font-size: 20px; font-weight: 700; color: #22324b;");
        layout->addWidget(titleLabel);

        QLabel *hintLabel = new QLabel(
            text("这里会保留所有建筑的等级进度。悬停进度条可以预览下一次升级收益；如果有保留的升级点，也可以在这里补点。",
                 "This panel keeps every building's level progress visible. Hover a progress bar to preview the next upgrade gain, and spend any saved upgrade points here."),
            this);
        hintLabel->setWordWrap(true);
        hintLabel->setStyleSheet("color: #55657f;");
        layout->addWidget(hintLabel);

        pointsLabel = new QLabel(this);
        pointsLabel->setStyleSheet("color: #55657f; background: #edf4ff; border-radius: 10px; padding: 10px 12px;");
        layout->addWidget(pointsLabel);

        QScrollArea *scrollArea = new QScrollArea(this);
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::NoFrame);

        QWidget *content = new QWidget(scrollArea);
        QVBoxLayout *contentLayout = new QVBoxLayout(content);
        contentLayout->setContentsMargins(0, 0, 0, 0);
        contentLayout->setSpacing(10);

        for (const UpgradeDialogEntry &entry : entries) {
            QFrame *card = new QFrame(content);
            card->setObjectName("upgradeCard");

            QHBoxLayout *rowLayout = new QHBoxLayout(card);
            rowLayout->setContentsMargins(16, 14, 16, 14);
            rowLayout->setSpacing(14);

            QLabel *iconLabel = new QLabel(card);
            iconLabel->setFixedSize(48, 48);
            iconLabel->setPixmap(QPixmap(entry.iconPath).scaled(iconLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            iconLabel->setAlignment(Qt::AlignCenter);
            rowLayout->addWidget(iconLabel, 0, Qt::AlignTop);

            QVBoxLayout *textLayout = new QVBoxLayout();
            textLayout->setSpacing(4);

            QLabel *nameLabel = new QLabel(QString("%1 (Lv.%2)").arg(entry.displayName).arg(entry.currentTier), card);
            nameLabel->setStyleSheet("font-size: 16px; font-weight: 700; color: #22324b;");
            textLayout->addWidget(nameLabel);

            QLabel *descriptionLabel = new QLabel(entry.description, card);
            descriptionLabel->setWordWrap(true);
            descriptionLabel->setStyleSheet("color: #55657f;");
            textLayout->addWidget(descriptionLabel);
            rowLayout->addLayout(textLayout, 1);

            QProgressBar *progressBar = new QProgressBar(card);
            progressBar->setRange(0, kMaxUpgradeTier);
            progressBar->setValue(entry.currentTier);
            progressBar->setFormat(text("Lv.%v / Lv.%m", "Lv.%v / Lv.%m"));
            progressBar->setMinimumWidth(220);
            progressBar->setToolTip(entry.effectTooltip);
            rowLayout->addWidget(progressBar, 0, Qt::AlignCenter);

            QPushButton *plusButton = new QPushButton(QStringLiteral("➕"), card);
            plusButton->setObjectName("upgradePlusButton");
            plusButton->setFixedSize(44, 44);
            plusButton->setCursor(Qt::PointingHandCursor);
            plusButton->setToolTip(entry.effectTooltip);
            plusButton->setVisible(entry.currentTier < kMaxUpgradeTier);
            rowLayout->addWidget(plusButton, 0, Qt::AlignCenter);

            const int rowIndex = rows.size();
            rows.append({plusButton, progressBar, nameLabel, entry.displayName, entry.optionId, entry.currentTier, entry.effectTooltip});
            connect(plusButton, &QPushButton::clicked, this, [this, rowIndex]() {
                if (this->availablePoints <= 0 || rowIndex < 0 || rowIndex >= rows.size()) {
                    return;
                }

                UpgradeRowState &row = rows[rowIndex];
                if (row.currentTier >= kMaxUpgradeTier) {
                    return;
                }

                selectedOptions.append(row.optionId);
                this->availablePoints--;
                row.currentTier = qMin(row.currentTier + 1, kMaxUpgradeTier);
                row.progressBar->setValue(row.currentTier);
                row.nameLabel->setText(QString("%1 (Lv.%2)").arg(row.displayName).arg(row.currentTier));

                const QString updatedTooltip = text(
                    QString("已累计升级：%1 当前 Lv.%2").arg(row.displayName).arg(row.currentTier),
                    QString("Queued upgrade: %1 is now Lv.%2").arg(row.displayName).arg(row.currentTier));
                row.progressBar->setToolTip(updatedTooltip);
                row.button->setToolTip(updatedTooltip);

                refreshUi();
            });

            contentLayout->addWidget(card);
        }
        contentLayout->addStretch();

        scrollArea->setWidget(content);
        layout->addWidget(scrollArea, 1);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
        QPushButton *closeBtn = buttonBox->addButton(text("关闭", "Close"), QDialogButtonBox::AcceptRole);
        closeBtn->setCursor(Qt::PointingHandCursor);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        layout->addWidget(buttonBox);

        refreshUi();
    }

    QVector<int> getSelectedOptions() const {
        return selectedOptions;
    }

private:
    struct UpgradeRowState {
        QPushButton *button = nullptr;
        QProgressBar *progressBar = nullptr;
        QLabel *nameLabel = nullptr;
        QString displayName;
        int optionId = 0;
        int currentTier = 0;
        QString effectTooltip;
    };

    QString text(const QString &zhText, const QString &enText) const {
        return Localization::text(languageCode, zhText, enText);
    }

    void refreshUi()
    {
        if (pointsLabel) {
            pointsLabel->setText(text(
                QString("当前保留升级点：%1").arg(availablePoints),
                QString("Saved upgrade points: %1").arg(availablePoints)));
        }

        for (UpgradeRowState &row : rows) {
            if (row.button) {
                const bool canUpgrade = availablePoints > 0 && row.currentTier < kMaxUpgradeTier;
                row.button->setEnabled(canUpgrade);
            }
        }
    }

    QString languageCode;
    int availablePoints = 0;
    QLabel *pointsLabel = nullptr;
    QVector<UpgradeRowState> rows;
    QVector<int> selectedOptions;
};


Gamescene::Gamescene(QWidget *parent)
    : isPlaceItem(false), currentTile(nullptr), QWidget{parent}
{
    //编辑窗口基本信息
    const QRect availableGeometry = QGuiApplication::primaryScreen()
        ? QGuiApplication::primaryScreen()->availableGeometry()
        : QRect(0, 0, 1600, 900);
    resize(QSize(1600, 900).boundedTo(availableGeometry.size()));
    setMinimumSize(QSize(qMin(960, availableGeometry.width()), qMin(540, availableGeometry.height())));
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setWindowIcon(QIcon(":/res/icon.ico"));
    setWindowTitle("Shapez");

    ConfigManager config;
    languageCode = config.getLanguage();

    qDebug() << "building map";
    map = new Map(36,64,this);
    map->installEventFilter(this);
    map->setMouseTracking(true);
    Tile Hub(Tile::Type::Hub);
    map->setTile(16,30,Hub);
    populateStartingResources();
    qDebug() << "successfully build map";

    //编辑问题
    map->current = 0;
    map->target = 0;
    map->questionLever = 0;
    setPuzzle();

    int fontId = QFontDatabase::addApplicationFont(":/res/font/ComicNeue-Bold.ttf");
    if (fontId == -1) {
        qDebug() << "Failed to load font!";
    }
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (fontFamilies.isEmpty()) {
        qDebug() << "No font families found!";
    }
    QString fontFamily = fontFamilies.at(0);
    qDebug() << "Loaded font family:" << fontFamily;
    font.setPointSize(28);
    font.setFamily(fontFamily);
    map->countLabel->setFont(font);
    font.setPointSize(18);
    map->levelLabel->setFont(font);

    // test = new QLabel(this);
    // //QPixmap combinedPixmap = Item(SQUARE,SQUARE,SQUARE,SQUARE).getPixmap();
    // QPixmap combinedPixmap = Item(CIRCLE,CIRCLE,CIRCLE,CIRCLE).getPixmap();
    // test->setPixmap(combinedPixmap);

    isDragging = false;
    defaultBeltDirection = NORTH;

    //计时器
    itemMoveTimer = new QTimer(this);
    connect(itemMoveTimer, &QTimer::timeout, this, [this]() {
        map->moveItems();
        refreshProgressLabels();
        tryAdvanceLevel();
    });
    itemMoveTimer->start(kDefaultItemMoveTimerInterval);

    balancerTimer = new QTimer(this);
    connect(balancerTimer, &QTimer::timeout, this, [this]() {
        map->balancerUpdate();
        refreshProgressLabels();
        tryAdvanceLevel();
    });
    balancerTimer->start(kDefaultBalancerTimerInterval);

    undergroundTimer = new QTimer(this);
    connect(undergroundTimer, &QTimer::timeout, this, [this]() {
        map->undergroundBeltUpdate();
        refreshProgressLabels();
        tryAdvanceLevel();
    });
    undergroundTimer->start(kDefaultUndergroundTimerInterval);

    minerTimer = new QTimer(this);
    connect(minerTimer, &QTimer::timeout, this, [this]() {
        map->performMining();
        refreshProgressLabels();
        tryAdvanceLevel();
    });
    minerTimer->start(kDefaultMinerTimerInterval);

    cutterTimer = new QTimer(this);
    connect(cutterTimer, &QTimer::timeout, this, [this]() {
        map->cutterUpdate();
        refreshProgressLabels();
        tryAdvanceLevel();
    });
    cutterTimer->start(kDefaultCutterTimerInterval);

    rotaterTimer = new QTimer(this);
    connect(rotaterTimer, &QTimer::timeout, this, [this]() {
        map->rotaterUpdate();
        refreshProgressLabels();
        tryAdvanceLevel();
    });
    rotaterTimer->start(kDefaultRotaterTimerInterval);

    stackerTimer = new QTimer(this);
    connect(stackerTimer, &QTimer::timeout, this, [this]() {
        map->stackerUpdate();
        refreshProgressLabels();
        tryAdvanceLevel();
    });
    stackerTimer->start(kDefaultStackerTimerInterval);

    mixerTimer = new QTimer(this);
    connect(mixerTimer, &QTimer::timeout, this, [this]() {
        map->mixerUpdate();
        refreshProgressLabels();
        tryAdvanceLevel();
    });
    mixerTimer->start(kDefaultMixerTimerInterval);

    painterTimer = new QTimer(this);
    connect(painterTimer, &QTimer::timeout, this, [this]() {
        map->painterUpdate();
        refreshProgressLabels();
        tryAdvanceLevel();
    });
    painterTimer->start(kDefaultPainterTimerInterval);

    ui_clickEffect.setSource(QUrl("qrc:/res/sounds/ui_click.wav"));
    ui_clickEffect.setVolume(0.5f);


    //建立底部建筑按钮
    beltbtn = new QPushButton();
    beltbtn->setParent(this);
    beltbtn->setFocusPolicy(Qt::NoFocus);
    beltbtn->setFixedSize(70, 70);
    beltbtn->setIconSize(QSize(50,50));
    beltbtn->setIcon(QIcon(":/res/building_icons/belt.png"));
    beltbtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    beltbtn->move(450,810);

    connect(beltbtn, &QPushButton::clicked, this, [this]() {
        ui_clickEffect.play();
        if(isPlaceItem && currentTile && currentTile->type == Tile::Type::Belt){
            qDebug() << "cancel placing belt";
            clearPlacementSelection();
        }else{
            qDebug() << "placing belt";
            setPlacementTile(createPlacementTile(Tile::Type::Belt, "belt"));
        }
        setFocus();
    });

    QPropertyAnimation *beltbtnanimation = new QPropertyAnimation(beltbtn, "geometry");
    beltbtnanimation->setDuration(100);
    beltbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect beltbtnGeometry = beltbtn->geometry();
    connect(beltbtn,&QPushButton::pressed,this,[=](){
        QRect rect = beltbtn->geometry();
        beltbtnanimation->setStartValue(rect);
        beltbtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.05, rect.y() + rect.height() * 0.05,
                                            rect.width() * 0.9, rect.height() * 0.9));
        beltbtnanimation->start();
    });
    connect(beltbtn,&QPushButton::released,this,[=]() {
        beltbtnanimation->setStartValue(beltbtn->geometry());
        beltbtnanimation->setEndValue(beltbtnGeometry);
        beltbtnanimation->start();
    });

    balancerbtn = new QPushButton();
    balancerbtn->setParent(this);
    balancerbtn->setFocusPolicy(Qt::NoFocus);
    balancerbtn->setFixedSize(70, 70);
    balancerbtn->setIconSize(QSize(50,50));
    balancerbtn->setIcon(QIcon(":/res/building_icons/balancer.png"));
    balancerbtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    balancerbtn->move(520,810);

    connect(balancerbtn, &QPushButton::clicked, this, [this]() {
        ui_clickEffect.play();
        if(isPlaceItem && currentTile && currentTile->name == "balancer"){
            qDebug() << "cancel placing balancer";
            clearPlacementSelection();
        }else{
            qDebug() << "placing balancer";
            setPlacementTile(createPlacementTile(Tile::Type::Building, "balancer", std::make_pair(1,2)));
        }
        setFocus();
    });

    QPropertyAnimation *balancerbtnanimation = new QPropertyAnimation(balancerbtn, "geometry");
    balancerbtnanimation->setDuration(100);
    balancerbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect balancerbtnGeometry = balancerbtn->geometry();
    connect(balancerbtn,&QPushButton::pressed,this,[=](){
        QRect rect = balancerbtn->geometry();
        balancerbtnanimation->setStartValue(rect);
        balancerbtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.025, rect.y() + rect.height() * 0.025,
                                                rect.width() * 0.95, rect.height() * 0.95));
        balancerbtnanimation->start();
    });
    connect(balancerbtn,&QPushButton::released,this,[=]() {
        balancerbtnanimation->setStartValue(balancerbtn->geometry());
        balancerbtnanimation->setEndValue(balancerbtnGeometry);
        balancerbtnanimation->start();
    });

    underground_beltbtn = new QPushButton();
    underground_beltbtn->setParent(this);
    underground_beltbtn->setFocusPolicy(Qt::NoFocus);
    underground_beltbtn->setFixedSize(70, 70);
    underground_beltbtn->setIconSize(QSize(50,50));
    underground_beltbtn->setIcon(QIcon(":/res/building_icons/underground_belt.png"));
    underground_beltbtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    underground_beltbtn->move(590,810);

    connect(underground_beltbtn, &QPushButton::clicked, this, [this]() {
        ui_clickEffect.play();
        if(isPlaceItem && currentTile && (currentTile->name == "underground_belt_entry" || currentTile->name == "underground_belt_exit")){
            qDebug() << "cancel placing underground_belt";
            clearPlacementSelection();
        }else{
            qDebug() << "placing underground_belt";
            setPlacementTile(createPlacementTile(Tile::Type::Building, "underground_belt_entry"));
        }
        setFocus();
    });

    QPropertyAnimation *underground_beltbtnanimation = new QPropertyAnimation(underground_beltbtn, "geometry");
    underground_beltbtnanimation->setDuration(100);
    underground_beltbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect underground_beltbtnGeometry = underground_beltbtn->geometry();
    connect(underground_beltbtn,&QPushButton::pressed,this,[=](){
        QRect rect = underground_beltbtn->geometry();
        underground_beltbtnanimation->setStartValue(rect);
        underground_beltbtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.025, rect.y() + rect.height() * 0.025,
                                                        rect.width() * 0.95, rect.height() * 0.95));
        underground_beltbtnanimation->start();
    });
    connect(underground_beltbtn,&QPushButton::released,this,[=]() {
        underground_beltbtnanimation->setStartValue(underground_beltbtn->geometry());
        underground_beltbtnanimation->setEndValue(underground_beltbtnGeometry);
        underground_beltbtnanimation->start();
    });

    minerbtn = new QPushButton();
    minerbtn->setParent(this);
    minerbtn->setFocusPolicy(Qt::NoFocus);
    minerbtn->setFixedSize(70, 70);
    minerbtn->setIconSize(QSize(50,50));
    minerbtn->setIcon(QIcon(":/res/building_icons/miner.png"));
    minerbtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    minerbtn->move(660,810);

    connect(minerbtn, &QPushButton::clicked, this, [this]() {
        ui_clickEffect.play();
        if(isPlaceItem && currentTile && currentTile->name == "miner"){
            qDebug() << "cancel placing miner";
            clearPlacementSelection();
        }else{
            qDebug() << "placing miner";
            setPlacementTile(createPlacementTile(Tile::Type::Building, "miner"));
        }
        setFocus();
    });

    QPropertyAnimation *minerbtnanimation = new QPropertyAnimation(minerbtn, "geometry");
    minerbtnanimation->setDuration(100);
    minerbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect minerbtnGeometry = minerbtn->geometry();
    connect(minerbtn,&QPushButton::pressed,this,[=](){
        QRect rect = minerbtn->geometry();
        minerbtnanimation->setStartValue(rect);
        minerbtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.025, rect.y() + rect.height() * 0.025,
                                             rect.width() * 0.95, rect.height() * 0.95));
        minerbtnanimation->start();
    });
    connect(minerbtn,&QPushButton::released,this,[=]() {
        minerbtnanimation->setStartValue(minerbtn->geometry());
        minerbtnanimation->setEndValue(minerbtnGeometry);
        minerbtnanimation->start();
    });

    cutterbtn = new QPushButton();
    cutterbtn->setParent(this);
    cutterbtn->setFocusPolicy(Qt::NoFocus);
    cutterbtn->setFixedSize(70, 70);
    cutterbtn->setIconSize(QSize(50,50));
    cutterbtn->setIcon(QIcon(":/res/building_icons/cutter.png"));
    cutterbtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    cutterbtn->move(730,810);

    connect(cutterbtn, &QPushButton::clicked, this, [this]() {
        ui_clickEffect.play();
        if(isPlaceItem && currentTile && currentTile->name == "cutter"){
            qDebug() << "cancel placing cutter";
            clearPlacementSelection();
        }else{
            qDebug() << "placing cutter";
            setPlacementTile(createPlacementTile(Tile::Type::Building, "cutter", std::make_pair(1,2)));
        }
        setFocus();
    });

    QPropertyAnimation * cutterbtnanimation = new QPropertyAnimation(cutterbtn, "geometry");
    cutterbtnanimation->setDuration(100);
    cutterbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect cutterbtnGeometry = cutterbtn->geometry();
    connect(cutterbtn,&QPushButton::pressed,this,[=](){
        QRect rect = cutterbtn->geometry();
        cutterbtnanimation->setStartValue(rect);
        cutterbtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.025, rect.y() + rect.height() * 0.025,
                                              rect.width() * 0.95, rect.height() * 0.95));
        cutterbtnanimation->start();
    });
    connect(cutterbtn,&QPushButton::released,this,[=]() {
        cutterbtnanimation->setStartValue(cutterbtn->geometry());
        cutterbtnanimation->setEndValue(cutterbtnGeometry);
        cutterbtnanimation->start();
    });

    rotaterbtn = new QPushButton();
    rotaterbtn->setParent(this);
    rotaterbtn->setFocusPolicy(Qt::NoFocus);
    rotaterbtn->setFixedSize(70, 70);
    rotaterbtn->setIconSize(QSize(50,50));
    rotaterbtn->setIcon(QIcon(":/res/building_icons/rotater.png"));
    rotaterbtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    rotaterbtn->move(800,810);

    connect(rotaterbtn, &QPushButton::clicked, this, [this]() {
        ui_clickEffect.play();
        if(isPlaceItem && currentTile && currentTile->name == "rotater"){
            qDebug() << "cancel placing rotater";
            clearPlacementSelection();
        }else{
            qDebug() << "placing rotater";
            setPlacementTile(createPlacementTile(Tile::Type::Building, "rotater"));
        }
        setFocus();
    });

    QPropertyAnimation *rotaterbtnanimation = new QPropertyAnimation(rotaterbtn, "geometry");
    rotaterbtnanimation->setDuration(100);
    rotaterbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect rotaterbtnGeometry = rotaterbtn->geometry();
    connect(rotaterbtn,&QPushButton::pressed,this,[=](){
        QRect rect = rotaterbtn->geometry();
        rotaterbtnanimation->setStartValue(rect);
        rotaterbtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.025, rect.y() + rect.height() * 0.025,
                                               rect.width() * 0.95, rect.height() * 0.95));
        rotaterbtnanimation->start();
    });
    connect(rotaterbtn,&QPushButton::released,this,[=]() {
        rotaterbtnanimation->setStartValue(rotaterbtn->geometry());
        rotaterbtnanimation->setEndValue(rotaterbtnGeometry);
        rotaterbtnanimation->start();
    });

    stackerbtn = new QPushButton();
    stackerbtn->setParent(this);
    stackerbtn->setFocusPolicy(Qt::NoFocus);
    stackerbtn->setFixedSize(70, 70);
    stackerbtn->setIconSize(QSize(50,50));
    stackerbtn->setIcon(QIcon(":/res/building_icons/stacker.png"));
    stackerbtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    stackerbtn->move(870,810);

    connect(stackerbtn, &QPushButton::clicked, this, [this]() {
        ui_clickEffect.play();
        if(isPlaceItem && currentTile && currentTile->name == "stacker"){
            qDebug() << "cancel placing stacker";
            clearPlacementSelection();
        }else{
            qDebug() << "placing stacker";
            setPlacementTile(createPlacementTile(Tile::Type::Building, "stacker", std::make_pair(1,2)));
        }
        setFocus();
    });

    QPropertyAnimation *stackerbtnanimation = new QPropertyAnimation(stackerbtn, "geometry");
    stackerbtnanimation->setDuration(100);
    stackerbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect stackerbtnGeometry = stackerbtn->geometry();
    connect(stackerbtn,&QPushButton::pressed,this,[=](){
        QRect rect = stackerbtn->geometry();
        stackerbtnanimation->setStartValue(rect);
        stackerbtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.025, rect.y() + rect.height() * 0.025,
                                               rect.width() * 0.95, rect.height() * 0.95));
        stackerbtnanimation->start();
    });
    connect(stackerbtn,&QPushButton::released,this,[=]() {
        stackerbtnanimation->setStartValue(stackerbtn->geometry());
        stackerbtnanimation->setEndValue(stackerbtnGeometry);
        stackerbtnanimation->start();
    });

    mixerbtn = new QPushButton();
    mixerbtn->setParent(this);
    mixerbtn->setFocusPolicy(Qt::NoFocus);
    mixerbtn->setFixedSize(70, 70);
    mixerbtn->setIconSize(QSize(50,50));
    mixerbtn->setIcon(QIcon(":/res/building_icons/mixer.png"));
    mixerbtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    mixerbtn->move(940,810);

    connect(mixerbtn, &QPushButton::clicked, this, [this]() {
        ui_clickEffect.play();
        if(isPlaceItem && currentTile && currentTile->name == "mixer"){
            qDebug() << "cancel placing mixer";
            clearPlacementSelection();
        }else{
            qDebug() << "placing mixer";
            setPlacementTile(createPlacementTile(Tile::Type::Building, "mixer", std::make_pair(1,2)));
        }
        setFocus();
    });

    QPropertyAnimation *mixerbtnanimation = new QPropertyAnimation(mixerbtn, "geometry");
    mixerbtnanimation->setDuration(100);
    mixerbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect mixerbtnGeometry = mixerbtn->geometry();
    connect(mixerbtn,&QPushButton::pressed,this,[=](){
        QRect rect = mixerbtn->geometry();
        mixerbtnanimation->setStartValue(rect);
        mixerbtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.025, rect.y() + rect.height() * 0.025,
                                             rect.width() * 0.95, rect.height() * 0.95));
        mixerbtnanimation->start();
    });
    connect(mixerbtn,&QPushButton::released,this,[=]() {
        mixerbtnanimation->setStartValue(mixerbtn->geometry());
        mixerbtnanimation->setEndValue(mixerbtnGeometry);
        mixerbtnanimation->start();
    });

    painterbtn = new QPushButton();
    painterbtn->setParent(this);
    painterbtn->setFocusPolicy(Qt::NoFocus);
    painterbtn->setFixedSize(70, 70);
    painterbtn->setIconSize(QSize(50,50));
    painterbtn->setIcon(QIcon(":/res/building_icons/painter.png"));
    painterbtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    painterbtn->move(1010,810);

    connect(painterbtn, &QPushButton::clicked, this, [this]() {
        ui_clickEffect.play();
        if(isPlaceItem && currentTile && currentTile->name == "painter"){
            qDebug() << "cancel placing painter";
            clearPlacementSelection();
        }else{
            qDebug() << "placing painter";
            setPlacementTile(createPlacementTile(Tile::Type::Building, "painter", std::make_pair(1,2)));
        }
        setFocus();
    });

    QPropertyAnimation *painterbtnanimation = new QPropertyAnimation(painterbtn, "geometry");
    painterbtnanimation->setDuration(100);
    painterbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect painterbtnGeometry = painterbtn->geometry();
    connect(painterbtn,&QPushButton::pressed,this,[=](){
        QRect rect = painterbtn->geometry();
        painterbtnanimation->setStartValue(rect);
        painterbtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.025, rect.y() + rect.height() * 0.025,
                                               rect.width() * 0.95, rect.height() * 0.95));
        painterbtnanimation->start();
    });
    connect(painterbtn,&QPushButton::released,this,[=]() {
        painterbtnanimation->setStartValue(painterbtn->geometry());
        painterbtnanimation->setEndValue(painterbtnGeometry);
        painterbtnanimation->start();
    });

    trashbtn = new QPushButton();
    trashbtn->setParent(this);
    trashbtn->setFocusPolicy(Qt::NoFocus);
    trashbtn->setFixedSize(70, 70);
    trashbtn->setIconSize(QSize(50,50));
    trashbtn->setIcon(QIcon(":/res/building_icons/trash.png"));
    trashbtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    trashbtn->move(1080,810);

    connect(trashbtn, &QPushButton::clicked, this, [this]() {
        ui_clickEffect.play();
        if(isPlaceItem && currentTile && currentTile->name == "trash"){
            qDebug() << "cancel placing trash";
            clearPlacementSelection();
        }else{
            qDebug() << "placing trash";
            setPlacementTile(createPlacementTile(Tile::Type::Building, "trash"));
        }
        setFocus();
    });

    QPropertyAnimation *trashbtnanimation = new QPropertyAnimation(trashbtn, "geometry");
    trashbtnanimation->setDuration(100);
    trashbtnanimation->setEasingCurve(QEasingCurve::OutQuad);
    QRect trashbtnGeometry = trashbtn->geometry();
    connect(trashbtn,&QPushButton::pressed,this,[=](){
        QRect rect = trashbtn->geometry();
        trashbtnanimation->setStartValue(rect);
        trashbtnanimation->setEndValue(QRect(rect.x() + rect.width() * 0.025, rect.y() + rect.height() * 0.025,
                                             rect.width() * 0.95, rect.height() * 0.95));
        trashbtnanimation->start();
    });
    connect(trashbtn,&QPushButton::released,this,[=]() {
        trashbtnanimation->setStartValue(trashbtn->geometry());
        trashbtnanimation->setEndValue(trashbtnGeometry);
        trashbtnanimation->start();
    });

    backbtn = new QPushButton();
    backbtn->setParent(this);
    backbtn->setFocusPolicy(Qt::NoFocus);
    backbtn->setFixedSize(50, 50);
    backbtn->setIconSize(QSize(30,30));
    backbtn->setIcon(QIcon(":/res/settings_menu_exit.png"));
    backbtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    backbtn->move(1525,15);

    connect(backbtn, &QPushButton::clicked, this, &Gamescene::returnToMainScene);

    savebtn = new QPushButton();
    savebtn->setParent(this);
    savebtn->setFocusPolicy(Qt::NoFocus);
    savebtn->setFixedSize(50, 50);
    savebtn->setIconSize(QSize(30,30));
    savebtn->setIcon(QIcon(":/res/save.png"));
    savebtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    savebtn->move(1475,15);

    connect(savebtn, &QPushButton::clicked, this, [this](){
        bool ok;
        QString filename = QInputDialog::getText(
            this,
            t("输入文件名", "Enter File Name"),
            t("请输入存档文件名:", "Please enter a save file name:"),
            QLineEdit::Normal,
            "save.json",
            &ok
        );

        if (ok && !filename.isEmpty()) {
            if (!filename.endsWith(".json")) {
                filename += ".json";
            }
            saveGame(filename);
            autoSaveGame("auto_save.json");
        } else {
            QMessageBox::warning(this, t("警告", "Warning"), t("未输入文件名，保存取消", "No file name was entered. Save was cancelled."));
        }
        //saveGame("save.json");
    });

    upgratebtn = new QPushButton();
    upgratebtn->setParent(this);
    upgratebtn->setFocusPolicy(Qt::NoFocus);
    upgratebtn->setFixedSize(50, 50);
    upgratebtn->setIconSize(QSize(30,30));
    upgratebtn->setIcon(QIcon(":/res/statistics.png"));
    upgratebtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"  // 悬停时背景颜色变深
        "}");
    upgratebtn->move(1425,15);

    connect(upgratebtn, &QPushButton::clicked, this, &Gamescene::showUpgradeOverview);

#ifdef SHAPEZ_ENABLE_DEBUG_CHEATS
    debugNextLevelBtn = new QPushButton(this);
    debugNextLevelBtn->setFocusPolicy(Qt::NoFocus);
    debugNextLevelBtn->setFixedSize(58, 50);
    debugNextLevelBtn->setText("Lv+");
    debugNextLevelBtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 8px;"
        "background-color: transparent;"
        "border: none;"
        "color: #1F2328;"
        "font-weight: 700;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(121, 122, 128, 60);"
        "}");
    debugNextLevelBtn->move(1359,15);
    connect(debugNextLevelBtn, &QPushButton::clicked, this, &Gamescene::skipCurrentLevelForDebug);
#endif

    const QVector<QPushButton *> tooltipButtons = {
        beltbtn, balancerbtn, underground_beltbtn, minerbtn, cutterbtn,
        rotaterbtn, stackerbtn, mixerbtn, painterbtn, trashbtn,
        backbtn, savebtn, upgratebtn
    };
    for (QPushButton *button : tooltipButtons) {
        button->setAttribute(Qt::WA_Hover, true);
        button->setMouseTracking(true);
        button->installEventFilter(this);
    }
#ifdef SHAPEZ_ENABLE_DEBUG_CHEATS
    debugNextLevelBtn->setAttribute(Qt::WA_Hover, true);
    debugNextLevelBtn->setMouseTracking(true);
    debugNextLevelBtn->installEventFilter(this);
#endif

    //测试
    // Tile forwardBelt(Tile::Type::Belt, "forward", NORTH);
    // Tile forwardBelt2(Tile::Type::Belt, "forward", EAST);
    // Tile rightBelt(Tile::Type::Belt, "right", NORTH);
    // Tile northMiner(Tile::Type::Building, NORTH, "miner-chainable");
    // Tile eastMiner(Tile::Type::Building, EAST, "miner-chainable");
    // Tile northCutter(Tile::Type::Building, EAST, "cutter", std::make_pair(1,2));
    // map->setTile(0,0,rightBelt);
    // map->setTile(1,0,forwardBelt);
    // map->setTile(2,0,forwardBelt);
    // map->setTile(0,1,forwardBelt2);
    // map->setTile(5,5,northMiner);
    // map->setTile(5,6,eastMiner);
    // map->setTile(2,20,northCutter);
    // map->setTile(17,0,forwardBelt);
    //map->deleteTile(2,20);

    applyTimerTiers();
    refreshProgressLabels();
    updateInterfaceLayout();
}

void Gamescene::applyTimerTiers()
{
    itemMoveTimer->setInterval(kBeltIntervals[qBound(0, itemMoveTier, kMaxUpgradeTier)]);
    balancerTimer->setInterval(kBalancerIntervals[qBound(0, balancerTier, kMaxUpgradeTier)]);
    undergroundTimer->setInterval(kUndergroundIntervals[qBound(0, undergroundTier, kMaxUpgradeTier)]);
    minerTimer->setInterval(kMinerIntervals[qBound(0, minerTier, kMaxUpgradeTier)]);
    cutterTimer->setInterval(kCutterIntervals[qBound(0, cutterTier, kMaxUpgradeTier)]);
    rotaterTimer->setInterval(kRotaterIntervals[qBound(0, rotaterTier, kMaxUpgradeTier)]);
    stackerTimer->setInterval(kStackerIntervals[qBound(0, stackerTier, kMaxUpgradeTier)]);
    mixerTimer->setInterval(kMixerIntervals[qBound(0, mixerTier, kMaxUpgradeTier)]);
    painterTimer->setInterval(kPainterIntervals[qBound(0, painterTier, kMaxUpgradeTier)]);
    // Update speed display values in map
    map->currentBeltIntervalMs   = itemMoveTimer->interval();
    map->currentBalancerIntervalMs = balancerTimer->interval();
    map->currentUndergroundIntervalMs = undergroundTimer->interval();
    map->currentMinerIntervalMs  = minerTimer->interval();
    map->currentCutterIntervalMs = cutterTimer->interval();
    map->currentRotaterIntervalMs = rotaterTimer->interval();
    map->currentStackerIntervalMs = stackerTimer->interval();
    map->currentMixerIntervalMs = mixerTimer->interval();
    map->currentPainterIntervalMs = painterTimer->interval();
    updateTexts();
}

void Gamescene::updateUndergroundBeltPreview(const QPoint &gridCell)
{
    if (!isPlaceItem || !currentTile) {
        map->undergroundPreviewStart = QPoint(-1, -1);
        map->undergroundPreviewEnd   = QPoint(-1, -1);
        map->update();
        return;
    }
    const QString name = currentTile->name;
    if (name != "underground_belt_entry" && name != "underground_belt_exit") {
        map->undergroundPreviewStart = QPoint(-1, -1);
        map->undergroundPreviewEnd   = QPoint(-1, -1);
        return;
    }

    Tile resolvedTile = resolvedUndergroundPlacementTile(gridCell, &map->undergroundPreviewStart, &map->undergroundPreviewEnd);
    Q_UNUSED(resolvedTile);
    map->update();
}

bool Gamescene::undergroundConnectionForDirection(const QPoint &gridCell, int direction, QString &resolvedName,
                                                  QPoint &previewStart, QPoint &previewEnd) const
{
    int dr = 0;
    int dc = 0;
    switch (direction) {
    case NORTH: dr = -1; break;
    case EAST:  dc = +1; break;
    case SOUTH: dr = +1; break;
    case WEST:  dc = -1; break;
    default: break;
    }

    for (int step = 1; step <= 4; ++step) {
        const QPoint scanPos(gridCell.x() - dr * step, gridCell.y() - dc * step);
        if (!map->inMap(scanPos.x(), scanPos.y())) {
            break;
        }
        Tile *tile = map->tiles[scanPos.x()][scanPos.y()];
        if (tile->name == "underground_belt_entry" && tile->direction == direction) {
            resolvedName = "underground_belt_exit";
            previewStart = scanPos;
            previewEnd = gridCell;
            return true;
        }
    }

    for (int step = 1; step <= 4; ++step) {
        const QPoint scanPos(gridCell.x() + dr * step, gridCell.y() + dc * step);
        if (!map->inMap(scanPos.x(), scanPos.y())) {
            break;
        }
        Tile *tile = map->tiles[scanPos.x()][scanPos.y()];
        if (tile->name == "underground_belt_exit" && tile->direction == direction) {
            resolvedName = "underground_belt_entry";
            previewStart = gridCell;
            previewEnd = scanPos;
            return true;
        }
    }

    previewStart = QPoint(-1, -1);
    previewEnd = QPoint(-1, -1);
    return false;
}

Tile Gamescene::resolvedUndergroundPlacementTile(const QPoint &gridCell, QPoint *previewStart, QPoint *previewEnd) const
{
    Tile resolved(Tile::Type::Building, currentTile ? currentTile->direction : NORTH, "underground_belt_entry");
    if (!currentTile) {
        if (previewStart) {
            *previewStart = QPoint(-1, -1);
        }
        if (previewEnd) {
            *previewEnd = QPoint(-1, -1);
        }
        return resolved;
    }

    QString resolvedName = "underground_belt_entry";
    QPoint bestStart(-1, -1);
    QPoint bestEnd(-1, -1);

    QVector<int> directions{currentTile->direction, NORTH, EAST, SOUTH, WEST};
    QSet<int> seenDirections;
    for (int direction : directions) {
        if (seenDirections.contains(direction)) {
            continue;
        }
        seenDirections.insert(direction);
        if (undergroundConnectionForDirection(gridCell, direction, resolvedName, bestStart, bestEnd)) {
            resolved.direction = direction;
            resolved.name = resolvedName;
            if (previewStart) {
                *previewStart = bestStart;
            }
            if (previewEnd) {
                *previewEnd = bestEnd;
            }
            return resolved;
        }
    }

    resolved.direction = currentTile->direction;
    resolved.name = "underground_belt_entry";
    if (previewStart) {
        *previewStart = QPoint(-1, -1);
    }
    if (previewEnd) {
        *previewEnd = QPoint(-1, -1);
    }
    return resolved;
}

void Gamescene::clearPlacementSelection(){
    isPlaceItem = false;
    map->releaseMouse();
    clearBeltDragPath();
    if (currentTile) {
        delete currentTile;
        currentTile = nullptr;
    }
    map->clearBlueprint();
    map->undergroundPreviewStart = QPoint(-1, -1);
    map->undergroundPreviewEnd   = QPoint(-1, -1);
    map->update();
}

void Gamescene::setPlacementTile(Tile *tile)
{
    if (currentTile) {
        delete currentTile;
        currentTile = nullptr;
    }

    currentTile = tile;
    isPlaceItem = currentTile != nullptr;
    refreshPlacementPreview();
}

Tile *Gamescene::createPlacementTile(Tile::Type type, const QString &name, std::pair<int, int> size) const
{
    const int direction = rememberedDirectionFor(name, type);
    if (type == Tile::Type::Belt) {
        return new Tile(Tile::Type::Belt, "forward", direction);
    }

    return new Tile(type, direction, name, size);
}

int Gamescene::rememberedDirectionFor(const QString &name, Tile::Type type) const
{
    if (type == Tile::Type::Belt) {
        return defaultBeltDirection;
    }

    return rememberedBuildingDirections.value(name, NORTH);
}

void Gamescene::rememberDirectionForCurrentTile()
{
    if (!currentTile) {
        return;
    }

    if (currentTile->type == Tile::Type::Belt) {
        defaultBeltDirection = currentTile->direction;
        return;
    }

    rememberedBuildingDirections[currentTile->name] = currentTile->direction;
}

void Gamescene::refreshPlacementPreview()
{
    if (isBeltDragging) {
        map->clearBlueprint();
        return;
    }

    if (isPlaceItem && currentTile) {
        const QPoint cursorPos = mapFromGlobal(QCursor::pos());
        if (mapViewportRect().contains(cursorPos)) {
            const QPoint mapCursorPos = map->mapFromGlobal(QCursor::pos());
            if (currentTile->name == "underground_belt_entry" || currentTile->name == "underground_belt_exit") {
                const QPoint gridCell = map->gridPositionFromPoint(mapCursorPos);
                Tile resolvedTile = resolvedUndergroundPlacementTile(gridCell, &map->undergroundPreviewStart, &map->undergroundPreviewEnd);
                map->setBlueprintTile(&resolvedTile);
            } else {
                map->undergroundPreviewStart = QPoint(-1, -1);
                map->undergroundPreviewEnd   = QPoint(-1, -1);
                map->setBlueprintTile(currentTile);
            }
            map->updateBlueprintCursor(mapCursorPos);
        } else {
            map->clearBlueprint();
            map->undergroundPreviewStart = QPoint(-1, -1);
            map->undergroundPreviewEnd   = QPoint(-1, -1);
        }
    } else {
        map->clearBlueprint();
        map->undergroundPreviewStart = QPoint(-1, -1);
        map->undergroundPreviewEnd   = QPoint(-1, -1);
    }
}

QRect Gamescene::mapViewportRect() const
{
    return QRect(0, 0, width(), height() - 100);
}

QPoint Gamescene::clampedMapPosition(const QPoint &desiredPosition) const
{
    const QRect viewport = mapViewportRect();
    const QSize mapSize = map->size();

    int x = desiredPosition.x();
    int y = desiredPosition.y();

    if (mapSize.width() <= viewport.width()) {
        x = viewport.left() + (viewport.width() - mapSize.width()) / 2;
    } else {
        const int minX = viewport.right() - mapSize.width() + 1;
        const int maxX = viewport.left();
        x = qBound(minX, x, maxX);
    }

    if (mapSize.height() <= viewport.height()) {
        y = viewport.top() + (viewport.height() - mapSize.height()) / 2;
    } else {
        const int minY = viewport.bottom() - mapSize.height() + 1;
        const int maxY = viewport.top();
        y = qBound(minY, y, maxY);
    }

    return QPoint(x, y);
}

void Gamescene::applyMapViewportMask()
{
    const QRect visibleRect = mapViewportRect().translated(-map->pos());
    map->setMask(QRegion(visibleRect).intersected(QRegion(map->rect())));
}

void Gamescene::panMapBy(const QPoint &delta)
{
    map->move(clampedMapPosition(map->pos() + delta));
    applyMapViewportMask();
}

void Gamescene::zoomMapAt(const QPoint &viewportPos, double zoomDelta)
{
    map->hideBuildingInfo();
    const int oldTileSize = map->tilePixelSize();
    const QPoint oldMapPos = map->pos();
    const QPoint localPosBeforeZoom = viewportPos - oldMapPos;
    const qreal rowCoord = static_cast<qreal>(localPosBeforeZoom.y()) / oldTileSize;
    const qreal colCoord = static_cast<qreal>(localPosBeforeZoom.x()) / oldTileSize;

    const double zoomFactor = zoomDelta > 0 ? 1.15 : (1.0 / 1.15);
    map->setZoomFactor(map->zoomFactor() * zoomFactor);

    const int newTileSize = map->tilePixelSize();
    const QPoint anchoredTopLeft(
        viewportPos.x() - qRound(colCoord * newTileSize),
        viewportPos.y() - qRound(rowCoord * newTileSize)
    );
    map->move(clampedMapPosition(anchoredTopLeft));
    applyMapViewportMask();
    refreshPlacementPreview();
}

QString Gamescene::t(const QString &zhText, const QString &enText) const
{
    return Localization::text(languageCode, zhText, enText);
}

QString Gamescene::upgradeOverviewText() const
{
    return t(
        QString(
            "当前关卡升级一览：\n\n"
            "传送带 Lv.%1：%2 格/秒\n"
            "平衡器 Lv.%3：%4 项/秒\n"
            "地下传送带 Lv.%5：%6 项/秒\n"
            "开采器 Lv.%7：%8 项/秒\n"
            "切割机 Lv.%9：%10 项/秒\n"
            "旋转器 Lv.%11：%12 项/秒\n"
            "合成器 Lv.%13：%14 项/秒\n"
            "混合器 Lv.%15：%16 项/秒\n"
            "染色器 Lv.%17：%18 项/秒\n\n"
            "每完成一个关卡，只能选择一项升级，最高 Lv.%19。\n"
            "同等级下传送带永远最快，平衡器只会略慢一点。")
            .arg(itemMoveTier)
            .arg(itemsPerSecondForInterval(itemMoveTimer->interval()), 0, 'f', 2)
            .arg(balancerTier)
            .arg(itemsPerSecondForInterval(balancerTimer->interval()), 0, 'f', 2)
            .arg(undergroundTier)
            .arg(itemsPerSecondForInterval(undergroundTimer->interval()), 0, 'f', 2)
            .arg(minerTier)
            .arg(itemsPerSecondForInterval(minerTimer->interval()), 0, 'f', 2)
            .arg(cutterTier)
            .arg(itemsPerSecondForInterval(cutterTimer->interval()), 0, 'f', 2)
            .arg(rotaterTier)
            .arg(itemsPerSecondForInterval(rotaterTimer->interval()), 0, 'f', 2)
            .arg(stackerTier)
            .arg(itemsPerSecondForInterval(stackerTimer->interval()), 0, 'f', 2)
            .arg(mixerTier)
            .arg(itemsPerSecondForInterval(mixerTimer->interval()), 0, 'f', 2)
            .arg(painterTier)
            .arg(itemsPerSecondForInterval(painterTimer->interval()), 0, 'f', 2)
            .arg(kMaxUpgradeTier),
        QString(
            "Current level-up upgrades:\n\n"
            "Belts Lv.%1: %2 tiles/s\n"
            "Balancers Lv.%3: %4 items/s\n"
            "Underground Belts Lv.%5: %6 items/s\n"
            "Miners Lv.%7: %8 items/s\n"
            "Cutters Lv.%9: %10 items/s\n"
            "Rotators Lv.%11: %12 items/s\n"
            "Stackers Lv.%13: %14 items/s\n"
            "Mixers Lv.%15: %16 items/s\n"
            "Painters Lv.%17: %18 items/s\n\n"
            "After each completed level, you choose exactly one upgrade, up to Lv.%19.\n"
            "Belts are always the fastest at the same tier, with balancers only slightly behind.")
            .arg(itemMoveTier)
            .arg(itemsPerSecondForInterval(itemMoveTimer->interval()), 0, 'f', 2)
            .arg(balancerTier)
            .arg(itemsPerSecondForInterval(balancerTimer->interval()), 0, 'f', 2)
            .arg(undergroundTier)
            .arg(itemsPerSecondForInterval(undergroundTimer->interval()), 0, 'f', 2)
            .arg(minerTier)
            .arg(itemsPerSecondForInterval(minerTimer->interval()), 0, 'f', 2)
            .arg(cutterTier)
            .arg(itemsPerSecondForInterval(cutterTimer->interval()), 0, 'f', 2)
            .arg(rotaterTier)
            .arg(itemsPerSecondForInterval(rotaterTimer->interval()), 0, 'f', 2)
            .arg(stackerTier)
            .arg(itemsPerSecondForInterval(stackerTimer->interval()), 0, 'f', 2)
            .arg(mixerTier)
            .arg(itemsPerSecondForInterval(mixerTimer->interval()), 0, 'f', 2)
            .arg(painterTier)
            .arg(itemsPerSecondForInterval(painterTimer->interval()), 0, 'f', 2)
            .arg(kMaxUpgradeTier));
}

void Gamescene::showUpgradeOverview()
{
    auto localizedRate = [this](double rate, bool usesTiles) {
        return t(
            QString("%1 %2").arg(rate, 0, 'f', 2).arg(usesTiles ? "格/秒" : "项/秒"),
            QString("%1 %2").arg(rate, 0, 'f', 2).arg(usesTiles ? "tiles/s" : "items/s"));
    };

    auto makeOverviewEntry = [this, &localizedRate](int optionId,
                                                    int currentTier,
                                                    const QString &iconPath,
                                                    const QString &zhName,
                                                    const QString &enName,
                                                    const int *intervals,
                                                    bool usesTiles) {
        UpgradeDialogEntry entry;
        entry.optionId = optionId;
        entry.currentTier = currentTier;
        entry.iconPath = iconPath;
        entry.displayName = t(zhName, enName);

        const double currentRate = itemsPerSecondForInterval(intervals[qBound(0, currentTier, kMaxUpgradeTier)]);
        const QString currentRateText = localizedRate(currentRate, usesTiles);
        entry.description = t(
            QString("当前效率：%1").arg(currentRateText),
            QString("Current throughput: %1").arg(currentRateText));

        if (currentTier < kMaxUpgradeTier) {
            const double nextRate = itemsPerSecondForInterval(intervals[qBound(0, currentTier + 1, kMaxUpgradeTier)]);
            const QString nextRateText = localizedRate(nextRate, usesTiles);
            entry.effectTooltip = t(
                QString("%1 Lv.%2 -> Lv.%3\n效率：%4 -> %5")
                    .arg(zhName)
                    .arg(currentTier)
                    .arg(currentTier + 1)
                    .arg(currentRateText)
                    .arg(nextRateText),
                QString("%1 Lv.%2 -> Lv.%3\nThroughput: %4 -> %5")
                    .arg(enName)
                    .arg(currentTier)
                    .arg(currentTier + 1)
                    .arg(currentRateText)
                    .arg(nextRateText));
        } else {
            entry.effectTooltip = t(
                QString("%1 已达到最高等级。\n当前效率：%2").arg(zhName, currentRateText),
                QString("%1 is already at max level.\nCurrent throughput: %2").arg(enName, currentRateText));
        }

        return entry;
    };

    const QVector<UpgradeDialogEntry> overviewEntries = {
        makeOverviewEntry(1, itemMoveTier, QStringLiteral(":/res/building_icons/belt.png"),
                          QStringLiteral("传送带"), QStringLiteral("Belts"), kBeltIntervals, true),
        makeOverviewEntry(2, balancerTier, QStringLiteral(":/res/building_icons/balancer.png"),
                          QStringLiteral("平衡器"), QStringLiteral("Balancers"), kBalancerIntervals, false),
        makeOverviewEntry(3, undergroundTier, QStringLiteral(":/res/building_icons/underground_belt.png"),
                          QStringLiteral("地下传送带"), QStringLiteral("Underground Belts"), kUndergroundIntervals, false),
        makeOverviewEntry(4, minerTier, QStringLiteral(":/res/building_icons/miner.png"),
                          QStringLiteral("开采器"), QStringLiteral("Miners"), kMinerIntervals, false),
        makeOverviewEntry(5, cutterTier, QStringLiteral(":/res/building_icons/cutter.png"),
                          QStringLiteral("切割机"), QStringLiteral("Cutters"), kCutterIntervals, false),
        makeOverviewEntry(6, rotaterTier, QStringLiteral(":/res/building_icons/rotater.png"),
                          QStringLiteral("旋转器"), QStringLiteral("Rotators"), kRotaterIntervals, false),
        makeOverviewEntry(7, stackerTier, QStringLiteral(":/res/building_icons/stacker.png"),
                          QStringLiteral("合成器"), QStringLiteral("Stackers"), kStackerIntervals, false),
        makeOverviewEntry(8, mixerTier, QStringLiteral(":/res/building_icons/mixer.png"),
                          QStringLiteral("混合器"), QStringLiteral("Mixers"), kMixerIntervals, false),
        makeOverviewEntry(9, painterTier, QStringLiteral(":/res/building_icons/painter.png"),
                          QStringLiteral("染色器"), QStringLiteral("Painters"), kPainterIntervals, false)
    };

    UpgradeOverviewDialog dialog(languageCode, overviewEntries, pendingUpgradePoints, this);
    dialog.exec();
    bool spentPoints = false;
    for (int optionId : dialog.getSelectedOptions()) {
        if (pendingUpgradePoints > 0 && applyUpgradeOption(optionId)) {
            pendingUpgradePoints--;
            spentPoints = true;
        }
    }
    if (spentPoints) {
        applyTimerTiers();
    } else {
        updateTexts();
    }
    setFocus();
}

void Gamescene::updateTexts()
{
    beltbtn->setToolTip(t(
        QString("传送带 (Lv.%1)\n沿朝向运输物品，拖动铺设时可以自动形成转弯。").arg(itemMoveTier),
        QString("Belt (Lv.%1)\nMoves items forward and can automatically form turns while dragging belts.").arg(itemMoveTier)));
    balancerbtn->setToolTip(t(
        QString("平衡器 (Lv.%1)\n把两路输入平均分配到两路输出，速度只略慢于同级传送带。").arg(balancerTier),
        QString("Balancer (Lv.%1)\nSplits two input flows evenly across two outputs and stays close to belt speed.").arg(balancerTier)));
    underground_beltbtn->setToolTip(t(
        QString("地下传送带 (Lv.%1)\n让物品从地下穿过拥挤区域，最远4格。放置时自动判断入口/出口。").arg(undergroundTier),
        QString("Underground Belt (Lv.%1)\nSends items underground. Range: 4 tiles. Auto-detects entry/exit on placement.").arg(undergroundTier)));
    minerbtn->setToolTip(t(
        QString("开采器 (Lv.%1)\n放在矿物上持续采集资源，并朝前方输出。").arg(minerTier),
        QString("Miner (Lv.%1)\nExtracts resources from ore tiles continuously and outputs them forward.").arg(minerTier)));
    cutterbtn->setToolTip(t(
        QString("切割机 (Lv.%1)\n把输入图形切成两部分，为更复杂的目标做准备。").arg(cutterTier),
        QString("Cutter (Lv.%1)\nCuts incoming shapes into two parts for more advanced production goals.").arg(cutterTier)));
    rotaterbtn->setToolTip(t(
        QString("旋转器 (Lv.%1)\n将输入图形顺时针旋转90°后输出，用来调整拼装方向。").arg(rotaterTier),
        QString("Rotator (Lv.%1)\nRotates incoming shapes 90° CW before output so you can align later processing.").arg(rotaterTier)));
    stackerbtn->setToolTip(t(
        QString("合成器 (Lv.%1)\n将两路不重叠的图形合成为一个目标，用来制作多矿物关卡。").arg(stackerTier),
        QString("Stacker (Lv.%1)\nCombines two non-overlapping shapes into one item for mixed-resource goals.").arg(stackerTier)));
    mixerbtn->setToolTip(t(
        QString("混合器 (Lv.%1)\n使用 RGB 加色法混色：红+绿=黄，红+蓝=紫，绿+蓝=青，三色凑齐会得到白。").arg(mixerTier),
        QString("Mixer (Lv.%1)\nUses additive RGB color mixing: red+green=yellow, red+blue=purple, green+blue=cyan, and all three channels combine into white.").arg(mixerTier)));
    painterbtn->setToolTip(t(
        QString("染色器 (Lv.%1)\n给输入图形整体上色。默认朝向时图形从左进，染料从右格上方进，右侧输出。").arg(painterTier),
        QString("Painter (Lv.%1)\nApplies dye to a whole shape. In its default orientation, shapes enter from the left, dye enters the right cell from above, and output leaves on the right.").arg(painterTier)));
    trashbtn->setToolTip(t(
        "垃圾桶\n销毁输入物品，用来清理多余或错误的产物。",
        "Trash\nDeletes incoming items so you can remove extra or incorrect outputs."));
#ifdef SHAPEZ_ENABLE_DEBUG_CHEATS
    if (debugNextLevelBtn) {
        debugNextLevelBtn->setToolTip(t(
            "调试：下一关\n立即完成当前关卡并进入升级选择。快捷键：F10",
            "Debug: Next Level\nInstantly completes the current level and opens the upgrade choice. Shortcut: F10"));
    }
#endif
    backbtn->setToolTip(t("返回主界面", "Back to Main Menu"));
    savebtn->setToolTip(t("保存游戏", "Save Game"));
    upgratebtn->setToolTip(t(
        QString("升级概览\n当前保留升级点：%1").arg(pendingUpgradePoints),
        QString("Upgrade Overview\nSaved upgrade points: %1").arg(pendingUpgradePoints)));
}

int Gamescene::directionForStep(const QPoint &from, const QPoint &to) const
{
    const int rowDelta = to.x() - from.x();
    const int columnDelta = to.y() - from.y();

    if (rowDelta == -1 && columnDelta == 0) {
        return NORTH;
    }
    if (rowDelta == 1 && columnDelta == 0) {
        return SOUTH;
    }
    if (rowDelta == 0 && columnDelta == 1) {
        return EAST;
    }
    if (rowDelta == 0 && columnDelta == -1) {
        return WEST;
    }

    return defaultBeltDirection;
}

Tile Gamescene::beltTileForPathIndex(int index) const
{
    if (beltDragPath.size() == 1) {
        if (beltDragConnectionTarget.x() >= 0) {
            return Tile(Tile::Type::Belt, "forward",
                        directionForStep(beltDragPath.front(), beltDragConnectionTarget));
        }
        return currentTile ? Tile(*currentTile) : Tile(Tile::Type::Belt, "forward", defaultBeltDirection);
    }

    if (beltDragPath.isEmpty()) {
        return Tile(Tile::Type::Belt, "forward", defaultBeltDirection);
    }

    const int incomingDirection = (index == 0)
        ? directionForStep(beltDragPath[index], beltDragPath[index + 1])
        : directionForStep(beltDragPath[index - 1], beltDragPath[index]);
    const int outgoingDirection = (index == beltDragPath.size() - 1)
        ? (beltDragConnectionTarget.x() >= 0
            ? directionForStep(beltDragPath[index], beltDragConnectionTarget)
            : incomingDirection)
        : directionForStep(beltDragPath[index], beltDragPath[index + 1]);

    QString state = "forward";
    if (outgoingDirection == (incomingDirection + 3) % 4) {
        state = "left";
    } else if (outgoingDirection == (incomingDirection + 1) % 4) {
        state = "right";
    }

    return Tile(Tile::Type::Belt, state, incomingDirection);
}

bool Gamescene::isCellAvailableForDraggedBelt(const QPoint &cell) const
{
    if (!map->inMap(cell.x(), cell.y())) {
        return false;
    }

    if (beltDragPath.contains(cell)) {
        return true;
    }

    return map->canPlaceTile(cell.x(), cell.y(), *currentTile);
}

bool Gamescene::canConnectDraggedBeltToCell(const QPoint &from, const QPoint &target) const
{
    if (!map->inMap(target.x(), target.y()) || beltDragPath.contains(target)) {
        return false;
    }

    const Tile targetTile = map->getTile(target.x(), target.y());
    if (targetTile.type == Tile::Type::Empty ||
        targetTile.type == Tile::Type::Resource ||
        targetTile.type == Tile::Type::Color) {
        return false;
    }

    const int incomingDirection = directionForStep(from, target);
    if (targetTile.type == Tile::Type::Belt) {
        return targetTile.direction == incomingDirection;
    }
    if (targetTile.type == Tile::Type::Hub) {
        return true;
    }
    if (targetTile.type == Tile::Type::Building && targetTile.name == "trash") {
        return true;
    }

    return map->canEnter(incomingDirection, std::make_pair(target.x(), target.y()));
}

void Gamescene::clearBeltDragPath()
{
    activeDraggedBeltCells.clear();
    beltDragPath.clear();
    beltDragConnectionTarget = QPoint(-1, -1);
    isBeltDragging = false;
}

void Gamescene::rebuildBeltDragPath()
{
    for (const QPoint &cell : activeDraggedBeltCells) {
        if (map->inMap(cell.x(), cell.y()) && map->getTile(cell.x(), cell.y()).type == Tile::Type::Belt) {
            map->deleteTile(cell.x(), cell.y());
        }
    }

    activeDraggedBeltCells.clear();

    for (int index = 0; index < beltDragPath.size(); ++index) {
        const QPoint &cell = beltDragPath[index];
        Tile beltTile = beltTileForPathIndex(index);
        map->setTile(cell.x(), cell.y(), beltTile, false);
        activeDraggedBeltCells.append(cell);
    }
}

void Gamescene::beginBeltDrag(const QPoint &startCell)
{
    isBeltDragging = true;
    beltDragConnectionTarget = QPoint(-1, -1);
    beltDragPath = {startCell};
    rebuildBeltDragPath();
    map->clearBlueprint();
    map->grabMouse();
}

void Gamescene::updateBeltDragPath(const QPoint &targetCell)
{
    if (!isBeltDragging || beltDragPath.isEmpty()) {
        return;
    }

    const bool hadConnectionTarget = beltDragConnectionTarget.x() >= 0;
    beltDragConnectionTarget = QPoint(-1, -1);
    if (targetCell == beltDragPath.back() && !hadConnectionTarget) {
        return;
    }

    QPoint currentCell = beltDragPath.back();

    while (currentCell != targetCell) {
        const int rowDelta = targetCell.x() - currentCell.x();
        const int columnDelta = targetCell.y() - currentCell.y();
        QVector<QPoint> candidateCells;

        auto appendCandidate = [&candidateCells](const QPoint &candidate) {
            if (!candidateCells.contains(candidate)) {
                candidateCells.append(candidate);
            }
        };

        if (rowDelta != 0 || columnDelta != 0) {
            const QPoint rowCandidate(currentCell.x() + (rowDelta > 0 ? 1 : -1), currentCell.y());
            const QPoint columnCandidate(currentCell.x(), currentCell.y() + (columnDelta > 0 ? 1 : -1));

            if (rowDelta != 0 && columnDelta != 0) {
                if (qAbs(rowDelta) >= qAbs(columnDelta)) {
                    appendCandidate(rowCandidate);
                    appendCandidate(columnCandidate);
                } else {
                    appendCandidate(columnCandidate);
                    appendCandidate(rowCandidate);
                }
            } else if (rowDelta != 0) {
                appendCandidate(rowCandidate);
            } else if (columnDelta != 0) {
                appendCandidate(columnCandidate);
            }
        }

        if (candidateCells.isEmpty()) {
            break;
        }

        bool advanced = false;
        for (const QPoint &nextCell : candidateCells) {
            if (beltDragPath.size() > 1 && nextCell == beltDragPath[beltDragPath.size() - 2]) {
                beltDragPath.removeLast();
                currentCell = beltDragPath.back();
                advanced = true;
                break;
            }

            if (isCellAvailableForDraggedBelt(nextCell)) {
                beltDragPath.append(nextCell);
                currentCell = nextCell;
                advanced = true;
                break;
            }

            if (nextCell == targetCell && canConnectDraggedBeltToCell(currentCell, nextCell)) {
                beltDragConnectionTarget = nextCell;
                currentCell = targetCell;
                advanced = true;
                break;
            }
        }

        if (!advanced) {
            break;
        }
    }

    rebuildBeltDragPath();
}

bool Gamescene::handleMapMousePress(QMouseEvent *event)
{
    map->hideBuildingInfo();

    if (event->button() == Qt::MiddleButton || (event->button() == Qt::LeftButton && !isPlaceItem)) {
        isPanning = true;
        lastPanGlobalPos = event->globalPosition().toPoint();
        map->grabMouse();
        return true;
    }

    const QPoint gridPos = map->gridPositionFromPoint(event->position().toPoint());
    const int gridX = gridPos.x();
    const int gridY = gridPos.y();

    if (event->button() == Qt::LeftButton) {
        if (!isPlaceItem || !currentTile) {
            return true;
        }

        if (currentTile->type == Tile::Type::Belt) {
            if (!map->canPlaceTile(gridX, gridY, *currentTile)) {
                qDebug() << "pos(" << gridX << "," << gridY << ") cannot place selected tile";
                return true;
            }

            beginBeltDrag(QPoint(gridX, gridY));
            return true;
        }

        Tile resolvedPlacement = *currentTile;
        if (currentTile->name == "underground_belt_entry" || currentTile->name == "underground_belt_exit") {
            resolvedPlacement = resolvedUndergroundPlacementTile(QPoint(gridX, gridY));
        }

        if (!map->canPlaceTile(gridX, gridY, resolvedPlacement)) {
            qDebug() << "pos(" << gridX << "," << gridY << ") cannot place selected tile";
            return true;
        }

        if (currentTile->name == "underground_belt_entry" || currentTile->name == "underground_belt_exit") {
            map->setTile(gridX, gridY, resolvedPlacement);
            clearPlacementSelection();
            return true;
        }

        map->setTile(gridX, gridY, resolvedPlacement);
        clearPlacementSelection();
        return true;
    }

    if (event->button() == Qt::RightButton) {
        if (isPlaceItem) {
            clearPlacementSelection();
            return true;
        }

        if (!map->inMap(gridX, gridY)) {
            return true;
        }

        map->deleteTile(gridX, gridY);
        return true;
    }

    return false;
}

bool Gamescene::handleMapMouseMove(QMouseEvent *event)
{
    if (isBeltDragging) {
        updateBeltDragPath(map->gridPositionFromPoint(event->position().toPoint()));
        return true;
    }

    if (isPlaceItem && currentTile) {
        const QPoint mapPos = event->position().toPoint();
        if (currentTile->name == "underground_belt_entry" || currentTile->name == "underground_belt_exit") {
            const QPoint gridCell = map->gridPositionFromPoint(mapPos);
            Tile resolvedTile = resolvedUndergroundPlacementTile(gridCell, &map->undergroundPreviewStart, &map->undergroundPreviewEnd);
            map->setBlueprintTile(&resolvedTile);
        } else {
            map->undergroundPreviewStart = QPoint(-1, -1);
            map->undergroundPreviewEnd   = QPoint(-1, -1);
            map->setBlueprintTile(currentTile);
        }
        map->updateBlueprintCursor(mapPos);
    } else {
        map->clearBlueprint();
        map->undergroundPreviewStart = QPoint(-1, -1);
        map->undergroundPreviewEnd   = QPoint(-1, -1);
    }

    if (!isPanning) {
        return false;
    }

    const QPoint globalPos = event->globalPosition().toPoint();
    panMapBy(globalPos - lastPanGlobalPos);
    lastPanGlobalPos = globalPos;
    return true;
}

bool Gamescene::handleMapMouseRelease(QMouseEvent *event)
{
    if (isBeltDragging && event->button() == Qt::LeftButton) {
        map->releaseMouse();
        clearPlacementSelection();
        return true;
    }

    if (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton) {
        isPanning = false;
        map->releaseMouse();
        return true;
    }

    return false;
}

void Gamescene::placeRandomResourceCluster(const QString &resourceName, int rowMin, int rowMax, int colMin, int colMax)
{
    Tile resourceTile(Tile::Type::Resource, NORTH, resourceName);

    for (int attempt = 0; attempt < 160; ++attempt) {
        const int anchorRow = QRandomGenerator::global()->bounded(rowMin, rowMax + 1);
        const int anchorCol = QRandomGenerator::global()->bounded(colMin, colMax + 1);
        const QPoint anchor(anchorRow, anchorCol);

        if (!map->inMap(anchorRow, anchorCol) || map->getTile(anchorRow, anchorCol).type != Tile::Type::Empty) {
            continue;
        }

        QVector<QPoint> cluster{anchor};
        const int targetSize = QRandomGenerator::global()->bounded(4, 16);

        for (int growthAttempt = 0; growthAttempt < 320 && cluster.size() < targetSize; ++growthAttempt) {
            const QPoint base = cluster[QRandomGenerator::global()->bounded(cluster.size())];
            QPoint candidate = base;

            switch (QRandomGenerator::global()->bounded(4)) {
            case 0:
                candidate.rx() -= 1;
                break;
            case 1:
                candidate.rx() += 1;
                break;
            case 2:
                candidate.ry() -= 1;
                break;
            default:
                candidate.ry() += 1;
                break;
            }

            if (candidate.x() < rowMin || candidate.x() > rowMax || candidate.y() < colMin || candidate.y() > colMax) {
                continue;
            }
            if (!map->inMap(candidate.x(), candidate.y())) {
                continue;
            }
            if (cluster.contains(candidate) || map->getTile(candidate.x(), candidate.y()).type != Tile::Type::Empty) {
                continue;
            }

            cluster.append(candidate);
        }

        if (cluster.size() < 8) {
            continue;
        }

        for (const QPoint &cell : cluster) {
            map->setTile(cell.x(), cell.y(), resourceTile, false);
        }
        return;
    }
}

void Gamescene::placeRandomColorCluster(const QString &colorName)
{
    Tile colorTile(Tile::Type::Color, NORTH, colorName);
    const int rowMin = 4;
    const int rowMax = map->getheight() - 5;
    const int colMin = 5;
    const int colMax = map->getwidth() - 6;

    const auto hasOreNeighbor = [this](const QPoint &cell) {
        static const QVector<QPoint> offsets = {
            QPoint(-1, 0), QPoint(1, 0), QPoint(0, -1), QPoint(0, 1)
        };
        for (const QPoint &offset : offsets) {
            const QPoint neighbor = cell + offset;
            if (!map->inMap(neighbor.x(), neighbor.y())) {
                continue;
            }
            const Tile neighborTile = map->getTile(neighbor.x(), neighbor.y());
            if (neighborTile.type == Tile::Type::Resource || neighborTile.type == Tile::Type::Color) {
                return true;
            }
        }
        return false;
    };

    const auto tryPlaceCluster = [&](bool attachToExisting) {
        for (int attempt = 0; attempt < 220; ++attempt) {
            const QPoint anchor(
                QRandomGenerator::global()->bounded(rowMin, rowMax + 1),
                QRandomGenerator::global()->bounded(colMin, colMax + 1)
            );

            if (map->getTile(anchor.x(), anchor.y()).type != Tile::Type::Empty) {
                continue;
            }

            if (attachToExisting && !hasOreNeighbor(anchor)) {
                continue;
            }
            if (!attachToExisting && hasOreNeighbor(anchor)) {
                continue;
            }

            QVector<QPoint> cluster{anchor};
            const int targetSize = QRandomGenerator::global()->bounded(3, 5);
            for (int growthAttempt = 0; growthAttempt < 80 && cluster.size() < targetSize; ++growthAttempt) {
                const QPoint base = cluster[QRandomGenerator::global()->bounded(cluster.size())];
                QPoint candidate = base;

                switch (QRandomGenerator::global()->bounded(4)) {
                case 0:
                    candidate.rx() -= 1;
                    break;
                case 1:
                    candidate.rx() += 1;
                    break;
                case 2:
                    candidate.ry() -= 1;
                    break;
                default:
                    candidate.ry() += 1;
                    break;
                }

                if (candidate.x() < rowMin || candidate.x() > rowMax || candidate.y() < colMin || candidate.y() > colMax) {
                    continue;
                }
                if (cluster.contains(candidate) || map->getTile(candidate.x(), candidate.y()).type != Tile::Type::Empty) {
                    continue;
                }
                if (!attachToExisting && hasOreNeighbor(candidate)) {
                    continue;
                }

                cluster.append(candidate);
            }

            if (cluster.size() < 3) {
                continue;
            }

            for (const QPoint &cell : cluster) {
                map->setTile(cell.x(), cell.y(), colorTile, false);
            }
            return true;
        }

        return false;
    };

    const bool preferAttached = true;
    if (!tryPlaceCluster(preferAttached)) {
        tryPlaceCluster(!preferAttached);
    }
}

void Gamescene::populateStartingResources()
{
    const int topRowMin = 4;
    const int topRowMax = qMax(topRowMin, map->getheight() / 2 - 4);
    const int bottomRowMin = qMin(map->getheight() - 5, map->getheight() / 2 + 3);
    const int bottomRowMax = map->getheight() - 5;
    const int leftColMin = 5;
    const int leftColMax = qMax(leftColMin, map->getwidth() / 2 - 11);
    const int rightColMin = qMin(map->getwidth() - 6, map->getwidth() / 2 + 10);
    const int rightColMax = map->getwidth() - 6;

    auto placeTwoClusters = [this, topRowMin, topRowMax, bottomRowMin, bottomRowMax, leftColMin, leftColMax, rightColMin, rightColMax](const QString &resourceName) {
        QVector<int> cornerIndices{0, 1, 2, 3};
        for (int clusterIndex = 0; clusterIndex < 2; ++clusterIndex) {
            const int pickedIndex = QRandomGenerator::global()->bounded(cornerIndices.size());
            const int corner = cornerIndices.takeAt(pickedIndex);

            switch (corner) {
            case 0:
                placeRandomResourceCluster(resourceName, topRowMin, topRowMax, leftColMin, leftColMax);
                break;
            case 1:
                placeRandomResourceCluster(resourceName, topRowMin, topRowMax, rightColMin, rightColMax);
                break;
            case 2:
                placeRandomResourceCluster(resourceName, bottomRowMin, bottomRowMax, leftColMin, leftColMax);
                break;
            default:
                placeRandomResourceCluster(resourceName, bottomRowMin, bottomRowMax, rightColMin, rightColMax);
                break;
            }
        }
    };

    placeTwoClusters("circle");
    placeTwoClusters("square");
    placeTwoClusters("diamond");
    placeRandomColorCluster("red");
    placeRandomColorCluster("green");
    placeRandomColorCluster("blue");
}

void Gamescene::updateInterfaceLayout()
{
    const int toolbarY = height() - 90;
    const int buildButtonStartX = (width() - 700) / 2;
    const int buildButtonSpacing = 70;

    QVector<QPushButton *> buildButtons = {
        beltbtn,
        balancerbtn,
        underground_beltbtn,
        minerbtn,
        cutterbtn,
        rotaterbtn,
        stackerbtn,
        mixerbtn,
        painterbtn,
        trashbtn
    };

    for (int index = 0; index < buildButtons.size(); ++index) {
        buildButtons[index]->move(buildButtonStartX + index * buildButtonSpacing, toolbarY);
    }

    backbtn->move(width() - 75, 15);
    savebtn->move(width() - 125, 15);
    upgratebtn->move(width() - 175, 15);
#ifdef SHAPEZ_ENABLE_DEBUG_CHEATS
    if (debugNextLevelBtn) {
        debugNextLevelBtn->move(width() - 239, 15);
        debugNextLevelBtn->raise();
    }
#endif
    backbtn->raise();
    savebtn->raise();
    upgratebtn->raise();

    QPoint desiredPosition = map->pos();
    if (!hasInitializedMapPosition) {
        desiredPosition = QPoint(
            (mapViewportRect().width() - map->size().width()) / 2,
            (mapViewportRect().height() - map->size().height()) / 2
        );
        hasInitializedMapPosition = true;
    }
    map->move(clampedMapPosition(desiredPosition));
    applyMapViewportMask();
    update();
}

std::array<int, 4> Gamescene::generatedGoalForLevel(int levelIndex) const
{
    if (levelIndex == 0) {
        return fullGoal(CIRCLE);
    }
    if (levelIndex == 1) {
        return fullGoal(SQUARE);
    }
    if (levelIndex == 2) {
        return {SQUARE, EMPTY, SQUARE, EMPTY};
    }

    QRandomGenerator generator(0x5EED1234u + static_cast<quint32>(levelIndex * 2654435761u));
    QVector<int> palette = {CIRCLE, SQUARE, DIAMOND};
    shuffleVector(palette, generator);

    const int usableTypes = qMin(1 + (levelIndex - 2) / 3, 3);
    const int filledQuadrants = qMin(2 + (levelIndex - 3) / 2, 4);
    GoalParts goal = {EMPTY, EMPTY, EMPTY, EMPTY};

    QVector<int> quadrants = {0, 1, 2, 3};
    shuffleVector(quadrants, generator);

    const int guaranteedTypes = qMin(usableTypes, filledQuadrants);
    for (int index = 0; index < filledQuadrants; ++index) {
        const int paletteIndex = index < guaranteedTypes
            ? index
            : generator.bounded(usableTypes);
        goal[quadrants[index]] = palette[paletteIndex];
    }

    if (levelIndex >= 6 && distinctTypeCount(goal) < qMin(usableTypes, 2)) {
        for (int quadrant : quadrants) {
            if (goal[quadrant] != EMPTY) {
                goal[quadrant] = palette[qMin(1, usableTypes - 1)];
                break;
            }
        }
    }

    return goal;
}

std::array<QString, 4> Gamescene::generatedGoalColorsForLevel(int levelIndex, const std::array<int, 4> &parts) const
{
    if (levelIndex < 8) {
        return defaultGoalColorsForParts(parts);
    }

    QRandomGenerator generator(0xC01A1234u + static_cast<quint32>(levelIndex * 2246822519u));
    QVector<QString> colorPalette;
    if (levelIndex < 11) {
        colorPalette = {"red", "green", "blue"};
    } else if (levelIndex < 14) {
        colorPalette = {"yellow", "cyan", "purple"};
    } else {
        colorPalette = {"red", "green", "blue", "yellow", "cyan", "purple", "white"};
    }

    const QString selectedColor = colorPalette[generator.bounded(colorPalette.size())];
    return defaultGoalColorsForParts(parts, selectedColor);
}

void Gamescene::refreshProgressLabels()
{
    map->countLabel->setText(QString("%1\n/%2").arg(map->current).arg(map->target));
    map->levelLabel->setText(QString("%1").arg(map->questionLever + 1));
    map->questionLabel->raise();
    map->countLabel->raise();
    map->levelLabel->raise();
}

bool Gamescene::applyUpgradeOption(int optionId)
{
    switch (optionId) {
    case 1:
        if (itemMoveTier < kMaxUpgradeTier) {
            itemMoveTier++;
            return true;
        }
        break;
    case 2:
        if (balancerTier < kMaxUpgradeTier) {
            balancerTier++;
            return true;
        }
        break;
    case 3:
        if (undergroundTier < kMaxUpgradeTier) {
            undergroundTier++;
            return true;
        }
        break;
    case 4:
        if (minerTier < kMaxUpgradeTier) {
            minerTier++;
            return true;
        }
        break;
    case 5:
        if (cutterTier < kMaxUpgradeTier) {
            cutterTier++;
            return true;
        }
        break;
    case 6:
        if (rotaterTier < kMaxUpgradeTier) {
            rotaterTier++;
            return true;
        }
        break;
    case 7:
        if (stackerTier < kMaxUpgradeTier) {
            stackerTier++;
            return true;
        }
        break;
    case 8:
        if (mixerTier < kMaxUpgradeTier) {
            mixerTier++;
            return true;
        }
        break;
    case 9:
        if (painterTier < kMaxUpgradeTier) {
            painterTier++;
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

void Gamescene::tryAdvanceLevel()
{
    if (map->current < map->target) {
        return;
    }

    const QVector<QTimer *> simulationTimers = {
        itemMoveTimer,
        balancerTimer,
        undergroundTimer,
        minerTimer,
        cutterTimer,
        rotaterTimer,
        stackerTimer,
        mixerTimer,
        painterTimer
    };
    for (QTimer *timer : simulationTimers) {
        timer->stop();
    }

    map->questionLever++;
    pendingUpgradePoints++;
    setPuzzle();

    auto localizedRate = [this](double rate, bool usesTiles) {
        return t(
            QString("%1 %2").arg(rate, 0, 'f', 2).arg(usesTiles ? "格/秒" : "项/秒"),
            QString("%1 %2").arg(rate, 0, 'f', 2).arg(usesTiles ? "tiles/s" : "items/s"));
    };

    auto makeUpgradeEntry = [this, &localizedRate](int optionId,
                                                   int currentTier,
                                                   const QString &iconPath,
                                                   const QString &zhName,
                                                   const QString &enName,
                                                   const QString &zhDescription,
                                                   const QString &enDescription,
                                                   const int *intervals,
                                                   bool usesTiles) {
        UpgradeDialogEntry entry;
        entry.optionId = optionId;
        entry.currentTier = currentTier;
        entry.iconPath = iconPath;
        entry.displayName = t(zhName, enName);
        entry.description = t(zhDescription, enDescription);

        const double currentRate = itemsPerSecondForInterval(intervals[qBound(0, currentTier, kMaxUpgradeTier)]);
        const QString currentRateText = localizedRate(currentRate, usesTiles);
        if (currentTier < kMaxUpgradeTier) {
            const double nextRate = itemsPerSecondForInterval(intervals[qBound(0, currentTier + 1, kMaxUpgradeTier)]);
            const QString nextRateText = localizedRate(nextRate, usesTiles);
            entry.effectTooltip = t(
                QString("%1 Lv.%2 -> Lv.%3\n效率：%4 -> %5")
                    .arg(zhName)
                    .arg(currentTier)
                    .arg(currentTier + 1)
                    .arg(currentRateText)
                    .arg(nextRateText),
                QString("%1 Lv.%2 -> Lv.%3\nThroughput: %4 -> %5")
                    .arg(enName)
                    .arg(currentTier)
                    .arg(currentTier + 1)
                    .arg(currentRateText)
                    .arg(nextRateText));
        } else {
            entry.effectTooltip = t(
                QString("%1 已达到最高等级。\n当前效率：%2").arg(zhName, currentRateText),
                QString("%1 is already at max level.\nCurrent throughput: %2").arg(enName, currentRateText));
        }

        return entry;
    };

    const QVector<UpgradeDialogEntry> upgradeEntries = {
        makeUpgradeEntry(1, itemMoveTier, QStringLiteral(":/res/building_icons/belt.png"),
                         QStringLiteral("传送带"), QStringLiteral("Belts"),
                         QStringLiteral("拖动铺设时会自动形成转弯，是整条产线里最快的一层物流。"),
                         QStringLiteral("Automatically forms turns while drag-placing and remains the fastest logistics layer."),
                         kBeltIntervals, true),
        makeUpgradeEntry(2, balancerTier, QStringLiteral(":/res/building_icons/balancer.png"),
                         QStringLiteral("平衡器"), QStringLiteral("Balancers"),
                         QStringLiteral("把两路输入平均分配到两路输出，适合修正吞吐不均的问题。"),
                         QStringLiteral("Splits two input flows evenly across two outputs so you can smooth out throughput."),
                         kBalancerIntervals, false),
        makeUpgradeEntry(3, undergroundTier, QStringLiteral(":/res/building_icons/underground_belt.png"),
                         QStringLiteral("地下传送带"), QStringLiteral("Underground Belts"),
                         QStringLiteral("让物品跨过拥挤区域，最远可以地下穿行 4 格。"),
                         QStringLiteral("Sends items underneath crowded lines and can bridge up to 4 tiles."),
                         kUndergroundIntervals, false),
        makeUpgradeEntry(4, minerTier, QStringLiteral(":/res/building_icons/miner.png"),
                         QStringLiteral("开采器"), QStringLiteral("Miners"),
                         QStringLiteral("持续开采矿物和染料矿，并把产物朝前方送出。"),
                         QStringLiteral("Continuously extracts both shape ore and dye ore, then outputs them forward."),
                         kMinerIntervals, false),
        makeUpgradeEntry(5, cutterTier, QStringLiteral(":/res/building_icons/cutter.png"),
                         QStringLiteral("切割机"), QStringLiteral("Cutters"),
                         QStringLiteral("把输入图形切成有效半边，为后面的拼装做准备。"),
                         QStringLiteral("Splits incoming shapes into valid halves for later assembly."),
                         kCutterIntervals, false),
        makeUpgradeEntry(6, rotaterTier, QStringLiteral(":/res/building_icons/rotater.png"),
                         QStringLiteral("旋转器"), QStringLiteral("Rotators"),
                         QStringLiteral("把输入图形顺时针旋转 90 度，用来对齐后续结构。"),
                         QStringLiteral("Rotates incoming shapes 90 degrees clockwise to line up later steps."),
                         kRotaterIntervals, false),
        makeUpgradeEntry(7, stackerTier, QStringLiteral(":/res/building_icons/stacker.png"),
                         QStringLiteral("合成器"), QStringLiteral("Stackers"),
                         QStringLiteral("把两路不重叠的图形合成为一个更复杂的目标。"),
                         QStringLiteral("Combines two non-overlapping shapes into one more complex output."),
                         kStackerIntervals, false),
        makeUpgradeEntry(8, mixerTier, QStringLiteral(":/res/building_icons/mixer.png"),
                         QStringLiteral("混合器"), QStringLiteral("Mixers"),
                         QStringLiteral("把两路染料混成二次色，是彩色目标的上游核心环节。"),
                         QStringLiteral("Mixes two dye inputs into a secondary color for late-game colored goals."),
                         kMixerIntervals, false),
        makeUpgradeEntry(9, painterTier, QStringLiteral(":/res/building_icons/painter.png"),
                         QStringLiteral("染色器"), QStringLiteral("Painters"),
                         QStringLiteral("用输入染料给整块图形上色，直接决定 Hub 是否验收。"),
                         QStringLiteral("Applies dye to the whole shape and directly affects Hub validation."),
                         kPainterIntervals, false)
    };

    UpgradeDialog dialog(languageCode, upgradeEntries, pendingUpgradePoints, this);
    dialog.exec();

    bool spentPoints = false;
    for (int optionId : dialog.getSelectedOptions()) {
        if (pendingUpgradePoints > 0 && applyUpgradeOption(optionId)) {
            pendingUpgradePoints--;
            spentPoints = true;
        }
    }

    if (spentPoints) {
        applyTimerTiers();
    } else {
        updateTexts();
    }
    for (QTimer *timer : simulationTimers) {
        timer->start(timer->interval());
    }
    refreshProgressLabels();
}

void Gamescene::skipCurrentLevelForDebug()
{
    map->current = map->target;
    refreshProgressLabels();
    tryAdvanceLevel();
    setFocus();
}

void Gamescene::setPuzzle(){
    map->current = 0;
    map->questionLabel->show();
    map->countLabel->show();
    map->levelLabel->show();
    map->target = 20;
    currentGoalParts = generatedGoalForLevel(map->questionLever);
    currentGoalColors = generatedGoalColorsForLevel(map->questionLever, currentGoalParts);
    map->setGoalShape(currentGoalParts, currentGoalColors);

    const int puzzlePixmapSize = 2 * map->tilePixelSize();
    QPixmap goalPixmap = Item().drawPixmap(
        currentGoalParts[0],
        currentGoalParts[1],
        currentGoalParts[2],
        currentGoalParts[3],
        puzzlePixmapSize,
        currentGoalColors);
    map->questionLabel->setPixmap(goalPixmap);
    refreshProgressLabels();
}

bool Gamescene::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == map) {
        switch (event->type()) {
        case QEvent::MouseButtonPress:
            return handleMapMousePress(static_cast<QMouseEvent *>(event));
        case QEvent::MouseMove:
            return handleMapMouseMove(static_cast<QMouseEvent *>(event));
        case QEvent::MouseButtonRelease:
            return handleMapMouseRelease(static_cast<QMouseEvent *>(event));
        case QEvent::Wheel: {
            auto *wheelEvent = static_cast<QWheelEvent *>(event);
            zoomMapAt(wheelEvent->position().toPoint(), wheelEvent->angleDelta().y());
            return true;
        }
        default:
            break;
        }
    }

    if (auto *button = qobject_cast<QPushButton *>(watched)) {
        if (event->type() == QEvent::ToolTip && !button->toolTip().isEmpty()) {
            auto *helpEvent = static_cast<QHelpEvent *>(event);
            QToolTip::showText(helpEvent->globalPos(), button->toolTip(), button);
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void Gamescene::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);
}

void Gamescene::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);
}

void Gamescene::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);
}

void Gamescene::keyPressEvent(QKeyEvent *event){
    if (isPlaceItem && currentTile) {
        if (event->key() == Qt::Key_R) {
            qDebug() << "press R";
            currentTile->changeDirection();
            rememberDirectionForCurrentTile();
            refreshPlacementPreview();
            // Update underground belt preview after rotation
            if (currentTile->name == "underground_belt_entry" || currentTile->name == "underground_belt_exit") {
                const QPoint gridCell = map->gridPositionFromPoint(map->mapFromGlobal(QCursor::pos()));
                updateUndergroundBeltPreview(gridCell);
            }
        }
        if (event->key() == Qt::Key_T) {
            qDebug() << "press T";
            currentTile->changeState();
            refreshPlacementPreview();
        }
        if (event->key() == Qt::Key_Escape) {
            clearPlacementSelection();
        }
    }
    if (event->key() == Qt::Key_1) {
        beltbtn->click();
    }
    if (event->key() == Qt::Key_2) {
        balancerbtn->click();
    }
    if (event->key() == Qt::Key_3) {
        underground_beltbtn->click();
    }
    if (event->key() == Qt::Key_4) {
        minerbtn->click();
    }
    if (event->key() == Qt::Key_5) {
        cutterbtn->click();
    }
    if (event->key() == Qt::Key_6) {
        rotaterbtn->click();
    }
    if (event->key() == Qt::Key_7) {
        stackerbtn->click();
    }
    if (event->key() == Qt::Key_8) {
        mixerbtn->click();
    }
    if (event->key() == Qt::Key_9) {
        painterbtn->click();
    }
    if (event->key() == Qt::Key_0) {
        trashbtn->click();
    }
#ifdef SHAPEZ_ENABLE_DEBUG_CHEATS
    if (event->key() == Qt::Key_F10) {
        skipCurrentLevelForDebug();
    }
#endif
}

void Gamescene::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateInterfaceLayout();
}

void Gamescene::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    QColor backgroundColor("#ECEEF2");
    painter.fillRect(rect(), backgroundColor);//设置背景色

    QColor rectColor(121, 122, 128, 60);
    painter.setBrush(rectColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(QRect((width() - 700) / 2, height() - 90, 700, 70), 8, 8);
}

void Gamescene::saveGame(const QString& filename) {
    QDir dir;
    QString savePath = dir.currentPath() + "/saves/" + filename;

    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open save file for writing:" << filename;
        return;
    }

    QJsonObject gameState;
    gameState["questionLever"] = map->questionLever;
    gameState["current"] = map->current;
    gameState["target"] = map->target;

    QJsonObject upgrades;
    upgrades["itemMoveTier"] = itemMoveTier;
    upgrades["balancerTier"] = balancerTier;
    upgrades["undergroundTier"] = undergroundTier;
    upgrades["minerTier"]    = minerTier;
    upgrades["cutterTier"]   = cutterTier;
    upgrades["rotaterTier"]  = rotaterTier;
    upgrades["stackerTier"]  = stackerTier;
    upgrades["mixerTier"]    = mixerTier;
    upgrades["painterTier"]  = painterTier;
    upgrades["pendingUpgradePoints"] = pendingUpgradePoints;
    gameState["upgrades"] = upgrades;

    QJsonObject timers;
    timers["itemMoveInterval"] = itemMoveTimer->interval();
    timers["balancerInterval"] = balancerTimer->interval();
    timers["undergroundInterval"] = undergroundTimer->interval();
    timers["minerInterval"]    = minerTimer->interval();
    timers["cutterInterval"]   = cutterTimer->interval();
    timers["rotaterInterval"]  = rotaterTimer->interval();
    timers["stackerInterval"]  = stackerTimer->interval();
    timers["mixerInterval"]    = mixerTimer->interval();
    timers["painterInterval"]  = painterTimer->interval();
    gameState["timers"] = timers;

    QJsonArray mapTiles;
    for (int x = 0; x < map->getheight(); ++x) {
        for (int y = 0; y < map->getwidth(); ++y) {
            Tile *tile = map->tiles[x][y];
            if (tile->type != Tile::Type::Empty && tile->father == nullptr) {
                QJsonObject tileObject;
                tileObject["x"] = x;
                tileObject["y"] = y;
                tileObject["type"] = static_cast<int>(tile->type);
                tileObject["direction"] = tile->direction;
                tileObject["state"] = tile->state;
                tileObject["name"] = tile->name;
                if (tile->name == "miner" && tile->mine != nullptr) {
                    tileObject["mineName"] = tile->mine->name;
                }

                QJsonObject sizeObject;
                sizeObject["first"] = tile->size.first;
                sizeObject["second"] = tile->size.second;
                tileObject["size"] = sizeObject;

                if (tile->item) {
                    tileObject["item"] = serializeItem(tile->item);
                }
                if (tile->secondaryItem) {
                    tileObject["secondaryItem"] = serializeItem(tile->secondaryItem);
                }
                mapTiles.append(tileObject);
            }
        }
    }
    gameState["map"] = mapTiles;

    QJsonDocument doc(gameState);
    file.write(doc.toJson());
    file.close();
    qDebug() << "Game saved to:" << savePath;
}

void Gamescene::loadGame(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open save file for reading:" << filename;
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject gameState = doc.object();

    map->clearMap();

    map->questionLever = gameState["questionLever"].toInt();
    map->target = gameState["target"].toInt();
    setPuzzle();
    map->current = gameState["current"].toInt();

    QJsonObject upgrades = gameState["upgrades"].toObject();
    itemMoveTier = upgrades.contains("itemMoveTier") ? upgrades["itemMoveTier"].toInt() : (upgrades["itemMoveUpgrate"].toBool() ? 1 : 0);
    balancerTier = upgrades.contains("balancerTier") ? upgrades["balancerTier"].toInt() : 0;
    undergroundTier = upgrades.contains("undergroundTier") ? upgrades["undergroundTier"].toInt() : 0;
    minerTier    = upgrades.contains("minerTier")    ? upgrades["minerTier"].toInt()    : (upgrades["minerUpgrate"].toBool()    ? 1 : 0);
    cutterTier   = upgrades.contains("cutterTier")   ? upgrades["cutterTier"].toInt()   : (upgrades["cutterUpgrate"].toBool()   ? 1 : 0);
    rotaterTier  = upgrades.contains("rotaterTier")  ? upgrades["rotaterTier"].toInt()  : 0;
    stackerTier  = upgrades.contains("stackerTier")  ? upgrades["stackerTier"].toInt()  : 0;
    mixerTier    = upgrades.contains("mixerTier")    ? upgrades["mixerTier"].toInt()    : 0;
    painterTier  = upgrades.contains("painterTier")  ? upgrades["painterTier"].toInt()  : 0;
    pendingUpgradePoints = upgrades.contains("pendingUpgradePoints") ? upgrades["pendingUpgradePoints"].toInt() : 0;

    applyTimerTiers();

    QJsonArray mapTiles = gameState["map"].toArray();
    for (const QJsonValue& tileValue : mapTiles) {
        QJsonObject tileObject = tileValue.toObject();
        int x = tileObject["x"].toInt();
        int y = tileObject["y"].toInt();
        Tile::Type type = static_cast<Tile::Type>(tileObject["type"].toInt());
        int direction = tileObject["direction"].toInt();
        QString state = tileObject["state"].toString();
        QString name = tileObject["name"].toString();
        QString mineName = normalizedLegacyDyeName(tileObject["mineName"].toString());
        if (type == Tile::Type::Color) {
            name = normalizedLegacyDyeName(name);
        }

        // 读取 size
        QJsonObject sizeObject = tileObject["size"].toObject();
        std::pair<int, int> size = std::make_pair(sizeObject["first"].toInt(), sizeObject["second"].toInt());

        if (type == Tile::Type::Belt) {
            Tile tile(type, state, direction);
            tile.size = size;  // 恢复 size

            map->setTile(x, y, tile, false);
            if (tileObject.contains("item")) {
                Item *item = deserializeItem(tileObject["item"].toObject());
                map->setItem(std::make_pair(x,y),item);
            }
        } else {
            if(direction==WEST || direction==EAST){
                int x = size.first;
                int y = size.second;
                size = std::make_pair(y,x);
            }
            Tile tile(type, direction, name, size);  // 恢复 size
            if (name == "miner" && !mineName.isEmpty()) {
                tile.mine = new Tile(mineTileTypeForName(mineName), NORTH, mineName);
            }
            if (tileObject.contains("item")) {
                tile.item = deserializeItem(tileObject["item"].toObject());
            }
            if (tileObject.contains("secondaryItem")) {
                tile.secondaryItem = deserializeItem(tileObject["secondaryItem"].toObject());
            }
            map->setTile(x, y, tile, false);
        }
    }

    file.close();
    map->updateLayout();
    refreshProgressLabels();
    qDebug() << "Game loaded from:" << filename;
}

void Gamescene::autoSaveGame(const QString& filename) {
    QDir dir;
    QString savePath = dir.currentPath() + "/auto_save/" + filename;

    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open save file for writing:" << filename;
        return;
    }

    QJsonObject gameState;
    gameState["questionLever"] = map->questionLever;
    gameState["current"] = map->current;
    gameState["target"] = map->target;

    QJsonObject upgrades;
    upgrades["itemMoveTier"] = itemMoveTier;
    upgrades["balancerTier"] = balancerTier;
    upgrades["undergroundTier"] = undergroundTier;
    upgrades["minerTier"]    = minerTier;
    upgrades["cutterTier"]   = cutterTier;
    upgrades["rotaterTier"]  = rotaterTier;
    upgrades["stackerTier"]  = stackerTier;
    upgrades["mixerTier"]    = mixerTier;
    upgrades["painterTier"]  = painterTier;
    upgrades["pendingUpgradePoints"] = pendingUpgradePoints;
    gameState["upgrades"] = upgrades;

    QJsonObject timers;
    timers["itemMoveInterval"] = itemMoveTimer->interval();
    timers["balancerInterval"] = balancerTimer->interval();
    timers["undergroundInterval"] = undergroundTimer->interval();
    timers["minerInterval"]    = minerTimer->interval();
    timers["cutterInterval"]   = cutterTimer->interval();
    timers["rotaterInterval"]  = rotaterTimer->interval();
    timers["stackerInterval"]  = stackerTimer->interval();
    timers["mixerInterval"]    = mixerTimer->interval();
    timers["painterInterval"]  = painterTimer->interval();
    gameState["timers"] = timers;

    QJsonArray mapTiles;
    for (int x = 0; x < map->getheight(); ++x) {
        for (int y = 0; y < map->getwidth(); ++y) {
            Tile *tile = map->tiles[x][y];
            if (tile->type != Tile::Type::Empty && tile->father == nullptr) {
                QJsonObject tileObject;
                tileObject["x"] = x;
                tileObject["y"] = y;
                tileObject["type"] = static_cast<int>(tile->type);
                tileObject["direction"] = tile->direction;
                tileObject["state"] = tile->state;
                tileObject["name"] = tile->name;
                if (tile->name == "miner" && tile->mine != nullptr) {
                    tileObject["mineName"] = tile->mine->name;
                }

                QJsonObject sizeObject;
                sizeObject["first"] = tile->size.first;
                sizeObject["second"] = tile->size.second;
                tileObject["size"] = sizeObject;

                if (tile->item) {
                    tileObject["item"] = serializeItem(tile->item);
                }
                if (tile->secondaryItem) {
                    tileObject["secondaryItem"] = serializeItem(tile->secondaryItem);
                }
                mapTiles.append(tileObject);
            }
        }
    }
    gameState["map"] = mapTiles;

    QJsonDocument doc(gameState);
    file.write(doc.toJson());
    file.close();
    qDebug() << "Game saved to:" << savePath;
}

void Gamescene::autoLoadGame(const QString& filename) {
    {
        QString loadPath = filename;
        if (QDir::isRelativePath(filename) && QFileInfo(filename).path() == ".") {
            loadPath = QDir(QDir::currentPath()).filePath("auto_save/" + filename);
        }

        QFile file(loadPath);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Failed to open save file for reading:" << loadPath;
            return;
        }

        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject gameState = doc.object();

        map->clearMap();

        map->questionLever = gameState["questionLever"].toInt();
        map->target = gameState["target"].toInt();
        setPuzzle();
        map->current = gameState["current"].toInt();

        QJsonObject upgrades = gameState["upgrades"].toObject();
        itemMoveTier = upgrades.contains("itemMoveTier") ? upgrades["itemMoveTier"].toInt() : (upgrades["itemMoveUpgrate"].toBool() ? 1 : 0);
        balancerTier = upgrades.contains("balancerTier") ? upgrades["balancerTier"].toInt() : 0;
        undergroundTier = upgrades.contains("undergroundTier") ? upgrades["undergroundTier"].toInt() : 0;
        minerTier    = upgrades.contains("minerTier")    ? upgrades["minerTier"].toInt()    : (upgrades["minerUpgrate"].toBool()    ? 1 : 0);
        cutterTier   = upgrades.contains("cutterTier")   ? upgrades["cutterTier"].toInt()   : (upgrades["cutterUpgrate"].toBool()   ? 1 : 0);
        rotaterTier  = upgrades.contains("rotaterTier")  ? upgrades["rotaterTier"].toInt()  : 0;
        stackerTier  = upgrades.contains("stackerTier")  ? upgrades["stackerTier"].toInt()  : 0;
        mixerTier    = upgrades.contains("mixerTier")    ? upgrades["mixerTier"].toInt()    : 0;
        painterTier  = upgrades.contains("painterTier")  ? upgrades["painterTier"].toInt()  : 0;
        pendingUpgradePoints = upgrades.contains("pendingUpgradePoints") ? upgrades["pendingUpgradePoints"].toInt() : 0;

        applyTimerTiers();

        QJsonArray mapTiles = gameState["map"].toArray();
        for (const QJsonValue& tileValue : mapTiles) {
            QJsonObject tileObject = tileValue.toObject();
            int x = tileObject["x"].toInt();
            int y = tileObject["y"].toInt();
            Tile::Type type = static_cast<Tile::Type>(tileObject["type"].toInt());
            int direction = tileObject["direction"].toInt();
        QString state = tileObject["state"].toString();
        QString name = tileObject["name"].toString();
        QString mineName = normalizedLegacyDyeName(tileObject["mineName"].toString());
        if (type == Tile::Type::Color) {
            name = normalizedLegacyDyeName(name);
        }

            // 读取 size
            QJsonObject sizeObject = tileObject["size"].toObject();
            std::pair<int, int> size = std::make_pair(sizeObject["first"].toInt(), sizeObject["second"].toInt());

            if (type == Tile::Type::Belt) {
                Tile tile(type, state, direction);
                tile.size = size;  // 恢复 size

                map->setTile(x, y, tile, false);
                if (tileObject.contains("item")) {
                    Item *item = deserializeItem(tileObject["item"].toObject());
                    map->setItem(std::make_pair(x,y),item);
                }
            } else {
                if(direction==WEST || direction==EAST){
                    int x = size.first;
                    int y = size.second;
                    size = std::make_pair(y,x);
                }
                Tile tile(type, direction, name, size);  // 恢复 size
                if (name == "miner" && !mineName.isEmpty()) {
                    tile.mine = new Tile(mineTileTypeForName(mineName), NORTH, mineName);
                }
                if (tileObject.contains("item")) {
                    tile.item = deserializeItem(tileObject["item"].toObject());
                }
                if (tileObject.contains("secondaryItem")) {
                    tile.secondaryItem = deserializeItem(tileObject["secondaryItem"].toObject());
                }
                map->setTile(x, y, tile, false);
            }
        }

        file.close();
        map->updateLayout();
        refreshProgressLabels();
        qDebug() << "Game loaded from:" << loadPath;
    }
}

Gamescene::~Gamescene() {

}

void Gamescene::returnToMainScene() {
    emit returnToMain();
    this->close();
    this->deleteLater();
}

#include "gamescene.moc"
