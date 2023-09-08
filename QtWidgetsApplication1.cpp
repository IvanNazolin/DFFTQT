#include "QtWidgetsApplication1.h"
#include "qfiledialog.h"


QtWidgetsApplication1::QtWidgetsApplication1(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

QtWidgetsApplication1::~QtWidgetsApplication1()
{}

void QtWidgetsApplication1::on_pushButton1_clicked()
{


}

void QtWidgetsApplication1::on_pushButton2_clicked()
{
    form2 = new QtWidgetsClassCase2;
    form2->show();
}

void QtWidgetsApplication1::on_pushButton3_clicked()
{
    form1 = new QtWidgetsClassCase3;
    form1->show();
}

void QtWidgetsApplication1::on_pushButton4_clicked()
{
    form = new QtWidgetsClass;
    form->show();
}

void QtWidgetsApplication1::on_pushButton6_clicked()
{
    form3 = new QtWidgetsClassCase6;
    form3->show();

}