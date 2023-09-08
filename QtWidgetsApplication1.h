#pragma once

#include <QtWidgets/QWidget>
#include "ui_QtWidgetsApplication1.h"
#include "QtWidgetsClass.h"
#include "QtWidgetsClassCase2.h"
#include "QtWidgetsClassCase3.h"
#include "QtWidgetsClassCase6.h"

class QtWidgetsApplication1 : public QWidget
{
    Q_OBJECT

public:
    QtWidgetsApplication1(QWidget *parent = nullptr);
    ~QtWidgetsApplication1();

private slots:
    void on_pushButton1_clicked();
    void on_pushButton2_clicked();
    void on_pushButton3_clicked();
    void on_pushButton4_clicked();
    void on_pushButton6_clicked();
private:
    Ui::QtWidgetsApplication1Class ui;
    QtWidgetsClass* form;

    Ui::QtWidgetsApplication1Class iu;
    QtWidgetsClassCase3* form1;

    Ui::QtWidgetsApplication1Class Iu;
    QtWidgetsClassCase2* form2;

    Ui::QtWidgetsApplication1Class qt;
    QtWidgetsClassCase6* form3;
};
