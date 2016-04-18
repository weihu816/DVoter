#include <QApplication>
#include "MainWindow.h"
#include <QThread>
#include <qtconcurrentrun.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}

