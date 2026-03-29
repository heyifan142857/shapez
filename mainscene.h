#ifndef MAINSCENE_H
#define MAINSCENE_H

#include <QWidget>
#include <QIcon>
#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QPropertyAnimation>
#include <QTimer>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QFileDialog>
#include <QResizeEvent>
#include "gamescene.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainScene;
}
QT_END_NAMESPACE

class Mainscene : public QWidget
{
    Q_OBJECT

public:
    Mainscene(QWidget *parent = nullptr);
    ~Mainscene();

    void loadGameAndSwitchToGameScene(const QString& filename);

    void loadGameAndSwitchToGameScene();

    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *event) override;

    Gamescene * _gamescene = NULL;


private:
    void applyLanguage();
    void toggleLanguage();
    QString t(const QString &zhText, const QString &enText) const;
    void updateLanguageButtonIcon();
    void updateInterfaceLayout();

    QMediaPlayer *player;
    QAudioOutput *audioOutput;
    Gamescene *gamescene = nullptr;
    QPushButton *newbtn = nullptr;
    QPushButton *readbtn = nullptr;
    QPushButton *continuebtn = nullptr;
    QPushButton *languagebtn = nullptr;
    QPushButton *steambtn = nullptr;
    QPushButton *githubbtn = nullptr;
    QString languageCode = "zh-CN";
private:
    Ui::MainScene *ui;
};
#endif // MAINSCENE_H
