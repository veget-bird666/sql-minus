#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "server.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void showMessage(QString message);

    void executeSqlStatement(const QString& sql);  // 执行单条SQL语句

private slots:
    void on_executeButton_clicked();

    void on_clearButton_clicked();

    void on_analysisButton_clicked();

private:
    Ui::Widget *ui;
    Server *server;
};
#endif // WIDGET_H
