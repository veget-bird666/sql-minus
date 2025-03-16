/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QTextEdit *commandEdit;
    QPushButton *executeButton;
    QPushButton *clearButton;
    QTextEdit *informationEdit;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName("Widget");
        Widget->resize(800, 600);
        commandEdit = new QTextEdit(Widget);
        commandEdit->setObjectName("commandEdit");
        commandEdit->setGeometry(QRect(130, 70, 531, 351));
        QFont font;
        font.setPointSize(12);
        commandEdit->setFont(font);
        executeButton = new QPushButton(Widget);
        executeButton->setObjectName("executeButton");
        executeButton->setGeometry(QRect(130, 520, 151, 51));
        QFont font1;
        font1.setPointSize(11);
        executeButton->setFont(font1);
        clearButton = new QPushButton(Widget);
        clearButton->setObjectName("clearButton");
        clearButton->setGeometry(QRect(500, 520, 161, 51));
        clearButton->setFont(font1);
        informationEdit = new QTextEdit(Widget);
        informationEdit->setObjectName("informationEdit");
        informationEdit->setGeometry(QRect(130, 440, 531, 66));

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "Widget", nullptr));
        executeButton->setText(QCoreApplication::translate("Widget", "\346\211\247\350\241\214", nullptr));
        clearButton->setText(QCoreApplication::translate("Widget", "\346\270\205\351\231\244", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
