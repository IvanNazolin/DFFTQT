#include "QtWidgetsClassCase3.h"
#include "qfiledialog.h"
#include "qstring.h"
#include <QProgressBar>
#include "../dfft.h"

bool paramerChecked1 = false;
QString Directory1;

QtWidgetsClassCase3::QtWidgetsClassCase3(QWidget *parent)
	: QMainWindow(parent)
{
	iu.setupUi(this);
}

QtWidgetsClassCase3::~QtWidgetsClassCase3()
{}

void QtWidgetsClassCase3::ProgramText(QString text)
{
	iu.textBrowser->append(text);
}

void QtWidgetsClassCase3::on_pushButton_clicked()
{
	if (paramerChecked1 == false)
	{
		iu.textEdit->setText("16000");
		//iu.textEdit_2->setText("10");
		iu.textEdit_3->setText("16384");
		iu.textEdit_4->setText("1");
		iu.checkBox_3->setChecked(true);
		paramerChecked1 = true;
	}
	else
	{
		iu.textEdit->setText("");
		//iu.textEdit_2->setText("");
		iu.textEdit_3->setText("");
		iu.textEdit_4->setText("");
		iu.checkBox_3->setChecked(false);
		iu.checkBox_2->setChecked(false);
		iu.checkBox_4->setChecked(false);
		paramerChecked1 = false;
	}
}

void QtWidgetsClassCase3::updateProgress3(int value)
{
	float prog = 100 / value;
	iu.progressBar->setRange(0, 100);
	iu.progressBar->setValue(value);
}

void QtWidgetsClassCase3::on_pushButton1_clicked() 
{
	int regim;
	if (iu.checkBox_2->isChecked())
	{
		regim = 1;
	}
	if (iu.checkBox_3->isChecked())
	{
		regim = 2;
	}
	if (iu.checkBox_4->isChecked())
	{
		regim = 3;
	}
	float widthL = (iu.textEdit_4->toPlainText()).toFloat();
	int samplerate = (iu.textEdit->toPlainText()).toInt();
	//int interval = (iu.textEdit_2->toPlainText()).toInt();
	int BPF = (iu.textEdit_3->toPlainText()).toInt();
	Directory1 = QFileDialog::getExistingDirectory(this, tr("Directory"),
		"",
		QFileDialog::DontResolveSymlinks | QFileDialog::ReadOnly);
	iu.label->setText(Directory1);
	std::wstring path = Directory1.toStdWString();
	int progress = 0;
	this->updateProgress3(1);
	case3(path, this, samplerate, BPF, regim, widthL);
}