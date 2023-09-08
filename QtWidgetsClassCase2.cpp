#include "QtWidgetsClassCase2.h"
#include "qfiledialog.h"
#include "qstring.h"
#include "../dfft.h"

QtWidgetsClassCase2::QtWidgetsClassCase2(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
}

QtWidgetsClassCase2::~QtWidgetsClassCase2()
{}

void QtWidgetsClassCase2::on_pushButton1_clicked()
{
	int workRegim;
	std::string modelwork;
	if (ui.checkBox->isChecked())
	{
		workRegim = 1;
	}
	if (ui.checkBox_2->isChecked())
	{
		workRegim = 2;
	}
	if (ui.checkBox_3->isChecked())
	{
		workRegim = 3;
	}
	QString comboRegim = ui.comboBox->currentText();
	modelwork = comboRegim.toStdString();
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open dat File"),
		"",
		tr("(*.dat)"));
	int e = fileName.lastIndexOf("/");
	e = e + 1;
	QString NameFile = fileName.sliced(e, fileName.length()-e);
	ui.label->setText(NameFile);
	case2(fileName.toStdWString(), workRegim, modelwork);

}
