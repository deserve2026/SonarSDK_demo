#include "mainwindow.h"
#include "LhForwardFrame.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<packageInfo>("packageInfo");

#ifdef USE_WINDOWS
    QFont font("Arial", 13);
#elif USE_LINUX
    QFont font("Arial", 13);
#endif

    a.setFont(font);

    /* PACK_SIZE defaults to 5800. If you need other sizes, modify this one.
    If you don't need to modify the size, you can annotate it like this */
    // LhForwardSDK::LhForwardFrame::PACK_SIZE = 5800;

    MainWindow w;
    w.show();
    return a.exec();
}
