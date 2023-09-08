#pragma once

#include <QMainWindow>
#include <QProgressBar>
#include "ui_QtWidgetsClass.h"

class QtWidgetsClass : public QMainWindow
{
	Q_OBJECT

public:
	QtWidgetsClass(QWidget *parent = nullptr);
	~QtWidgetsClass();
	void updateProgress(int value);
	void ProgramText(QString text);

private slots:
	void on_pushButton1_clicked();
	void on_pushButton_clicked();

private:
	Ui::QtWidgetsClassClass ui;
};
