#pragma once

#include <QMainWindow>
#include "ui_QtWidgetsClassCase6.h"

class QtWidgetsClassCase6 : public QMainWindow
{
	Q_OBJECT

public:
    void updateProgress6(int value);
	QtWidgetsClassCase6(QWidget *parent = nullptr);
	~QtWidgetsClassCase6();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_4_clicked();
    void on_btn2_clicked();
    void on_pushButton_9_clicked();
    void MakeFX();
    void check();

private:
	Ui::QtWidgetsClassCase6Class qt;
};
