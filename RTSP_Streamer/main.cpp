#include "mainwindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CMainWindow mainWindow;
    int ret = -1;
    if (mainWindow.initialize() == true)
    {
        mainWindow.show();
        ret = a.exec();
    }
    return ret;
}
