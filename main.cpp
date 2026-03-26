#include "mainscene.h"

#include <QApplication>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QDir dir;
    qDebug() << "Current working directory:" << QDir::currentPath();
    if (!dir.exists("saves")) {
        if (dir.mkdir("saves")) {
            qDebug() << "Created folder:" << "saves";
        } else {
            qWarning() << "Failed to create folder:" << "saves";
        }
    }
    if (!dir.exists("auto_save")) {
        if (dir.mkdir("auto_save")) {
            qDebug() << "Created folder:" << "auto_save";
        } else {
            qWarning() << "Failed to create folder:" << "auto_save";
        }
    }
    Mainscene w;
    w.show();
    qDebug() << "Main menu is active";
    return a.exec();
}
