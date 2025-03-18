#include "widget.h"
#include <QApplication>
Widget* widget;


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    widget = &w;
    w.show();
    return a.exec();
}
