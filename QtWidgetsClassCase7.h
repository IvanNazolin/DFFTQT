#pragma once

#include <QMainWindow>
#include "ui_QtWidgetsClassCase7.h"

class QtWidgetsClassCase7 : public QMainWindow
{
	Q_OBJECT

public:
	QtWidgetsClassCase7(QWidget *parent = nullptr);
	~QtWidgetsClassCase7();

private:
	Ui::QtWidgetsClassCase7Class ui;
};
