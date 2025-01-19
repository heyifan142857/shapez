#include "mainscene.h"

#include <QApplication>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QDir dir;
    qDebug() << "当前工作目录:" << QDir::currentPath();
    if (!dir.exists("saves")) {
        if (dir.mkdir("saves")) {
            qDebug() << "文件夹创建成功:" << "saves";
        } else {
            qWarning() << "无法创建文件夹:" << "saves";
        }
    }
    if (!dir.exists("auto_save")) {
        if (dir.mkdir("auto_save")) {
            qDebug() << "文件夹创建成功:" << "auto_save";
        } else {
            qWarning() << "无法创建文件夹:" << "auto_save";
        }
    }
    Mainscene w;
    w.show();
    qDebug() << "当前在主界面";
    return a.exec();
}
