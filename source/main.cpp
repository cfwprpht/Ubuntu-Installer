#include "../include/ubuntuinstaller.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    UbuntuInstaller w;
    w.show();

    return a.exec();
}
