#include "mainscene.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Mainscene w;
    w.show();
    qDebug() << "当前在主界面";
    return a.exec();
}
