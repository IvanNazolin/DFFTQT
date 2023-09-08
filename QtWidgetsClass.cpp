#include "QtWidgetsClass.h"
#include "qfiledialog.h"
#include "../dfft.h"
#include "qstring.h"
#include <QProgressBar>

bool paramerChecked = false;
QString Directory;
QtWidgetsClass::QtWidgetsClass(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
}

QtWidgetsClass::~QtWidgetsClass()
{}

void QtWidgetsClass::ProgramText(QString text) 
{
	ui.textBrowser->append(text);
}

void QtWidgetsClass::updateProgress(int value)
{
	//float prog = 100 / value;
	ui.progressBar->setRange(0, 100);
	ui.progressBar->setValue(value);
}

void QtWidgetsClass::on_pushButton_clicked()
{
	if (paramerChecked==false)
	{
		ui.textEdit->setText("32000");
		ui.textEdit_2->setText("10");
		ui.textEdit_3->setText("262144");
		ui.textEdit_4->setText("0.5");
		ui.textEdit_5->setText("1");
		ui.checkBox_3->setChecked(true);
		paramerChecked = true;
	}
	else
	{
		ui.textEdit->setText("");
		ui.textEdit_2->setText("");
		ui.textEdit_3->setText("");
		ui.textEdit_4->setText("");
		ui.textEdit_5->setText("");
		ui.checkBox_3->setChecked(false);
		ui.checkBox_2->setChecked(false);
		ui.checkBox_4->setChecked(false);
		paramerChecked=false;
	}
}

void QtWidgetsClass::on_pushButton1_clicked()
{
	int regim;
	if (ui.checkBox_2->isChecked())
	{
		regim = 1;
	}
	if (ui.checkBox_3->isChecked())
	{
		regim = 2;
	}
	if (ui.checkBox_4->isChecked())
	{
		regim = 3;
	}
	int crBlock = (ui.textEdit_5->toPlainText()).toInt();
	float widthL = (ui.textEdit_4->toPlainText()).toFloat();
	int samplerate = (ui.textEdit->toPlainText()).toInt();
	int interval = (ui.textEdit_2->toPlainText()).toInt();
	int BPF = (ui.textEdit_3->toPlainText()).toInt();
	Directory = QFileDialog::getExistingDirectory(this, tr("Directory"),
		"",
		QFileDialog::DontResolveSymlinks | QFileDialog::ReadOnly);
	ui.label->setText(Directory);
	std::wstring path = Directory.toStdWString();
	int progress = 0;
	this->updateProgress(10);
	case4(path, this, samplerate, interval, BPF,regim, widthL, crBlock);
};


