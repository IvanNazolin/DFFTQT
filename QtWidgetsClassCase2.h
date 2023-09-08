#pragma once

#include <QMainWindow>
#include "ui_QtWidgetsClassCase2.h"

class QtWidgetsClassCase2 : public QMainWindow
{
	Q_OBJECT

public:
	QtWidgetsClassCase2(QWidget *parent = nullptr);
	~QtWidgetsClassCase2();
private slots:
	void on_pushButton1_clicked();

private:
	Ui::QtWidgetsClassCase2Class ui;
};
