/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
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
    QTextEdit *informationEdit;
    QPushButton *executeButton;
    QPushButton *clearButton;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName("Widget");
        Widget->resize(611, 587);
        Widget->setStyleSheet(QString::fromUtf8("background-color:rgb(43, 45, 48);"));
        commandEdit = new QTextEdit(Widget);
        commandEdit->setObjectName("commandEdit");
        commandEdit->setGeometry(QRect(50, 50, 511, 141));
        QFont font;
        font.setPointSize(11);
        commandEdit->setFont(font);
        commandEdit->setStyleSheet(QString::fromUtf8("color:white;\n"
"background-color:rgb(30, 31, 34);"));
        informationEdit = new QTextEdit(Widget);
        informationEdit->setObjectName("informationEdit");
        informationEdit->setGeometry(QRect(50, 230, 511, 231));
        informationEdit->setFont(font);
        informationEdit->setStyleSheet(QString::fromUtf8("color:white;\n"
"background-color:rgb(30, 31, 34);"));
        executeButton = new QPushButton(Widget);
        executeButton->setObjectName("executeButton");
        executeButton->setGeometry(QRect(50, 500, 161, 61));
        executeButton->setFont(font);
        executeButton->setStyleSheet(QString::fromUtf8("color:white;\n"
"background-color:rgb(30, 31, 34);"));
        clearButton = new QPushButton(Widget);
        clearButton->setObjectName("clearButton");
        clearButton->setGeometry(QRect(400, 500, 161, 61));
        clearButton->setFont(font);
        clearButton->setStyleSheet(QString::fromUtf8("color:white;\n"
"background-color:rgb(30, 31, 34);"));

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
