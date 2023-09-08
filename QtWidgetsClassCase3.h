#pragma once

#include <QMainWindow>
#include "qprogressbar.h"
#include "ui_QtWidgetsClassCase3.h"

class QtWidgetsClassCase3 : public QMainWindow
{
	Q_OBJECT

public:
	QtWidgetsClassCase3(QWidget *parent = nullptr);
	~QtWidgetsClassCase3();
	void updateProgress3(int value);
	void ProgramText(QString text);

private slots:
	void on_pushButton1_clicked();
	void on_pushButton_clicked();

private:
	Ui::QtWidgetsClassCase3Class iu;
};
