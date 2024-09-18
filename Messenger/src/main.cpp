#include "../gui/Messenger.h"
#include <QtWidgets/QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Messenger w;
    w.show();
    return a.exec();
}