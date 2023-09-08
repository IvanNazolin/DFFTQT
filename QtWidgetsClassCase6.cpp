#include "QtWidgetsClassCase6.h"
#include<windows.h>
//#include <iostream>
#include <stdio.h>
#include <dos.h>
#include <stdlib.h>
#include <conio.h>
//#include <string.h>
#include <math.h>
#include <locale.h>
#include <string>
#include <cmath>
#include "QFileDialog"
#include <filesystem>
#include <fstream>
#include <direct.h>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
//#include "main.cpp"

#include <QApplication>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>
#include "qstring.h"
#include "../dfft.h"

using namespace  std;
namespace fs = std::filesystem;

#define TO_STRING_AUX( x )  #x
#define TO_STRING( x )      TO_STRING_AUX( x )

QString strx;
QString strf;
QString floadf;
string text;
QString  startedPath;
int chek = 1;
int chek1 = 1;
int last = 0;
int allowance = 0;

void get_path(const std::wstring& filePath, std::wstring& fileName) {
    int len = filePath.length(), k = 0;
    int pos = -1;

    for (int j = len - 1; j >= 0; j--) {
        if (filePath[j] == '/' || filePath[j] == '\\') {
            pos = j;
            break;
        }
    }

    if (pos == -1) {
        qInfo() << "Crash; pos = -1; slash not found! Not secure";
        qApp->exit(1);
    }


    for (int j = pos + 1; j < len; j++) {
        fileName += (filePath[j]);
    }

}

QtWidgetsClassCase6::QtWidgetsClassCase6(QWidget* parent)
    : QMainWindow(parent)
{
    qt.setupUi(this);
}

void QtWidgetsClassCase6::updateProgress6(int value)
{
    float prog = 100 / value;
    qt.progressBar->setRange(0, 100);
    qt.progressBar->setValue(value);
}

void get_filesCase6(vector<wstring>& paths, const wstring& current_path)
{
    for (const auto& file : filesystem::directory_iterator(current_path))
    {
        if (filesystem::is_directory(file))
        {
            get_filesCase6(paths, file.path());
        }
        else
        {
            paths.push_back(file.path());
        }
    }
}

void QtWidgetsClassCase6::on_pushButton_clicked()
{
    if (chek1 == 1)
    {
        qt.Edit1Fmax1->setPlainText("-200");
        qt.Edit1Fmax2->setPlainText("-500");

        qt.Edit2Fmax1->setPlainText("-500");
        qt.Edit2Fmax2->setPlainText("-1000");

        qt.Edit3Fmax1->setPlainText("-1000");
        qt.Edit3Fmax2->setPlainText("-1500");

        qt.Edit4Fmax1->setPlainText("-2000");
        qt.Edit4Fmax2->setPlainText("-3000");

        qt.Edit5Fmax1->setPlainText("-3000");
        qt.Edit5Fmax2->setPlainText("-4000");

        qt.Edit6Fmax1->setPlainText("-4000");
        qt.Edit6Fmax2->setPlainText("-5000");
        chek1 = 0;
    }
    else
    {
        qt.Edit1Fmax1->setPlainText("");
        qt.Edit1Fmax2->setPlainText("");

        qt.Edit2Fmax1->setPlainText("");
        qt.Edit2Fmax2->setPlainText("");

        qt.Edit3Fmax1->setPlainText("");
        qt.Edit3Fmax2->setPlainText("");

        qt.Edit4Fmax1->setPlainText("");
        qt.Edit4Fmax2->setPlainText("");

        qt.Edit5Fmax1->setPlainText("");
        qt.Edit5Fmax2->setPlainText("");

        qt.Edit6Fmax1->setPlainText("");
        qt.Edit6Fmax2->setPlainText("");
        chek1 = 1;
    }
}

std::wstring getFilePathWithoutFileNameAndExtension(const std::wstring& path)
{
    size_t lastSlashIndex = -1;
    size_t lastDotIndex = -1;
    int len = path.length(), k = 0;

    for (int j = len - 1; j >= 0; j--) {
        if (path[j] == '/' || path[j] == '\\') {
            lastSlashIndex = j;
            break;
        }
    }

    for (int j = len - 1; j >= 0; j--) {
        if (path[j] == '.') {
            lastDotIndex = j;
            break;
        }
    }

    if (lastDotIndex == -1 || lastSlashIndex == -1) {
        qInfo() << "Crash; pos = -1; slash not found! Not secure";
        qApp->exit(1);
    }

    if (lastSlashIndex != std::wstring::npos && lastDotIndex != std::wstring::npos && lastDotIndex > lastSlashIndex) {
        return path.substr(0, lastSlashIndex);
    }
    else {
        return L"";
    }
}

void QtWidgetsClassCase6::check()
{
    qt.markerOfWork->setText(QString("Cheking"));
    wstring star_path = startedPath.toStdWString();
    vector<wstring>startPath;
    get_filesCase6(startPath, star_path);
    for (long cpunter = 0; cpunter < startPath.size(); cpunter++)
    {
        if (startPath[cpunter].substr(startPath[cpunter].length() - 5, startPath[cpunter].length()) == L"x.dat" || startPath[cpunter].substr(startPath[cpunter].length() - 5, startPath[cpunter].length()) == L"f.dat")
        {
            QDialog errorF;
            wstring errorText = L"OSHIBKA";
            errorF.setWindowTitle(QString::fromStdWString(errorText));
            QVBoxLayout layout(&errorF);
            QLabel labelF("В папке уже были произведены расчеты");
            QLabel labelP(QString::fromStdWString(startPath[cpunter]));
            layout.addWidget(&labelF);
            layout.addWidget(&labelP);
            // Установка вертикального слоя в диалоговое окно
            errorF.setLayout(&layout);

            // Отображаем диалоговое окно
            errorF.exec();
            allowance = 1;
            qt.markerOfWork->setText("Проверка провалена");

        }
    }
}

void QtWidgetsClassCase6::MakeFX()
{
    check();
    string line;
    QString path_to_dat[8];
    string mas[150];
    string mask1k7[1500];
    double fmask0txo[8];
    double fmask1txo[8];
    float fmask1[8];
    float fmask0[8];

    //получение всех файлов в стартовой директории.
    wstring star_path = startedPath.toStdWString();
    vector<wstring>startPath;
    string path_to_M[1];
    get_filesCase6(startPath, star_path);
    //qt.markerOfWork->setText("Функция запущена");
    float r = startPath.size();
    float persent = r / 100;
    if (allowance == 0)
    {
        //qt.label_13->setText(QString(TO_STRING("Выполняется")));
        for(long cpunter = 0; cpunter < startPath.size(); cpunter++){
            //Ui::Widget *ui;

            QString path_to_txo[8] = { "" };
            wstring path_mera;
            if (startPath[cpunter].substr(startPath[cpunter].length() - 4, startPath[cpunter].length()) == L"mera" || startPath[cpunter].substr(startPath[cpunter].length() - 4, startPath[cpunter].length()) == L"MERA" || startPath[cpunter].substr(startPath[cpunter].length() - 4, startPath[cpunter].length()) == L"Mera")
            {
                path_mera = startPath[cpunter];
                vector<wstring>paths;
                wstring path_to_dir = getFilePathWithoutFileNameAndExtension(path_mera);
                ifstream mera(path_mera.c_str());
                for (int counteri = 0; !mera.eof(); counteri++)
                {
                    getline(mera, mas[counteri]);
                }
                mera.close();
                //сортировка исходного массива для разделения его на два массив mask1 и mask0 и параллельное преобразование данных в тип float;
                for (int ik1 = 0, ik0 = 0, i = 0; i < 150; i++)
                {
                    if (mas[i].substr(0, 2) == "k0") //перевод элемента в mask0
                    {
                        string s = mas[i].substr(3, mas[i].length());
                        fmask0[ik0] = stof(s);
                        ik0 = ik0 + 1;
                    }

                    if (mas[i].substr(0, 2) == "k1") //перевод элемента в mask1
                    {
                        string s = mas[i].substr(3, mas[i].length());
                        fmask1[ik1] = stof(s);
                        ik1 = ik1 + 1;
                    }
                }

                get_filesCase6(paths, path_to_dir);
                for (int i = 0, i1 = 0, i2 = 0; i < paths.size(); i++)
                {
                    if (paths[i].substr(paths[i].length() - 3, paths[i].length()) == L"TX0")
                    {
                        path_to_txo[i2] = QString::fromStdWString(paths[i]);
                        //path_to_new_Txo[i2] = path_to_txo[i2].sliced(0, path_to_txo[i2].length() - 17) +  QString::fromStdString(new_txo[i2]);
                        i2 = i2 + 1;
                    }
                    if (paths[i].substr(paths[i].length() - 3, paths[i].length()) == L"dat")
                    {
                        path_to_dat[i1] = QString::fromStdWString(paths[i]);
                        //  path_to_new_dat[i1] = path_to_dat[i1].sliced(0, path_to_dat[i1].length() - 18) +  QString::fromStdString(new_dat[i1]);
                        i1 += 1;
                    }
                }
               
                QString qw1;
                qw1 = QString::fromStdWString(path_to_dir) + "/" + "Channel_1";
                CreateDirectory(qw1.toStdWString().c_str(), NULL);
                QString qw2 = QString::fromStdWString(path_to_dir) + "/" + "Channel_2";
                CreateDirectory(qw2.toStdWString().c_str(), NULL);
                QString qw3 = QString::fromStdWString(path_to_dir) + "/" + "Channel_3";
                CreateDirectory(qw3.toStdWString().c_str(), NULL);
                QString qw4 = QString::fromStdWString(path_to_dir) + "/" + "Channel_4";
                CreateDirectory(qw4.toStdWString().c_str(), NULL);
                QString qw5 = QString::fromStdWString(path_to_dir) + "/" + "Channel_5";
                CreateDirectory(qw5.toStdWString().c_str(), NULL);
                QString qw6 = QString::fromStdWString(path_to_dir) + "/" + "Channel_6";
                CreateDirectory(qw6.toStdWString().c_str(), NULL);
                QString qw7 = QString::fromStdWString(path_to_dir) + "/" + "Channel_7";
                CreateDirectory(qw7.toStdWString().c_str(), NULL);
                QString path_to_xd[7] = {qw1,qw2,qw3,qw4,qw5,qw6,qw7};

                if (path_to_txo[0] != "")
                {
                    for (int i = 0, c = 0; i < 8; i++)
                    {
                        string s;
                        ifstream filetxo(path_to_txo[i].toStdWString().c_str());
                        getline(filetxo, s);
                        int pointer = s.find(" ");
                        fmask0txo[c] = stod(s.substr(0, pointer));
                        fmask1txo[c] = stod(s.substr(pointer, s.length()));
                        c += 1;
                        filetxo.close();
                    }
                }

                //чтение данных из файлов пути к которым лежат в  path_to_dat;
                for (int i = 0; i < 7; i++)
                {
                    ifstream q(path_to_dat[7].toStdWString().c_str(), ios_base::binary);
                    ifstream f(path_to_dat[i].toStdWString().c_str(), ios_base::binary);
                    ofstream fx((path_to_xd[i].toStdWString() + L"\\x.dat").c_str(), ios_base::binary);
                    ofstream ff((path_to_xd[i].toStdWString() + L"\\f.dat").c_str(), ios_base::binary);
                    while (!f.eof())
                    {
                        short num = 0;
                        f.read((char*)&num, sizeof(short));
                        float x;
                        if (path_to_txo[0] != "")
                        {
                            x = fmask1txo[i] * ((fmask1[i] * (num - fmask0[i])) + fmask0txo[i]);
                            fx.write((char*)&x, sizeof(float));
                        }
                        else {
                            x = fmask1[i] * (num - fmask0[i]);
                            fx.write((char*)&x, sizeof(float));
                        }
                    }
                    while (!q.eof())
                    {
                        short num = 0;
                        q.read((char*)&num, sizeof(short));
                        float y;
                        if (path_to_txo[0] != "")
                        {
                            y = fmask1txo[7] * ((fmask1[7] * (num - fmask0[7])) + fmask0txo[7]);
                            ff.write((char*)&y, sizeof(float));
                        }
                        else
                        {
                            y = fmask1[7] * (num - fmask0[7]);
                            ff.write((char*)&y, sizeof(float));
                        }
                    }
                    q.close();
                    f.close();
                    fx.close();
                    ff.close();
                }
            }
            last += 1;
            updateProgress6(50);
            //qt.markerOfWork->setText(QString::fromStdString(to_string(last)));
        }
    }
}

void QtWidgetsClassCase6::on_pushButton_4_clicked()
{

    MakeFX();


    if (allowance == 0)
    {
        QTextStream cout(stdout);

#define Path 150

        FILE* frd1, * frd2, * fwr1, * fwr2;
        int counterForSizeofFile = 0;
        int i, N, f;
        wchar_t
            fw[Path],		// путь с именем файла записи сигнала ударных импульсов
            xw[Path];		// путь с именем файла записи сигнала реакции

        float x, x_old, mean1, mean2, data, p,
            Fmax[300];		// значения амплитуд ударных импульсов;

        //----------------- Обязательные для задания параметры ---------------------

        long k, j, point,
            koeff = -1.0,		// коэффициент умножения сигнала силы
            zero_point = 100,	// число точек до удара
            tau1 = 2, 			// постоянная времени обработки удара и отклика до удара

            N_bpf = 16384,		// число точек между импульсами
            tau2 = 3200.0,		// постоянная времени затухания для N_bpf = 16384


            //			N_bpf = 65536,
            //			tau2 = 3200 * 4,	// постоянная времени затухания для N_bpf = 65536

            //		 N_bpf = 32768,
            //		 tau2 = 3200*2,		// постоянная времени затухания для N_bpf = 65536

            //		 tau2= 150000.0,	// tau2=max без затухания

            Fmax_1 = -200,   	//-300 -900  -2500 нижний порог отбора амплитуд ударных импульсов
            Fmax_2 = -500,	    //-500 -1200 -2800 верхний порог отбора амплитуд ударных импульсов
            n_otskok = 500,	// минимальное число точек до двойного удара
            Porog_udar2 = 10,	//8 порог обнаружения двойного удара
            Porog_tau = -20,	//пророг определения длительности удара
            N_max[300],		// координаты амплитуд ударных импульсов
            N_st_end[300][2];	// массив точек начала и конца импульсов
        qt.textBrowser->append(QString::fromStdString(TO_STRING("Разработчик Назолин А.Л. 29.03.2023г.")));
        qt.textBrowser->append(QString::fromStdString(TO_STRING("Программа формирует сигналы силы fw.dat и реакции xw.dat для последующего расчета АЧХ ускряемости статора.")));
        qt.textBrowser->append(QString::fromStdString(TO_STRING("Число точек между импульсами: N_bpf = %ld точек"+ QString::number(N_bpf))));
        qt.textBrowser->append(QString::fromStdString(TO_STRING(TO_STRING("Постоянная времени exp окна временной фильтрации сигналов силы и удара до удара: tau1=") + TO_STRING(QString::number(tau1)) + TO_STRING(" точек"))));
        qt.textBrowser->append(QString::fromStdString(TO_STRING(TO_STRING("Постоянная времени exp окна временной фильтрации сигналов силы и удара после удара: tau2=") + TO_STRING(QString::number(tau2)) + TO_STRING(" точек"))));

        const wchar_t* StartedPath = startedPath.toStdWString().c_str();

        vector<wstring>paths;
        /*!*!!!!*/ get_filesCase6(paths, startedPath.toStdWString());
        //for (int i = 0; i < paths.size(); i++) qInfo() << paths[i] ;
        qInfo() << paths[1];
        int stertsize = 1;
        stertsize = paths.size();
        qInfo() << stertsize;
        wstring* filefdat{ new wstring[stertsize] };
        //wstring* filefdat = (wstring*)malloc(stertsize * sizeof(wstring));
        if (filefdat == NULL || stertsize <= 0)
            exit(-1);
        wstring* filexdat{ new wstring[stertsize] };
       //wstring* filexdat = (wstring*)malloc(stertsize * sizeof(wstring));
        if (filefdat == NULL)
            exit(-1);
        int countf = 0;
        int countx = 0;
        //далее бдожен быть алгорим разделения оного массива на два по признаку окончания на x.dat и f.dat;
        for (long countCommon = 0; countCommon < paths.size(); ++countCommon)
        {
            qInfo() << paths[countCommon].substr(paths[countCommon].length() - 5, paths[countCommon].length());
            if (paths[countCommon].substr(paths[countCommon].length() - 5, paths[countCommon].length()) == L"f.dat")
            {
                filefdat[countf] = paths[countCommon];
                countf += 1;
                qInfo() << "findedfdat";
            }
            if (paths[countCommon].substr(paths[countCommon].length() - 5, paths[countCommon].length()) == L"x.dat")
            {
                filexdat[countx] = paths[countCommon];
                countx += 1;
                qInfo() << "findedxdat";
            }
        }


        for (long counter = 0; counter < stertsize / 2; counter++)
        {
            last += 1;
            //qt.markerOfWork->setText(QString::fromStdString(to_string(last)));
            wstring text = filefdat[counter].substr(0, filefdat[counter].length() - 5);
            if (text == L"") 
            {
                qInfo() << "Finished";
                //qt.label_13->setText(QString::fromStdWString(L"VIPOLNENO"));
                break;
            }
            for (f = 1; f <= 6; f++) //f<6
            {
                const wchar_t* CharF = filefdat[counter].c_str();
                const wchar_t* CharX = filexdat[counter].c_str();
                if ((frd1 = _wfopen(CharF, L"rb")) == NULL) {
                    fprintf(stderr, " Немогу открыть исходный файл f.dat\n");
                    fclose(frd1);

                };

                if ((frd2 = _wfopen(CharX, L"rb")) == NULL) {
                    fprintf(stderr, " Немогу открыть исходный файл x.dat \n");
                    fclose(frd2);

                };
                wstring scope;
                wstring path=L"";
                wstring path1=L"";
                switch (f)
                {
                case 1:
                    if (qt.Edit1Fmax1->toPlainText() == "" && qt.Edit1Fmax2->toPlainText() == "")
                    {
                    }
                    else {
                        if (qt.checkBox1->isChecked()) {
                            QString  Fmax1_String = qt.Edit1Fmax1->toPlainText();
                            QString  Fmax2_String = qt.Edit1Fmax2->toPlainText();
                            Fmax_1 = Fmax1_String.toLong();
                            Fmax_2 = Fmax2_String.toLong(); //значение с бокса
                            scope = to_wstring(abs(Fmax_1)) + L"-" + to_wstring(abs(Fmax_2));
                            path = text + L"fw" + to_wstring(abs(Fmax_1)) + L"-" + to_wstring(abs(Fmax_2)) + L".dat";
                            path1 = text + L"xw" + to_wstring(abs(Fmax_1)) + L"-" + to_wstring(abs(Fmax_2)) + L".dat";
                            const wchar_t* Path1 = path.c_str();
                            const wchar_t* Path2 = path1.c_str();

                            wcscpy_s(fw, Path, Path1);
                            wcscpy_s(xw, Path, Path2);
                            break;
                        }
                    }

                case 2:
                    if (qt.Edit2Fmax1->toPlainText() == "" && qt.Edit2Fmax2->toPlainText() == "") {}
                    else {
                        if (qt.checkBox2->isChecked()) {
                            QString  Fmax1_String = qt.Edit2Fmax1->toPlainText();
                            QString  Fmax2_String = qt.Edit2Fmax2->toPlainText();
                            Fmax_1 = Fmax1_String.toLong();
                            Fmax_2 = Fmax2_String.toLong();
                            scope = to_wstring(abs(Fmax_1)) + L"-" + to_wstring(abs(Fmax_2));
                            path = text + L"fw" + to_wstring(abs(Fmax_1)) + L"-" + to_wstring(abs(Fmax_2)) + L".dat";
                            path1 = text + L"xw" + to_wstring(abs(Fmax_1)) + L"-" + to_wstring(abs(Fmax_2)) + L".dat";
                            const wchar_t* Path3 = path.c_str();
                            const wchar_t* Path4 = path1.c_str();

                            wcscpy_s(fw, Path, Path3);
                            wcscpy_s(xw, Path, Path4);
                            break;
                        }
                    }

                case 3:
                    if (qt.Edit3Fmax1->toPlainText() == "" && qt.Edit3Fmax2->toPlainText() == "") {}
                    else {
                        if (qt.checkBox3->isChecked()) {
                            QString  Fmax1_String = qt.Edit3Fmax1->toPlainText();
                            QString  Fmax2_String = qt.Edit3Fmax2->toPlainText();
                            Fmax_1 = Fmax1_String.toLong();
                            Fmax_2 = Fmax2_String.toLong();
                            scope = to_wstring(abs(Fmax_1)) + L"-" + to_wstring(abs(Fmax_2));
                            path = text + L"fw" + to_wstring(abs(Fmax_1)) + L"-" + to_wstring(abs(Fmax_2)) + L".dat";
                            path1 = text + L"xw" + to_wstring(abs(Fmax_1)) + L"-" + to_wstring(abs(Fmax_2)) + L".dat";
                            const wchar_t* Path5 = path.c_str();
                            const wchar_t* Path6 = path1.c_str();

                            wcscpy_s(fw, Path, Path5);
                            wcscpy_s(xw, Path, Path6);
                            break;
                        }
                    }

                case 4:
                    if (qt.Edit4Fmax1->toPlainText() == "" && qt.Edit4Fmax2->toPlainText() == "") {}
                    else {
                        if (qt.checkBox4->isChecked()) {
                            QString  Fmax1_String = qt.Edit4Fmax1->toPlainText();
                            QString  Fmax2_String = qt.Edit4Fmax2->toPlainText();
                            Fmax_1 = Fmax1_String.toLong();
                            Fmax_2 = Fmax2_String.toLong();
                            scope = to_wstring(abs(Fmax_1)) + L"-" + to_wstring(abs(Fmax_2));
                            path = text + L"fw" + to_wstring(abs(Fmax_1)) + L"-" + to_wstring(abs(Fmax_2)) + L".dat";
                            path1 = text + L"xw" + to_wstring(abs(Fmax_1)) + L"-" + to_wstring(abs(Fmax_2)) + L".dat";
                            const wchar_t* Path7 = path.c_str();
                            const wchar_t* Path8 = path1.c_str();

                            wcscpy_s(fw, Path, Path7);
                            wcscpy_s(xw, Path, Path8);
                            break;
                        }
                    }


                case 5:
                    if (qt.Edit5Fmax1->toPlainText() == "" && qt.Edit5Fmax2->toPlainText() == "") {}
                    else {
                        if (qt.checkBox5->isChecked()) {
                            QString  Fmax1_String = qt.Edit5Fmax1->toPlainText();
                            QString  Fmax2_String = qt.Edit5Fmax2->toPlainText();
                            Fmax_1 = Fmax1_String.toLong();
                            Fmax_2 = Fmax2_String.toLong();
                            scope = to_wstring(abs(Fmax_1)) + L"-" + to_wstring(abs(Fmax_2));
                            path = text + L"fw" + to_wstring(abs(Fmax_1)) + L"-" + to_wstring(abs(Fmax_2)) + L".dat";
                            path1 = text + L"xw" + to_wstring(abs(Fmax_1)) + L"-" + to_wstring(abs(Fmax_2)) + L".dat";
                            const wchar_t* Path9 = path.c_str();
                            const wchar_t* Path10 = path1.c_str();

                            wcscpy_s(fw, Path, Path9);
                            wcscpy_s(xw, Path, Path10);
                            break;
                        }
                    }

                case 6:
                    if (qt.Edit6Fmax1->toPlainText() == "" && qt.Edit6Fmax2->toPlainText() == "") {}
                    else {
                        if (qt.checkBox6->isChecked()) {
                            QString  Fmax1_String = qt.Edit6Fmax1->toPlainText();
                            QString  Fmax2_String = qt.Edit6Fmax2->toPlainText();
                            Fmax_1 = Fmax1_String.toLong();
                            Fmax_2 = Fmax2_String.toLong();
                            scope = to_wstring(abs(Fmax_1)) + L"-" + to_wstring(abs(Fmax_2));
                            path = text + L"fw" + to_wstring(abs(Fmax_1)) + L"-" + to_wstring(abs(Fmax_2)) + L".dat";
                            path1 = text + L"xw" + to_wstring(abs(Fmax_1)) + L"-" + to_wstring(abs(Fmax_2)) + L".dat";
                            const wchar_t* Path11 = path.c_str();
                            const wchar_t* Path12 = path1.c_str();

                            wcscpy_s(fw, Path, Path11);
                            wcscpy_s(xw, Path, Path12);
                            // break;
                        }
                    }
                };


                if ((fwr1 = _wfopen(fw, L"wb")) == NULL) {
                    fprintf(stderr, "Немогу открыть файл %s для записи \n", fw);

                };

                if ((fwr2 = _wfopen(xw, L"wb")) == NULL) {
                    fprintf(stderr, "Немогу открыть файл %s для записи \n", xw);

                };
                qt.textBrowser->append("Отбор импульсов с амплитудой удара : ");
                qt.textBrowser->append(QString::number(-Fmax_1));
                qt.textBrowser->append(QString::number(-Fmax_2));
               

                // qInfo()<<"\n Отбор импульсов с амплитудой удара:\n Fmin = %ld,H\n Fmax = %ld,H"<< -Fmax_1<< -Fmax_2;

                point = -1;
                data = 0;
                N_max[0] = 0;
                Fmax[0] = 0;
                j = 0;
                i = 1;
                k = 0;
                x_old = 0;
                p = 0.0;
                while (fread(&x, sizeof(float), 1, frd1) != 0)
                {
                    point++;
                    if ((x > Fmax_1))
                    {
                        j++;

                        if ((j > n_otskok) && (j < N_bpf) && (fabs(x - x_old) > Porog_udar2))
                        {
                            p++;
                        };

                        if ((k == 1) && (j >= N_bpf))
                        {
                            if ((p == 0) && (data > Fmax_2))
                            {
                                i++;
                            }
                            else {
                                p = 0;
                            };
                            k = 0;
                            data = 0;
                        };

                    }
                    else {
                        if (x < data)
                        {
                            data = x;
                            N_max[i] = point;
                            Fmax[i] = data;
                            k = 1;
                            j = 0;
                        };
                    };
                    x_old = x;
                };


                if (p > 0) i--;

                //  qInfo()<<"\n№\tИндекс\t\tТочек\tfв,Гц \tFmax,Н";

                for (i--, j = 1; j <= i; j++)
                {
                    fseek(frd1, N_max[j] * 4, SEEK_SET);
                    fread(&x, sizeof(float), 1, frd1);

                    k = 0;
                    while (x < Porog_tau)
                    {
                        k++;
                        fseek(frd1, (N_max[j] - k) * 4, SEEK_SET);
                        fread(&x, sizeof(float), 1, frd1);
                    };
                    N_st_end[j][0] = N_max[j] - k + 1;

                    fseek(frd1, N_max[j] * 4, SEEK_SET);
                    fread(&x, sizeof(float), 1, frd1);

                    k = 0;
                    while (x < Porog_tau)
                    {
                        k++;
                        fseek(frd1, (N_max[j] + k) * 4, SEEK_SET);
                        fread(&x, sizeof(float), 1, frd1);
                    };

                    N_st_end[j][1] = N_max[j] + k;
                    data = N_st_end[j][1] - N_st_end[j][0];
                    //    qInfo()<<"\n%d\t%d\t\t%2.0f\t%5.0f\t%5.0f"<< j<< N_max[j]<< data<< 1.5 * 64000 / data<< -Fmax[j];
                    qt.textBrowser->append(QString::number(data));
                    //printf("\n %ld", N_st_end[j][1] - N_st_end[j][0]);
                };

                for (j = 1; j <= i; j++)
                {
                    fseek(frd1, N_st_end[j][1] * 4, SEEK_SET);
                    fseek(frd2, N_st_end[j][0] * 4, SEEK_SET);

                    for (mean1 = mean2 = 0, k = 1; k <= N_bpf; k++)
                    {

                        fread(&x, sizeof(float), 1, frd1);
                        mean1 += x;

                        fread(&x, sizeof(float), 1, frd2);
                        mean2 += x;
                    };
                    mean1 /= N_bpf;
                    mean2 /= N_bpf;


                    fseek(frd1, (N_st_end[j][0] - zero_point) * 4, SEEK_SET);
                    fseek(frd2, (N_st_end[j][0] - zero_point) * 4, SEEK_SET);
                    N = N_st_end[j][1] - N_st_end[j][0] + zero_point;
                    
                    for (p = 1, k = 1; k <= N_bpf; k++, p++)
                    {
                        //  обработка до удара
                        if (k <= zero_point)
                            //экспоненциальное окно для силы и отклика создание файлов
                        {
                            counterForSizeofFile += 1;
                            fread(&x, sizeof(float), 1, frd1);
                            data = koeff * (x - mean1) * exp(-(zero_point - p) / tau1);

                            fwrite(&data, sizeof(float), 1, fwr1);
                            //				fwrite(&x, sizeof(float), 1, fwr3);

                            fread(&x, sizeof(float), 1, frd2);
                            data = (x - mean2) * exp(-(zero_point - p) / tau1);
                            fwrite(&data, sizeof(float), 1, fwr2);
                        }
                        else {
                            // обработка во время и после импульса

                            if (k <= N)	// единичное окно силы и отклика во время импульса
                            {
                                counterForSizeofFile += 1;
                                fread(&x, sizeof(float), 1, frd1);
                                data = koeff * (x - mean1);
                                fwrite(&data, sizeof(float), 1, fwr1);
                                //						fwrite(&x, sizeof(float), 1, fwr3);

                                fread(&x, sizeof(float), 1, frd2);
                                data = (x - mean2);
                                fwrite(&data, sizeof(float), 1, fwr2);

                            }
                            else {
                                counterForSizeofFile += 1;
                                //экспоненциальное окно силы и отклика после импульса
                                fread(&x, sizeof(float), 1, frd1);
                                data = koeff * (x - mean1) * exp(-(p - N) / tau1);
                                fwrite(&data, sizeof(float), 1, fwr1);
                                //						fwrite(&x, sizeof(float), 1, fwr3);

                                fread(&x, sizeof(float), 1, frd2);
                                data = (x - mean2) * exp(-(p - zero_point) / tau2);
                                //					data = exp(-(p-zero_point)/tau2);    // запись exp
                                fwrite(&data, sizeof(float), 1, fwr2);
                            };
                        };
                    }

                };

                fclose(frd1);
                fclose(frd2);
                fclose(fwr2);
                fclose(fwr1);
                updateProgress6(floor(50 / 6.0 * (f + 1)));
                if (qt.checkBox->isChecked()) {
                    if (counterForSizeofFile > 0) {
                        case5(path, path1, scope, this);
                        counterForSizeofFile = -1;
                    }
                }
            };

        }
        updateProgress6(100);
        qInfo() << "Finished";
        //qt.label_13->setText(QString::fromStdWString(L"VIPOLNENO"));
       }
}

void QtWidgetsClassCase6::on_btn2_clicked()
{
    if (chek == 1)
    {
        qt.checkBox1->setChecked(true);
        qt.checkBox2->setChecked(true);
        qt.checkBox3->setChecked(true);
        qt.checkBox4->setChecked(true);
        qt.checkBox5->setChecked(true);
        qt.checkBox6->setChecked(true);
        chek = 0;
    }
    else
    {
        qt.checkBox1->setChecked(false);
        qt.checkBox2->setChecked(false);
        qt.checkBox3->setChecked(false);
        qt.checkBox4->setChecked(false);
        qt.checkBox5->setChecked(false);
        qt.checkBox6->setChecked(false);
        chek = 1;
    }
}

void QtWidgetsClassCase6::on_pushButton_9_clicked()
{
    startedPath = QFileDialog::getExistingDirectory(0, "Vibor papki", "");

    string a = "";
    wstring b = QString::fromStdString(a).toStdWString();

    string c = QString::fromStdWString(b).toStdString();
    qDebug() << startedPath;
}

QtWidgetsClassCase6::~QtWidgetsClassCase6()
{}
