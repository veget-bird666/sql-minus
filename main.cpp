#include "widget.h"
#include <QApplication>
Widget* widget;


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.setWindowTitle("sql-minus");
    widget = &w;
    w.show();
    return a.exec();
}
