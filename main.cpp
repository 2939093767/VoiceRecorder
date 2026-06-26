#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(QStringLiteral("语音记录"));
    a.setApplicationVersion(QStringLiteral("1.0.0"));
    MainWindow w;
    w.show();
    return a.exec();
}
