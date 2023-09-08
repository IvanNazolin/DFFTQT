#include <iostream>
#include <math.h>
#include <corecrt_math_defines.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "fft-real-pair.h"
#include "customAlgo.h"
#include "Obrab_signal.h"
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include "QtWidgetsApplication1\QtWidgetsClass.h"
#include "QtWidgetsApplication1\QtWidgetsClassCase3.h"
#include "QtWidgetsApplication1\QtWidgetsClassCase2.h"
#include "QtWidgetsApplication1\QtWidgetsClassCase6.h"
#include "qstring.h"
using namespace std;

int numberchannel;

uint32_t	Npt,	// Число значений временного ряда a
			Npt_b,	// Число значений временного ряда b
			N;		// Число значений врменного ряда в алгоритме "MIC-FFT-ACHX", "MIC-DFT-ACHX"

long	Fs = 32000,			// Частота дисктеризации, 32768 Гц
		npt = 16384,		// Число точек БПФ
		np = npt / 2;		// Число значений в спектре

int		nBlocks = 1,			// Число независимых блоков усреднения
Offset = 1,				// Смещение порций БПФ, задающе число точек смещения, равное npt/ 2^(Offset-1) (1, 2, 4, 8)  
Type_file_read,			// Тип бинарного файла чтения исходного временного ряда 1 - int; 2 - float; 3 - double
Type_file_write,		// Тип бинарного файла записи результатов 1 - int; 2 - float; 3 - double
Type_spec = 2,			// Тип рассчитываемого спектра 1 - СПМ, 2 - СМ, 3 - Амп (double)
flag_stat = 1,			// Флаг расчета статистики исходного временного ряда 1 - да 0 - нет
flag_filtr = 1,			// Флаг удаления тренда временного ряда 1- да 0 - нет
n_max = 256,			// Число точек большого временного окна sin(x)/x для удаления тренда
n_min = 1,				// Число точек малого временного окна sin(x)/x после удаления тренда
flag_stat_filtr = 1,	// Флаг расчета статистики фильтрованного файла 1 - да 0 - нет
flag_wr_filtr = 1;		// Флаг записи файла фильтрации

int		mode = 10;				// Максималное число режимов в ВА испытании

double 	dF;		// Разрешение в спектре БПФ по частоте

FILE* info,		// Pointer into file FFT.log    (text)
* config;	// Pointer into file config.ini  (text)

errno_t err;
char	NameInfo[] = "FFT.log",			// Имя файла отчета о работе программы
NameConfig[] = "config.ini";	// Имя файла конфигурации


double* ReadfileRUS(const wchar_t* read_file, int Type_file, uint32_t* N);
// Чтение данных из файла read_file c инициализацией и заполнением буфера *v
double* Readfile(char* read_file, int Type_file, uint32_t* N);

//	Запись результатов расчета DFT спектра в файл 
long Save_bin_file(double* v, long n_data, char* s, int Type_file);
long Save_bin_fileRUS(double* v, long n_data, wchar_t* s, int Type_file);
//	Запись результатов расчета в столбец текстового файла 
long Save_Text_file_Column(double* v, long n_data, char* s);

//	Запись результатов расчета в строку текстового файла 
long Save_Result_Text_Line(double* v, long n_data, int heading);

//	Запись результатов расчета Param_AЧХ в строку текстового файла 
long Save_ACHX_file(double* v, long n_data, int heading, char* s, int channel);
long Save_ACHX_fileR(double* v, long n_data, int heading, wchar_t* s, int channel, int channelNumber);
//	Запись результатов расчета Param в строку текстового файла 
long Save_Param_file(double* v, long n_data, int heading, wchar_t* s, int channel);

void whitespace(wchar_t* s);

void get_files(vector<wstring>& paths, const wstring& current_path)
{
	for (const auto& file : filesystem::directory_iterator(current_path))
	{
		if (filesystem::is_directory(file))
		{
			get_files(paths, file.path().wstring());
		}
		else
		{
			paths.push_back(file.path().wstring());
		}
	}

}


int case1() 
{
	//////////
	double
		* a,		// Массив исходных данных временного ряда из файла a.dat
		* b,		// Массив исходных данных временного ряда из файла b.dat
		* x;		// Массив результатов обработки данных из файла a.dat

	char Name[15],		// Имя алгоритма программы customAlgo.c 	
		Name_wps[80];	// Имя сценария программы Obrab_signal.c 	
	int Modul;		// вариант работы этой программы
	int i;
	int permission = 1;


	char read_file_b[80];
	char 	read_file_a[80],		// Имя файла временного ряда а.dat
		write_file[80];			// имя файла сохранения результатов

	system("chcp 1251");	// кодировка вывода в консоль русских букв
	std::cout << "Программа расчета ДБПФ от 21.11.2020 \n";

	// Открываем файл *.log протокола выполнения команд программы 
	err = fopen_s(&info, NameInfo, "wt");
	if (err == 0)
		printf("\n The file %s was opened\n", NameInfo);
	else
	{
		printf("\n The file %s was not opened\n", NameInfo);
		exit(1);
	}

	printf("\n Выберите вариант работы программы: \n 0 - Выход\n 1 - Тестирование алгоритма ДБПФ\n 2 - Тестирование функций customAlgo.c");
	printf("\n 3 - ТВВ-320 Param 16кГц 10 реж БПФ 16384 8бл\n 4 - ТВВ-1000-2 АЧХ Param 32кГц БПФ 262144 1бл\n 5 - Передаточная функция\n 6 - Функция когерентности\n ");
	if (scanf_s("%d", &Modul) != 1) exit(1);

	DeviceInfo DI;
	ParamFFT ParFFT;
	/////////
	//Тестирование алгоритма ДБПФ 
	// Тестирование Функций MIT путем вычисления ошибки восстановления сигнла при взятии прямого и обратного ДБПФ 
	Test();
}

int case2(std::wstring WorkPath, int Workregim,std::string modelwork)
{
	//////
	double
		* a,		// Массив исходных данных временного ряда из файла a.dat
		* b,		// Массив исходных данных временного ряда из файла b.dat
		* x;		// Массив результатов обработки данных из файла a.dat

	char Name[15],		// Имя алгоритма программы customAlgo.c 	
		Name_wps[80];	// Имя сценария программы Obrab_signal.c 	
	int Modul;		// вариант работы этой программы
	int i;
	int permission = 1;


	char read_file_b[80];
	char 	read_file_a[80],		// Имя файла временного ряда а.dat
		write_file[80];			// имя файла сохранения результатов

	system("chcp 1251");	// кодировка вывода в консоль русских букв
	std::cout << "Программа расчета ДБПФ от 21.11.2020 \n";

	// Открываем файл *.log протокола выполнения команд программы 
	err = fopen_s(&info, NameInfo, "wt");
	if (err == 0)
		printf("\n The file %s was opened\n", NameInfo);
	else
	{
		printf("\n The file %s was not opened\n", NameInfo);
		exit(1);
	}

	DeviceInfo DI;
	ParamFFT ParFFT;
	/////
	// инициализация имени файла чтения данных
	strcpy_s(read_file_a, _countof(read_file_a), "Channel_1_16khz_double.dat");
	Type_file_read = 3; // Тип бинарного файла чтения исходного временного ряда 1 - int; 2 - float; 3 - double

	//		strcpy_s(read_file, _countof(read_file), "Channel_1_16khz_float.dat");
	//		Type_file_read = 2; // Тип бинарного файла чтения исходного временного ряда 1 - int; 2 - float; 3 - double

			// запись данных файла в динамический массив *a
	a = (double*)Readfile(read_file_a, Type_file_read, &Npt);
	if (Npt == 2)
	{
		printf("\n Error Readfile(): Kod_error = %f; nomber line = %f", a[0], a[1]);
		fprintf(info, "\n Error Readfile(): Kod_error = %f; nomber line = %f", a[0], a[1]);
		exit(1);
	};

	// Работа с контроллером MCM-204
	N = 16400;
	DI.dataCount = N;
	DI.rate = 16000;

	strcpy_s(Name, _countof(Name), "MIC-FFT"); // "Stat", "Sing"  "Trend" "FFT" "DFT" "TrendFFT" "TrendDFT" "SCADA-FFT1" "SCADA-DFT1" "SCADA-FFT2" "SCADA-DFT2" "MIC-FFT" "MIC-DFT"
	printf("\n Пользовательский алгоритм %s\n", Name);
	fprintf(info, "\n\n Пользовательский алгоритм %s\n", Name);

	// Вызов функции CustomAlgo с возвращеним результатата в массив x
	x = (double*)CustomAlgo(0, a, DI, Name, &N);
	if (N == 2)
	{
		printf("\n Error run CustomAlgo.c: Kod_error = %f; nomber line = %f", x[0], x[1]);
		fprintf(info, "\n Error run CustomAlgo: Kod_error = %f; nomber line = %f", x[0], x[1]);
		exit(1);
	};

	printf("\n Имя файла данных %s", read_file_a);
	fprintf(info, "\n Имя файла данных %s", read_file_a);

	// Формирование имени бинарного файла результатов 
	strcpy_s(write_file, _countof(write_file), Name);
	strcat_s(write_file, _countof(write_file), ".dat");
	Type_file_write = 2;	// Тип бинарного файла записи результатов 1 - int; 2 - float; 3 - double


	//Запись бинарного файла write_file типа Type_file_write
	if (Save_bin_file(x, N, write_file, Type_file_write) != N)
	{
		printf("\n Error Save_data");
		fprintf(info, "\n Error Save_data");
	};

	// Формирование имени текстового файла результатов 
	strcpy_s(write_file, _countof(write_file), Name);
	strcat_s(write_file, _countof(write_file), ".txt");
	// Запись в столбец текстового файла
	if (Save_Text_file_Column(x, N, write_file) != N)
	{
		printf("\n Error Save_data");
		fprintf(info, "\n Error Save_data");
	};

	free(a);
	free(x);
}

int case3(wstring WorkPath1, QtWidgetsClassCase3* tq, int parametr11, int parametr31, int parametr41, float parametr51)
{
	//////
	wchar_t  write_fileIV[80];
	double
		* a,		// Массив исходных данных временного ряда из файла a.dat
		* b,		// Массив исходных данных временного ряда из файла b.dat
		* x;		// Массив результатов обработки данных из файла a.dat

	char Name[15],		// Имя алгоритма программы customAlgo.c 	
		Name_wps[80];	// Имя сценария программы Obrab_signal.c 	
	int Modul;		// вариант работы этой программы
	int i;
	int permission = 1;


	char read_file_b[80];
	wchar_t		// Имя файла временного ряда а.dat
		write_file[280];			// имя файла сохранения результатов

	system("chcp 1251");	// кодировка вывода в консоль русских букв
	std::cout << "Программа расчета ДБПФ от 21.11.2020 \n";

	// Открываем файл *.log протокола выполнения команд программы 
	err = fopen_s(&info, NameInfo, "wt");
	if (err == 0)
		printf("\n The file %s was opened\n", NameInfo);
	else
	{
		printf("\n The file %s was not opened\n", NameInfo);
		exit(1);
	}


	DeviceInfo DI;
	ParamFFT ParFFT;

	/////
	//начало получения подготовительных данных
	int counterpersent8 = 0;
	int paramMODE = 1;
	float fmask1txo[8];
	float fmask0txo[8];
	float fmask1[8];
	float fmask0[8];
	wstring fileTX0[80] = { L"" };
	wstring txo[10] = { L"" };
	wstring mera[10] = { L"" };
	wstring channel1[10] = { L"" };
	wstring channel2[10] = { L"" };
	wstring channel3[10] = { L"" };
	wstring channel4[10] = { L"" };
	wstring channel5[10] = { L"" };
	wstring channel6[10] = { L"" };
	wstring channel7[10] = { L"" };
	wstring channel8[10] = { L"" };
	wstring startedPath = WorkPath1;
	wstring channels[80] = { L"" };
	const wchar_t* StartedPath = startedPath.c_str();
	vector<wstring>paths;
	get_files(paths, StartedPath);
	//получение массивов с путями к файлам
	WorkPath1 = WorkPath1 + L"/" + L"ТВВ320_param_16кГц_БПФ_16384_8b.txt";
	for (int i = 0, ch1 = 0, ch2 = 0, ch3 = 0, ch4 = 0, ch5 = 0, ch6 = 0, ch7 = 0, ch8 = 0, mer = 0, tx = 0; i < paths.size(); i++)
	{
		if (paths[i].substr(paths[i].length() - 4, paths[i].length()) == L"mera" || (paths[i].substr(paths[i].length() - 4, paths[i].length()) == L"Mera") || (paths[i].substr(paths[i].length() - 4, paths[i].length()) == L"MERA"))
		{
			mera[mer] = paths[i];
			mer += 1;
		}
		if (paths[i].substr(paths[i].length() - 3, 3) == L"TX0")
		{
			fileTX0[tx] = paths[i];
			tx += 1;
		}

		if (paths[i].substr(paths[i].length() - 4, paths[i].length()) == L".dat")
		{
			if (paths[i].substr(paths[i].length() - 17, 8) == L"Канал_ 1")
			{
				channel1[ch1] = paths[i];
				ch1 += 1;
			}
			if (paths[i].substr(paths[i].length() - 17, 8) == L"Канал_ 2")
			{
				channel2[ch2] = paths[i];
				ch2 += 1;
			}
			if (paths[i].substr(paths[i].length() - 17, 8) == L"Канал_ 3")
			{
				channel3[ch3] = paths[i];
				ch3 += 1;
			}
			if (paths[i].substr(paths[i].length() - 17, 8) == L"Канал_ 4")
			{
				channel4[ch4] = paths[i];
				ch4 += 1;
			}
			if (paths[i].substr(paths[i].length() - 17, 8) == L"Канал_ 5")
			{
				channel5[ch5] = paths[i];
				ch5 += 1;
			}
			if (paths[i].substr(paths[i].length() - 17, 8) == L"Канал_ 6")
			{
				channel6[ch6] = paths[i];
				ch6 += 1;
			}
			if (paths[i].substr(paths[i].length() - 17, 8) == L"Канал_ 7")
			{
				channel7[ch7] = paths[i];
				ch7 += 1;
			}
			if (paths[i].substr(paths[i].length() - 17, 8) == L"Канал_ 8")
			{
				channel8[ch8] = paths[i];
				ch8 += 1;
			}
		}
	}

	for (int i = 0; i < 10; i++)
	{
		channels[i] = channel1[i];
	}
	for (int i = 10; i < 20; i++)
	{
		channels[i] = channel2[i - 10];
	}
	for (int i = 20; i < 30; i++)
	{
		channels[i] = channel3[i - 20];
	}
	for (int i = 30; i < 40; i++)
	{
		channels[i] = channel4[i - 30];
	}
	for (int i = 40; i < 50; i++)
	{
		channels[i] = channel5[i - 40];
	}
	for (int i = 50; i < 60; i++)
	{
		channels[i] = channel6[i - 50];
	}
	for (int i = 60; i < 70; i++)
	{
		channels[i] = channel7[i - 60];
	}
	for (int i = 70; i < 80; i++)
	{
		channels[i] = channel8[i - 70];
	}
	string mas[150];
	int c = 0;
	for (int b = 0, mer = 0, k = 0; b < 80; b++)
	{
		if (channels[b] != L"")
		{
			k++;
			if (k % 8 == 0)
			{
				mer++;
			}
			//const wchar_t* read_file_aRUS=channels[i].c_str();
			ifstream meraF(mera[mer]);
			for (int counteri = 0; !meraF.eof(); counteri++)
			{
				getline(meraF, mas[counteri]);
				if (counteri > 146)
				{
					break;
				}
			}
			meraF.close();
			///////////////////////////////////////////////////////////////////////
			for (int i22 = 0, c22 = 0; i22 < 8; i22++)
			{
				string s;
				ifstream filetxo(fileTX0[i22]);
				getline(filetxo, s);
				int pointer = s.find(" ");
				fmask0txo[c22] = stod(s.substr(0, pointer));
				fmask1txo[c22] = stod(s.substr(pointer, s.length()));
				c22 += 1;
				filetxo.close();
			}
			//////////////////////////////////////////////////////////////////////
							//сортировка исходного массива для разделения его на дава массив mask1 и mask0 и паралельное преобразование данных в тип float;
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

			wstring outFILE = L"out.dat";
			ifstream courseF(channels[b], ios_base::binary);
			ofstream outF(outFILE, ios_base::binary);
			
			for (int counteri = 0; !courseF.eof(); counteri++)
			{
				if (fileTX0[0] != L"")
				{
					short num = 0;
					courseF.read((char*)&num, sizeof(short));
					float x = fmask1txo[c] * ((fmask1[c] * (num - fmask0[c])) + fmask0txo[c]);
					outF.write((char*)&x, sizeof(float));
				}
				else
				{
					short num = 0;
					courseF.read((char*)&num, sizeof(short));
					float x = fmask1[c] * (num - fmask0[c]);
					outF.write((char*)&x, sizeof(float));
				}
			}
			c++;
			courseF.close();
			outF.close();
			if (c == 8)
			{
				c = 0;
			}
			// инициализация имени файла чтения данных
			char  read_file_a[] = "out.dat";
			//wcscpy_s(read_file_a, _countof(read_file_a), L"C:\\Users\\Иван\\Desktop\\QtWidgetsApplication1\\QtWidgetsApplication1\\out.dat");
			Type_file_read = 2; // Тип бинарного файла чтения исходного временного ряда 1 - int; 2 - float; 3 - double

			//		strcpy_s(read_file, _countof(read_file), "Channel_1_16khz_float.dat");
			//		Type_file_read = 2; // Тип бинарного файла чтения исходного временного ряда 1 - int; 2 - float; 3 - double


					// запись данных файла в динамический массив *a
			a = (double*)Readfile(read_file_a, Type_file_read, &Npt);
			if (Npt == 2)
			{
				printf("\n Error Readfile(): Kod_error = %f; nomber line = %f", a[0], a[1]);
				fprintf(info, "\n Error Readfile(): Kod_error = %f; nomber line = %f", a[0], a[1]);
				exit(1);
			};

			// Задание параметров алгоритма ДБПФ-АЧХ
			ParFFT.Fs = parametr11;		// частота дискретизации значений сигнала в файле read_file
			ParFFT.Npt = Npt;		// число временных значений сигнала
			ParFFT.npt = parametr31;		// число точек БПФ
			ParFFT.TypeSpec = parametr41;	// Тип рассчитываемого спектра 1 - СПМ, 2 - СМ, 3 - Амп (double)

			// Расчет Зависимых параметровы алгоритма ДБПФ-АЧХ
			ParFFT.np = ParFFT.npt / 2;							// Число значений в спектре
			ParFFT.Offset = ParFFT.npt;							// Cмещение следующей порции БПФ  
			ParFFT.nBlocks = ParFFT.Npt / ParFFT.Offset - 1;	// Число блоков усреднения
			ParFFT.dF = (float)ParFFT.Fs / (float)ParFFT.npt;	// Разрешение в спектре по частоте  
			ParFFT.Fmax = ParFFT.Fs / 2;						// Максимальная частота в спектре, Гц  
			ParFFT.win_dF = parametr51;									// +- ширина полосы поиска гармоники


			// Проверка достаточности значений сигнала файла read_file для применения алгоритма FFT-Param
			if (ParFFT.nBlocks < 8)
			{
				printf("\n Insufficient data in file %s on line (%d) in file %s\n", channels[b].c_str(), __LINE__, __FILE__);
				fprintf(info, "\n Insufficient data in file %s on line (%d) in file %s\n", channels[b].c_str(), __LINE__, __FILE__);
				exit(1);
			}
			else
			{
				ParFFT.nBlocks = 8;
			};

			strcpy_s(Name_wps, _countof(Name_wps), "TBB-320-2 Param 16кГц. БПФ 16384 8бл.txt");
			tq->ProgramText("Пользовательский алгоритм: "+ QString(Name_wps));
			printf("\n Пользовательский алгоритм: %s\n", Name_wps);
			fprintf(info, "\n\n Пользовательский алгоритм: %s\n", Name_wps);
			tq->ProgramText("Имя файла данных "+ QString::fromStdWString(channels[b].c_str()));
			printf("\n Имя файла данных %s", channels[b].c_str());
			fprintf(info, "\n Имя файла данных %s", channels[b].c_str());
			tq->ProgramText("Частота дискретизации сигнал"+ QString::number(ParFFT.Fs));
			printf("\n Частота дискретизации сигнала %d", ParFFT.Fs);
			fprintf(info, "\n Частота дискретизации сигнала %d", ParFFT.Fs);
			tq->ProgramText("Число значений в файле: "+QString::number(ParFFT.Npt));
			printf("\n Число значений в файле: %d", ParFFT.Npt);
			fprintf(info, "\n Число значений в файле: %d", ParFFT.Npt);

			if ((ParFFT.npt & (ParFFT.npt - 1)) == 0)// Is power of 2 // Fft_transformRadix2(real, imag, n);
			{
				tq->ProgramText("Алгоритм БПФ. Число точек FF "+ QString::number(ParFFT.npt));
				printf("\n Алгоритм БПФ. Число точек FFT %d", ParFFT.npt);
				fprintf(info, "\n\n Алгоритм БПФ. Число точек FFT %d", ParFFT.npt);
			}
			else  // Fft_transformBluestein(real, imag, n);
			{
				tq->ProgramText("Алгоритм ДПФ. Число точек DFT: "+ QString::number(ParFFT.npt));
				printf("\n Алгоритм ДПФ. Число точек DFT: %ld", ParFFT.npt);
				fprintf(info, "\n\n Алгоритм ДПФ. Число точек: DFT %ld", ParFFT.npt);
			};
			tq->ProgramText("Число блоков FFT: "+ QString::number(ParFFT.nBlocks));
			printf("\n Число блоков FFT:  %d", ParFFT.nBlocks);
			fprintf(info, "\n\n Число блоков FFT: %d", ParFFT.nBlocks);
			tq->ProgramText("Число точек смещения: "+ QString::number(ParFFT.Offset));
			printf("\n Число точек смещения: %d", ParFFT.Offset);
			fprintf(info, "\n Число точек смещения: %d", ParFFT.Offset);

			// инициализация имени текстового файла записи результатов

			wcscpy_s(write_file, WorkPath1.c_str());

			// Вызов функции Obrab_signal.c с возвращеним расчетных значений спектральных параметров в массиве x
			x = (double*)TBB320_Param_16000_16384_8(a, ParFFT, &Npt);

			if (Npt == 2)
			{
				printf("\n Error run Obrab_signal.c: Kod_error = %f; nomber line = %f", x[0], x[1]);
				fprintf(info, "\n Error run Obrab_signal.c: Kod_error = %f; nomber line = %f", x[0], x[1]);
				exit(1);
			};

			// Запись расчетных значений в строку текстового файла write_file
			if (Save_Param_file(x, Npt, paramMODE, write_file, 1) != Npt)
			{

				printf("\n Error Save_data");
				fprintf(info, "\n Error Save_data");
				exit(1);
			};
			paramMODE += 1;
			//free(mas);
			free(a);
			free(x);
		}
		else
		{
			whitespace(write_file);
		}
		tq->updateProgress3(floor(100 / 80.0 * (b + 1)));
	}
	tq->ProgramText("Программа выполнена.");
}

int case4(wstring WorkPath, QtWidgetsClass* qt, int parametr1, int parametr2, int parametr3, int parametr4, float parametr5, int parametr6)
{
/////
	double
		* a,		// Массив исходных данных временного ряда из файла a.dat
		* b,		// Массив исходных данных временного ряда из файла b.dat
		* x;		// Массив результатов обработки данных из файла a.dat

	char Name[15],		// Имя алгоритма программы customAlgo.c 	
		Name_wps[80];	// Имя сценария программы Obrab_signal.c 	
	int i;
	int permission = 1;

	wchar_t  write_fileIV[80];

	char read_file_b[80];
	char 	read_file_a[80],		// Имя файла временного ряда а.dat
		write_file[80];			// имя файла сохранения результатов

	system("chcp 1251");	// кодировка вывода в консоль русских букв
	std::cout << "Программа расчета ДБПФ от 21.11.2020 \n";

	// Открываем файл *.log протокола выполнения команд программы 
	err = fopen_s(&info, NameInfo, "wt");
	if (err == 0)
		printf("\n The file %s was opened\n", NameInfo);
	else
	{
		printf("\n The file %s was not opened\n", NameInfo);
		exit(1);
	}

	//printf("\n Выберите вариант работы программы: \n 0 - Выход\n 1 - Тестирование алгоритма ДБПФ\n 2 - Тестирование функций customAlgo.c");
	//printf("\n 3 - ТВВ-320 Param 16кГц 10 реж БПФ 16384 8бл\n 4 - ТВВ-1000-2 АЧХ Param 32кГц БПФ 262144 1бл\n 5 - Передаточная функция\n 6 - Функция когерентности\n ");
	//if (scanf_s("%d", &Modul) != 1) exit(1);

	DeviceInfo DI;
	ParamFFT ParFFT;
///////


	float fmask1[8] = { 0 };
	float fmask0[8] = { 0 };
	double masTX0;
	double masTX1;
	string mas[150] = { "" };
	string mastxS;
	wstring fileTX0[8] = { L"" };
	wstring mera = L"0";
	wstring channel1 = L"0";
	wstring channel2 = L"0";
	wstring channel3 = L"0";
	wstring channel4 = L"0";
	wstring channel5 = L"0";
	wstring channel6 = L"0";
	wstring channel7 = L"0";
	wstring channel8 = L"0";
	wstring channels[8] = { L"0" };
	wstring startedPath = WorkPath;

	const wchar_t* StartedPath = startedPath.c_str();
	vector<wstring>paths;
	get_files(paths, StartedPath);

	for (int i = 0, tx = 0; i < paths.size(); i++)
	{
		if (paths[i].substr(paths[i].length() - 3, 3) == L"TX0")
		{
			fileTX0[tx] = paths[i];
			tx += 1;
		}
		if (paths[i].substr(paths[i].length() - 4, paths[i].length()) == L"mera" || (paths[i].substr(paths[i].length() - 4, paths[i].length()) == L"Mera") || (paths[i].substr(paths[i].length() - 4, paths[i].length()) == L"MERA"))
		{
			mera = paths[i];
		}

		if (paths[i].substr(paths[i].length() - 4, paths[i].length()) == L".dat")
		{
			if (paths[i].substr(paths[i].length() - 17, 8) == L"Канал_ 1")
			{
				channel1 = paths[i];
			}
			if (paths[i].substr(paths[i].length() - 17, 8) == L"Канал_ 2")
			{
				channel2 = paths[i];
			}
			if (paths[i].substr(paths[i].length() - 17, 8) == L"Канал_ 3")
			{
				channel3 = paths[i];
			}
			if (paths[i].substr(paths[i].length() - 17, 8) == L"Канал_ 4")
			{
				channel4 = paths[i];
			}
			if (paths[i].substr(paths[i].length() - 17, 8) == L"Канал_ 5")
			{
				channel5 = paths[i];
			}
			if (paths[i].substr(paths[i].length() - 17, 8) == L"Канал_ 6")
			{
				channel6 = paths[i];
			}
			if (paths[i].substr(paths[i].length() - 17, 8) == L"Канал_ 7")
			{
				channel7 = paths[i];
			}
			if (paths[i].substr(paths[i].length() - 17, 8) == L"Канал_ 8")
			{
				channel8 = paths[i];
			}
		}
	}

	ifstream meraF(mera);
	for (int counteri = 0; !meraF.eof(); counteri++)
	{
		getline(meraF, mas[counteri]);
		if (counteri > 146)
		{
			break;
		}
	}
	meraF.close();

	//сортировка исходного массива для разделения его на дава массив mask1 и mask0 и паралельное преобразование данных в тип float;
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

	channels[0] = channel1;
	channels[1] = channel2;
	channels[2] = channel3;
	channels[3] = channel4;
	channels[4] = channel5;
	channels[5] = channel6;
	channels[6] = channel7;
	channels[7] = channel8;

	for (int i1 = 0; i1 < 8; i1++)
	{
		// строка в кторую хаписывется срез от пути для получения номера
		numberchannel = stoi(channels[i1].substr(channels[i1].length() - 8, 4));
		ifstream TXOF(fileTX0[i1]);
		getline(TXOF, mastxS);
		TXOF.close();
		int pointer = mastxS.find(" ");
		masTX0 = stod(mastxS.substr(0, pointer));
		masTX1 = stod(mastxS.substr(pointer, mastxS.length()));

		wstring outFILE = L"out.dat";
		ifstream courseF(channels[i1], ios_base::binary);
		ofstream outF(outFILE, ios_base::binary);
		if (fileTX0[0] == L"")
		{
			for (int countere1 = 0; !courseF.eof(); countere1++)
			{
				short num = 0;
				courseF.read((char*)&num, sizeof(short));
				float x = fmask1[i1] * (num - fmask0[i1]);
				outF.write((char*)&x, sizeof(float));
			}
		}
		else
		{
			for (int countere = 0; !courseF.eof(); countere++)
			{
				short num = 0;
				courseF.read((char*)&num, sizeof(short));
				float x = masTX1 * ((fmask1[i1] * (num - fmask0[i1])) + masTX0);
				outF.write((char*)&x, sizeof(float));
			}
		}
		outF.close();
		courseF.close();
		// инициализация имени файла чтения данных
		strcpy_s(read_file_a, _countof(read_file_b), "out.dat");
		Type_file_read = 2; // Тип бинарного файла чтения исходного временного ряда 1 - int; 2 - float; 3 - double

		// запись данных файла в динамический массив *a
		a = (double*)Readfile(read_file_a, Type_file_read, &Npt);
		if (Npt == 2)
		{
			qt->ProgramText("Error Readfile(): Kod_error = " + QString::number(a[0]) + "number line =" + QString::number(a[1]));
			//printf("\n Error Readfile(): Kod_error = %f; number line = %f", a[0], a[1]);
			fprintf(info, "\n Error Readfile(): Kod_error = %f; number line = %f", a[0], a[1]);
			exit(1);
		};

		// Задание параметров алгоритма ДБПФ-АЧХ
		ParFFT.Fs = parametr1;		// частота 1 значений сигнала в файле read_file
		ParFFT.Npt = Fs * parametr2;	// число временных значений сигнала в интервале 10 сек.
		ParFFT.npt = parametr3;	// число точек БПФ
		ParFFT.TypeSpec = parametr4;	// Тип рассчитываемого спектра 1 - СПМ, 2 - СМ, 3 - Амп (double)

		// Проверка достаточности значений сигнала файла read_file для применения алгоритма FFT-АЧХ
		if ((Npt < ParFFT.Npt) || (ParFFT.npt > ParFFT.Npt))
		{
			//	printf("\n Insufficient data in file %s on line (%d) in file %s\n", read_file_a, __LINE__, __FILE__);
			fprintf(info, "\n Insufficient data in file %s on line (%d) in file %s\n", read_file_a, __LINE__, __FILE__);
			exit(1);
		};

		// Расчет Зависимых параметров алгоритма ДБПФ-АЧХ
		ParFFT.np = ParFFT.npt / 2;							// Число значений в спектре
		ParFFT.Offset = ParFFT.npt;							// Cмещение следующей порции БПФ  
		ParFFT.nBlocks = parametr6;									// Число блоков усреднения
		ParFFT.dF = (float)ParFFT.Fs / (float)ParFFT.npt;	// Разрешение в спектре по частоте  
		ParFFT.Fmax = ParFFT.Fs / 2;						// Максимальная частота в спектре, Гц  
		ParFFT.win_dF = parametr5;								// +- ширина полосы поиска гармоники

		mode = Npt / ParFFT.Npt;		// число 10 сек. интервалов в файле read_file
		if (mode > 0) N = ParFFT.Npt;

		strcpy_s(Name_wps, _countof(Name_wps), "TBB-1000-2 АЧХ Param 32кГц. БПФ 262144 1бл.");
		qt->ProgramText("Пользовательский алгоритм: " + QString(Name_wps));
		//printf("\n Пользовательский алгоритм: %s", Name_wps);
		fprintf(info, "\n\n Пользовательский алгоритм: %s", Name_wps);
		qt->ProgramText("Имя файла данных" + QString(read_file_a));
		//printf("\n Имя файла данных %s", read_file_a);
		fprintf(info, "\n Имя файла данных ", read_file_a);
		qt->ProgramText("Частота дискретизации " + QString::number(parametr1));
		//printf("\n Частота дискретизации %d", ParFFT.Fs);
		fprintf(info, "\n Частота дискретизации %d", ParFFT.Fs);
		qt->ProgramText("Число 10 сек. интервалов: " + QString::number(mode));
		//printf("\n Число 10 сек. интервалов: %d", mode);
		fprintf(info, "\n Число 10 сек. интервалов: %d", mode);
		qt->ProgramText("Число значений в 10 сек. интервале: " + QString::number(Fs * parametr2));
		//printf("\n Число значений в 10 сек. интервале: %d", ParFFT.Npt);
		fprintf(info, "\n Число значений в 10 сек. интервале: %d", ParFFT.Npt);
		qt->ProgramText("Число блоков FFT в каждом 10 сек интервале: " + QString::number(parametr6));
		//printf("\n Число блоков FFT в каждом 10 сек интервале: %d", ParFFT.nBlocks);
		fprintf(info, "\n\n Число блоков FFT в каждом 10 сек интервале: %d", ParFFT.nBlocks);

		if ((ParFFT.npt & (ParFFT.npt - 1)) == 0)// Is power of 2 // Fft_transformRadix2(real, imag, n);
		{
			qt->ProgramText("Алгоритм БПФ. Число точек FFT " + QString::number(parametr3));
			//printf("\n Алгоритм БПФ. Число точек FFT %d", ParFFT.npt);
			fprintf(info, "\n\n Алгоритм БПФ. Число точек FFT %d", ParFFT.npt);
		}
		else  // Fft_transformBluestein(real, imag, n);
		{
			qt->ProgramText("Алгоритм ДПФ. Число точек DFT " + QString::number(parametr3));
			//printf("\n Алгоритм ДПФ. Число точек DFT %ld", ParFFT.npt);
			fprintf(info, "\n\n Алгоритм ДПФ. Число точек DFT %ld", ParFFT.npt);
		};

		// инициализация имени текстового файла записи результатов
		wcscpy_s(write_fileIV, (startedPath + L"/" + L"АЧХ_param_32кГц_БПФ_262144.txt").c_str());

		for (i = 0; i < mode; i++)
		{
			// Вызов функции Obrab_signal.c с возвращеним расчетных значений спектральных параметров 10 сек интервалов в массиве x
			x = (double*)TBB1000_ACHX_Param_32000_262144_1(&a[N * i], ParFFT, &N);

			if (N == 2)
			{
				printf("\n Error run Obrab_signal.c: Kod_error = %f; nomber line = %f", x[0], x[1]);
				fprintf(info, "\n Error run Obrab_signal.cpp: Kod_error = %f; nomber line = %f", x[0], x[1]);
				exit(1);
			};

			// Запись расчетных значений в строку текстового файла write_fileIV
			if (Save_ACHX_fileR(x, N, permission, write_fileIV, 1, i1) != N)
			{
				qt->ProgramText("Error Save_data");
				printf("\n Error Save_data");
				fprintf(info, "\n Error Save_data");
				exit(1);
			};
			permission += 1;
			free(x);
			N = 10 * Fs;
		};
		free(a);
		whitespace(write_fileIV);
		qt->updateProgress(floor(100 / 8.0 * (i1 + 1)));
	}
	qt->ProgramText("Программа выполнена.");
}

int case5(wstring WorkPathFileA, wstring WorkPathFileB,wstring scope, QtWidgetsClassCase6* io)
{
	///////
	double
		* a,		// Массив исходных данных временного ряда из файла a.dat
		* b,		// Массив исходных данных временного ряда из файла b.dat
		* x;		// Массив результатов обработки данных из файла a.dat

	wchar_t Name[15];		// Имя алгоритма программы customAlgo.c 	
	char	Name_wps[80];	// Имя сценария программы Obrab_signal.c 	
	int Modul;		// вариант работы этой программы
	int i;
	int permission = 1;

	wchar_t write_file[300];			// имя файла сохранения результатов

	system("chcp 1251");	// кодировка вывода в консоль русских букв
	std::cout << "Программа расчета ДБПФ от 21.11.2020 \n";

	// Открываем файл *.log протокола выполнения команд программы 
	err = fopen_s(&info, NameInfo, "wt");
	if (err == 0)
		printf("\n The file %s was opened\n", NameInfo);
	else
	{
		printf("\n The file %s was not opened\n", NameInfo);
		exit(1);
	}
	DeviceInfo DI;
	ParamFFT ParFFT;
	//////////
	strcpy_s(Name_wps, _countof(Name_wps), "Передаточная функция.");
	printf("\n Пользовательский алгоритм: %s", Name_wps);
	fprintf(info, "\n\n Пользовательский алгоритм: %s", Name_wps);


	// Диапазон ударных сил
	//strcpy_s(Name, _countof(Name), "1000-1500.dat");
	 
	// инициализация имени файла силы
	//strcpy_s(read_file_a, _countof(read_file_a), "fw");
	//strcat_s(read_file_a, _countof(read_file_a), Name);
	Type_file_read = 2; // Тип бинарного файла чтения исходного временного ряда 1 - int; 2 - float; 3 - double
	const wchar_t* read_file_a = WorkPathFileA.c_str();
	// запись данных файла в динамический массив *a
	a = (double*)ReadfileRUS(read_file_a, Type_file_read, &Npt);
	if (Npt == 2)
	{
		printf("\n Error Readfile(): Kod_error = %f; nomber line = %f", a[0], a[1]);
		fprintf(info, "\n Error Readfile(): Kod_error = %f; nomber line = %f", a[0], a[1]);
		exit(1);
	};

	// инициализация имени файла реакции
	//strcpy_s(read_file_a, _countof(read_file_a), "xw");
	//strcat_s(read_file_a, _countof(read_file_a), Name);
	Type_file_read = 2; // Тип бинарного файла чтения исходного временного ряда 1 - int; 2 - float; 3 - double
	const wchar_t* read_file_b = WorkPathFileB.c_str();
	// запись данных файла в динамический массив *a
	b = (double*)ReadfileRUS(read_file_b, Type_file_read, &Npt_b);
	if (Npt_b == 2)
	{
		printf("\n Error Readfile(): Kod_error = %f; nomber line = %f", b[0], b[1]);
		fprintf(info, "\n Error Readfile(): Kod_error = %f; nomber line = %f", b[0], b[1]);
		exit(1);
	};
	if (Npt != Npt_b)
	{
		printf("\n Ошибка!!! Число точек сигнала силы %d != числу точек сигнала реакции %d", Npt, Npt_b);
		fprintf(info, "\n\n Ошибка!!! Число точек сигнала силы %d != числу точек сигнала реакции %d", Npt, Npt_b);
		exit(1);
	};

	// Задание параметров алгоритма ДБПФ-АЧХ
	ParFFT.Fs = 64000;		// частота дискретизации значений сигнала файлов read_file_a и read_file_b
	ParFFT.Npt = Npt;		// число временных значений в сигнале сила и сигнале реакции b
	ParFFT.npt = 16384;		// число точек БПФ
	ParFFT.TypeSpec = 2;	// Тип рассчитываемого спектра 1 - СПМ, 2 - СМ, 3 - Амп (double)

	// Проверка достаточности значений сигнала файла read_file для применения алгоритма FFT-АЧХ
	if (ParFFT.npt > ParFFT.Npt)
	{
		printf("\n Insufficient data in file %s on line (%d) in file %s\n", read_file_a, __LINE__, __FILE__);
		fprintf(info, "\n Insufficient data in file %s on line (%d) in file %s\n", read_file_a, __LINE__, __FILE__);
		exit(1);
	};

	// Расчет Зависимых параметров алгоритма ДБПФ-АЧХ
	ParFFT.np = ParFFT.npt / 2;							// Число значений в спектре
	ParFFT.Offset = ParFFT.npt;							// Cмещение следующей порции БПФ  
	ParFFT.nBlocks = ParFFT.Npt / ParFFT.Offset - 1;	// Число блоков усреднения
	ParFFT.dF = (float)ParFFT.Fs / (float)ParFFT.npt;	// Разрешение в спектре по частоте  
	ParFFT.Fmax = ParFFT.Fs / 2;						// Максимальная частота в спектре, Гц  

	printf("\n Имя файла сигнала силы %s", read_file_a);
	fprintf(info, "\n Имя файла сигнала силы %s", read_file_a);

	printf("\n Имя файла сигнала реакции %s", read_file_b);
	fprintf(info, "\n Имя файла сигнала реакции %s", read_file_b);

	printf("\n Частота дискретизации сигналов %d", ParFFT.Fs);
	fprintf(info, "\n Частота дискретизации сигналов %d", ParFFT.Fs);

	printf("\n Число значений в файлах: %d", ParFFT.Npt);
	fprintf(info, "\n Число значений в файлах: %d", ParFFT.Npt);

	if ((ParFFT.npt & (ParFFT.npt - 1)) == 0)// Is power of 2 // Fft_transformRadix2(real, imag, n);
	{
		printf("\n Алгоритм БПФ. Число точек FFT %d", ParFFT.npt);
		fprintf(info, "\n\n Алгоритм БПФ. Число точек FFT %d", ParFFT.npt);
	}
	else  // Fft_transformBluestein(real, imag, n);
	{
		printf("\n Алгоритм ДПФ. Число точек DFT: %ld", ParFFT.npt);
		fprintf(info, "\n\n Алгоритм ДПФ. Число точек: DFT %ld", ParFFT.npt);
	};

	printf("\n Число блоков усреднения FFT:  %d", ParFFT.nBlocks);
	fprintf(info, "\n\n Число блоков усреднения FFT: %d", ParFFT.nBlocks);

	printf("\n Число точек смещения: %d", ParFFT.Offset);
	fprintf(info, "\n Число точек смещения: %d", ParFFT.Offset);

	int Type_H = 1; // Вид характеристики: 1 - H1(f); 2 - H2(f) 

	printf("\n Вид характеристики: H%d", Type_H = 1);
	fprintf(info, "\n Вид характеристики: H%d", Type_H = 1);

	// Вызов функции Obrab_signal.c с возвращеним расчетных значений спектральных параметров в массиве x
	x = (double*)ACHX_function(a, b, ParFFT, Type_H, &Npt);

	if (Npt == 2)
	{
		printf("\n Error run Obrab_signal.c: Kod_error = %f; nomber line = %f", x[0], x[1]);
		fprintf(info, "\n Error run Obrab_signal.c: Kod_error = %f; nomber line = %f", x[0], x[1]);
		exit(1);
	};

	for (int count= WorkPathFileA.length()-1;count >= 0;count--)
	{
		if (WorkPathFileA[count] == L'\\')
		{
			if (Type_H == 1)
				WorkPathFileA = WorkPathFileA.substr(0, count + 1) +L"F"+scope+ L"_H1-1.dat";
			else
				WorkPathFileA = WorkPathFileA.substr(0, count + 1) +L"F"+scope+ L"_H2-2.dat";
			
			wcscpy_s(write_file, WorkPathFileA.c_str());
			break;
		}
	}

	//wcscpy_s(write_file, Name);
	Type_file_write = 2;	// Тип бинарного файла записи результатов 1 - int; 2 - float; 3 - double

	//Запись бинарного файла write_file типа Type_file_write
	if (Save_bin_fileRUS(x, Npt, write_file, Type_file_write) != Npt)
	{
		printf("\n Error Save_data");
		fprintf(info, "\n Error Save_data");
	};
	free(a);
	free(x);
	fclose(info);
}

//------------------ Чтение данных из файла read_file c инициализацией и заполнением буфера *v
// read_file - имя файла данных для чтения
// Type_file - тип файла данных (1 - shot (16 бит); 2 - float (32 бита); 3 - double (64 бита)
// v - имя массива данных в ОЗУ (возвращается в место вызова функции)
// N - число значений файла и массива данных ОЗУ (возвращается в место вызова функции)
double* Readfile(char* read_file, int Type_file, uint32_t* N)
{
	double* v;
	FILE* in;		// Указатель на файл read_file
	struct	stat statbuf;	// Струкутра определения параметров файла
	
	uint32_t i;		// текущий индекс массива

	double* ret = (double*)malloc(sizeof(double) * 2);
	// Возвращаемые коды ошибки ret[0]
	// 1.0 - ошибка выделения динамической памяти 
	// 2.0 - ошибка математической обработки данных
	// 3.0 - ошибка задания параметров 
	// 4.0 - ошибка недостаточно данных для математической обработки
	// 5.0 - ошибка. Запрещенные мат. операции (деление на ноль, корень из отрицательного числа)
	// 6.0 - ошибка чтения файла

	// Возвращаемые коды ошибки ret[1]
	// xxx.0 - Номер строки программы возникновения ошибки

	printf("\n Read data from file %s", read_file);
	fprintf(info, "\n Read data from file %s", read_file);

	err = fopen_s(&in, read_file, "rb");
	if (err == 0)
	{
		printf("\n The file %s was opened", read_file);
		fprintf(info, "\n The file %s was opened", read_file);

		//Инициализация параметров файла
		stat(read_file, &statbuf);

		// Определение числа значений файла
		switch (Type_file)
		{
		case 1:	*N = statbuf.st_size / 2L;	//int 16 бит число
			break;
		case 2:	*N = statbuf.st_size / 4L; //float 32 бита число
			break;
		case 3:	*N = statbuf.st_size / 8L; //double 64 бита число
		}

		// Выделение динамической памяти под значения исходного временного ряда
		if ((v = (double*)calloc(*N, sizeof(double))) == NULL)
		{
			printf("\n Not enough  memory for v[%ld] on line (%d) in file %s", *N, __LINE__, __FILE__);
			fprintf(info, "\n Not enough  memory for v[%ld] on line (%d) in file %s", *N, __LINE__, __FILE__);
			*N = 2;  ret[0] = 1.0;	ret[1] = __LINE__;	return ret;
		}

		// Close file *in if it is not NULL   
		if (in)
		{
			//Инициализация памяти значениями временного ряда из файла
			switch (Type_file)
			{
			case 1:
				int data_int;
				for (i = 0; i < *N; i++)
				{
					if (fread(&data_int, sizeof(float), 1, in) != 1)
					{
						printf("\n Error read data from file %s on line (%d) in file %s\n", read_file, __LINE__, __FILE__);
						fprintf(info, "\n Error read data from file %s on line (%d) in file %s\n", read_file, __LINE__, __FILE__);
						*N = 2; ret[0] = 6.0;	ret[1] = __LINE__;	return ret;
					};
					v[i] = data_int;
				};
				break;
			case 2:
				float data;
				for (i = 0; i < *N; i++)
				{
					if (fread(&data, sizeof(float), 1, in) != 1)
					{
						printf("\n Error read data from file %s on line (%d) in file %s\n", read_file, __LINE__, __FILE__);
						fprintf(info, "\n Error read data from file %s on line (%d) in file %s\n", read_file, __LINE__, __FILE__);
						*N = 2; ret[0] = 6.0;	ret[1] = __LINE__;	return ret;
					};
					v[i] = data;
				};
				break;
			case 3:
				if (fread(&v[0], sizeof(double), *N, in) != *N)
				{
					printf("\n Error read data from file %s on line (%d) in file %s\n", read_file, __LINE__, __FILE__);
					fprintf(info, "\n Error read data from file %s on line (%d) in file %s\n", read_file, __LINE__, __FILE__);
					*N = 2; ret[0] = 6.0;	ret[1] = __LINE__;	return ret;
				};
				break;
			default:
				printf("\n Error parametr Type_file = %d on line (%d) in file %s\n", Type_file, __LINE__, __FILE__);
				fprintf(info, "\n Error parametr Type_file = %d on line (%d) in file %s\n", Type_file, __LINE__, __FILE__);
				*N = 2; ret[0] = 6.0;	ret[1] = __LINE__;	return ret;
			}

			printf("\n Число значений временного ряда = %ld", *N);
			fprintf(info, "\n Число значений временного ряда= %ld", *N);

			err = fclose(in);
			if (err == 0)
				printf("\n The file %s was readed and closed\n", read_file);
			else
				printf("\nThe file %s was readed and closed\n", read_file);
		}
	}
	else
	{
		printf("\n The file %s was not opened\n", read_file);
		fprintf(info, "\n The file %s was not opened\n", read_file);
		fprintf(info, "\n Error. Exit Programm.");
		*N = 2; ret[0] = 6.0;	ret[1] = __LINE__;	return ret;
	}
	free(ret);
	return v;
}

double* ReadfileRUS(const wchar_t* read_file, int Type_file, uint32_t* N)
{
	double* v;
	FILE* in;		// Указатель на файл read_file
	struct	_stat statbuf;	// Струкутра определения параметров файла

	uint32_t i;		// текущий индекс массива

	double* ret = (double*)malloc(sizeof(double) * 2);
	// Возвращаемые коды ошибки ret[0]
	// 1.0 - ошибка выделения динамической памяти 
	// 2.0 - ошибка математической обработки данных
	// 3.0 - ошибка задания параметров 
	// 4.0 - ошибка недостаточно данных для математической обработки
	// 5.0 - ошибка. Запрещенные мат. операции (деление на ноль, корень из отрицательного числа)
	// 6.0 - ошибка чтения файла

	// Возвращаемые коды ошибки ret[1]
	// xxx.0 - Номер строки программы возникновения ошибки

	printf("\n Read data from file %s", read_file);
	fprintf(info, "\n Read data from file %s", read_file);
	err = _wfopen_s(&in, read_file, L"rb");
	if (err == 0)
	{
		printf("\n The file %s was opened", read_file);
		fprintf(info, "\n The file %s was opened", read_file);

		//Инициализация параметров файла
		_wstat(read_file, &statbuf);

		// Определение числа значений файла
		switch (Type_file)
		{
		case 1:	*N = statbuf.st_size / 2L;	//int 16 бит число
			break;
		case 2:	*N = statbuf.st_size / 4L; //float 32 бита число
			break;
		case 3:	*N = statbuf.st_size / 8L; //double 64 бита число
		}

		// Выделение динамической памяти под значения исходного временного ряда
		if ((v = (double*)calloc(*N, sizeof(double))) == NULL)
		{
			printf("\n Not enough  memory for v[%ld] on line (%d) in file %s", *N, __LINE__, __FILE__);
			fprintf(info, "\n Not enough  memory for v[%ld] on line (%d) in file %s", *N, __LINE__, __FILE__);
			*N = 2;  ret[0] = 1.0;	ret[1] = __LINE__;	return ret;
		}

		// Close file *in if it is not NULL   
		if (in)
		{
			//Инициализация памяти значениями временного ряда из файла
			switch (Type_file)
			{
			case 1:
				int data_int;
				for (i = 0; i < *N; i++)
				{
					if (fread(&data_int, sizeof(float), 1, in) != 1)
					{
						printf("\n Error read data from file %s on line (%d) in file %s\n", read_file, __LINE__, __FILE__);
						fprintf(info, "\n Error read data from file %s on line (%d) in file %s\n", read_file, __LINE__, __FILE__);
						*N = 2; ret[0] = 6.0;	ret[1] = __LINE__;	return ret;
					};
					v[i] = data_int;
				};
				break;
			case 2:
				float data;
				for (i = 0; i < *N; i++)
				{
					if (fread(&data, sizeof(float), 1, in) != 1)
					{
						printf("\n Error read data from file %s on line (%d) in file %s\n", read_file, __LINE__, __FILE__);
						fprintf(info, "\n Error read data from file %s on line (%d) in file %s\n", read_file, __LINE__, __FILE__);
						*N = 2; ret[0] = 6.0;	ret[1] = __LINE__;	return ret;
					};
					v[i] = data;
				};
				break;
			case 3:
				if (fread(&v[0], sizeof(double), *N, in) != *N)
				{
					printf("\n Error read data from file %s on line (%d) in file %s\n", read_file, __LINE__, __FILE__);
					fprintf(info, "\n Error read data from file %s on line (%d) in file %s\n", read_file, __LINE__, __FILE__);
					*N = 2; ret[0] = 6.0;	ret[1] = __LINE__;	return ret;
				};
				break;
			default:
				printf("\n Error parametr Type_file = %d on line (%d) in file %s\n", Type_file, __LINE__, __FILE__);
				fprintf(info, "\n Error parametr Type_file = %d on line (%d) in file %s\n", Type_file, __LINE__, __FILE__);
				*N = 2; ret[0] = 6.0;	ret[1] = __LINE__;	return ret;
			}

			printf("\n Число значений временного ряда = %ld", *N);
			fprintf(info, "\n Число значений временного ряда= %ld", *N);

			err = fclose(in);
			if (err == 0)
				printf("\n The file %s was readed and closed\n", read_file);
			else
				printf("\nThe file %s was readed and closed\n", read_file);
		}
	}
	else
	{
		printf("\n The file %s was not opened\n", read_file);
		fprintf(info, "\n The file %s was not opened\n", read_file);
		fprintf(info, "\n Error. Exit Programm.");
		*N = 2; ret[0] = 6.0;	ret[1] = __LINE__;	return ret;
	}
	free(ret);
	return v;
}

//	Запись результатов расчета CastomASlgo.c в файл в бинарный файл
// *v - массив резултатов
// n_data - число значений массива *v
// *s - имя бинарного файла запсиси результатов
// Type_file -  тип бинарного файла 1 - int, 2 - float, 3 - double

long Save_bin_fileRUS(double* v, long n_data, wchar_t* s, int Type_file)
{
	FILE* f_wr;
	long i;

	err = _wfopen_s(&f_wr, s, L"wb");
	if (err == 0)
	{
		printf("\n The file %s was opened", s);
		fprintf(info, "\n The file %s was opened", s);
	}
	else
	{
		printf("\n The %s was not opened", s);
		fprintf(info, "\n The %s was not opened", s);
		return 0;
	}

	// Write and Close f_wr if it is not NULL   
	if (f_wr)
	{
		switch (Type_file)
		{
		case 1:
			int in;
			for (i = 0; i < n_data; i++)
			{
				in = (int)v[i];
				if (fwrite(&in, sizeof(int), 1, f_wr) != 1)
				{
					printf("\n Error write data into file %s", s);
					fprintf(info, "\n Error write data into file %s", s);
					fclose(f_wr);
					return i;
				};
			};
			n_data = i - 1;
			break;
		case 2:
			float f;
			for (i = 0; i < n_data; i++)
			{
				f = (float)v[i];
				if (fwrite(&f, sizeof(float), 1, f_wr) != 1)
				{
					printf("\n Error write data into file %s", s);
					fprintf(info, "\n Error write data into file %s", s);
					fclose(f_wr);
					return i;
				};
			};
			n_data = i - 1;
			break;
		case 3:
			if (fwrite(&v[0], sizeof(double), n_data, f_wr) != n_data)
			{
				printf("\n Error write data into file %s", s);
				fprintf(info, "\n Error write data into file %s", s);
				fclose(f_wr);
				return 0;
			};
		};

		err = fclose(f_wr);
		if (err == 0)
		{
			printf("\n The file %s was write and closed - Ok", s);
			fprintf(info, "\n The file %s was write and closed - Ok", s);
		}
		else
		{
			printf("\n The file %s was not closed", s);
			fprintf(info, "\n The file %s was not closed", s);
			return 0;
		}
	}
	else
	{
		printf("\n Error open file %s", s);
		fprintf(info, "\n Error open file %s", s);
		return 0;
	}
	return n_data;
}


long Save_bin_file(double* v, long n_data, char* s, int Type_file)
{
	FILE* f_wr;
	long i;

	err = fopen_s(&f_wr, s, "wb");
	if (err == 0)
	{
		printf("\n The file %s was opened", s);
		fprintf(info, "\n The file %s was opened", s);
	}
	else
	{
		printf("\n The %s was not opened", s);
		fprintf(info, "\n The %s was not opened", s);
		return 0;
	}

	// Write and Close f_wr if it is not NULL   
	if (f_wr)
	{
		switch (Type_file)
		{
		case 1:
			int in;
			for (i = 0; i < n_data; i++)
			{
				in = (int)v[i];
				if (fwrite(&in, sizeof(int), 1, f_wr) != 1)
				{
					printf("\n Error write data into file %s", s);
					fprintf(info, "\n Error write data into file %s", s);
					fclose(f_wr);
					return i;
				};
			};
			n_data = i - 1;
			break;
		case 2:
			float f;
			for (i = 0; i < n_data; i++)
			{
				f = (float)v[i];
				if (fwrite(&f, sizeof(float), 1, f_wr) != 1)
				{
					printf("\n Error write data into file %s", s);
					fprintf(info, "\n Error write data into file %s", s);
					fclose(f_wr);
					return i;
				};
			};
			n_data = i - 1;
			break;
		case 3:
			if (fwrite(&v[0], sizeof(double), n_data, f_wr) != n_data)
			{
				printf("\n Error write data into file %s", s);
				fprintf(info, "\n Error write data into file %s", s);
				fclose(f_wr);
				return 0;
			};
		};

		err = fclose(f_wr);
		if (err == 0)
		{
			printf("\n The file %s was write and closed - Ok", s);
			fprintf(info, "\n The file %s was write and closed - Ok", s);
		}
		else
		{
			printf("\n The file %s was not closed", s);
			fprintf(info, "\n The file %s was not closed", s);
			return 0;
		}
	}
	else
	{
		printf("\n Error open file %s", s);
		fprintf(info, "\n Error open file %s", s);
		return 0;
	}
	return n_data;
}

//	Запись результатов расчета в столбец текстового файла 
long Save_Text_file_Column(double* v, long n_data, char* s)
{
	FILE* f_wr;
	long i;

	err = fopen_s(&f_wr, s, "wt");
	if (err == 0)
	{
		printf("\n The file %s was opened", s);
		fprintf(info, "\n The file %s was opened", s);
	}
	else
	{
		printf("\n The %s was not opened", s);
		fprintf(info, "\n The %s was not opened", s);
		return 0;
	}

	// Write and Close f_wr if it is not NULL   
	if (f_wr)
	{
		for (i = 0; i < n_data; i++)
		{
			fprintf(f_wr, "%f\n", v[i]);
		}

		err = fclose(f_wr);
		if (err == 0)
		{
			printf("\n The file %s was write and closed - Ok", s);
			fprintf(info, "\n The file %s was write and closed - Ok", s);
		}
		else
		{
			printf("\n The file %s was not closed", s);
			fprintf(info, "\n The file %s was not closed", s);
			return 0;
		}
	}
	else
	{
		printf("\n Error open file %s", s);
		fprintf(info, "\n Error open file %s", s);
		return 0;
	}
	return n_data;
}

//	Запись результатов расчета в строку текстового файла 
// v -  мсассив результатов
// n_data - число данных масиива v
//heading - 1 - запись результатов с заголовком;  0 - нет, только результаты

long Save_Result_Text_Line(double* v, long n_data, int heading)
{
	char s[80];
	FILE* f_wr;
	double d = 0;
	long i;

	err = strcpy_s(s, _countof(s), "Result.txt");

	err = fopen_s(&f_wr, s, "wt");
	if (err == 0)
	{
		printf("\n The file %s was opened", s);
		fprintf(info, "\n The file %s was opened", s);
	}
	else
	{
		printf("\n The %s was not opened", s);
		fprintf(info, "\n The %s was not opened", s);
		return 0;
	}

	// Write and Close f_wr if it is not NULL   
	if (f_wr)
	{
		if (heading == 1)
		{
			fprintf(f_wr, "Канал_ №\t\tAg(100)\t\tAgm(155-185)\tFgm(155-185)\tAg(1450)\tAg(2900)\tAg(4350)\tAg(5800)\tAg(7250)\tAgm(1-8000)\tFgm(1-8000)\tM(155-185)\tM1(2-10)\tM2(10-20)\tM3(20-30)\tM4(30-40)\tM5(40-50)\tM6(50-60)\tM7(60-70)\tM8(70-80)\tM1g(2-10)\tM2g(10-20)\tM3g(20-30)\tM4g(30-40)\tM5g(40-50)\tM6g(50-60)\tM7g(60-70)\tM8g(70-80)\tM(1-5000)\n");
		};
		fprintf(f_wr, "Канал_ №\t\t%7.4f\t\t %7.4f\t\t %7.1f\t\t", v[13], v[14], v[15]);
		for (i = 16; i <= 21; i++)	fprintf(f_wr, "%7.4f\t\t", v[i]);
		fprintf(f_wr, "%7.1f\t\t", v[22]);
		for (i = 23; i < (n_data - 2); i++)	fprintf(f_wr, "%7.4f\t\t", v[i]);
		fprintf(f_wr, "%7.2f\n", v[i]);

		err = fclose(f_wr);
		if (err == 0)
		{
			printf("\n The file %s was write and closed - Ok", s);
			fprintf(info, "\n The file %s was write and closed - Ok", s);
		}
		else
		{
			printf("\n The file %s was not closed", s);
			fprintf(info, "\n The file %s was not closed", s);
			exit(1);
		}
	}
	else
	{
		printf("\n Error open file %s", s);
		fprintf(info, "\n Error open file %s", s);
		return 0;
	}
	return n_data;
}


long Save_ACHX_fileR(double* v, long n_data, int heading, wchar_t* s, int channel,int channelNumber)
{
	FILE* f_wr;
	double d = 0;
	short i;

	if (heading == 1)
	{
		// открываем новый файл с удаление предыдущей версии
		err = _wfopen_s(&f_wr, s, L"w");
	}
	else
	{
		// добавляем занянеия в уже созданный файл
		err = _wfopen_s(&f_wr, s, L"a+");
	};
	if (err == 0)
	{
		printf("\n The file %s was opened", s);
		fprintf(info, "\n The file %s was opened", s);
	}
	else
	{
		printf("\n The %s was not opened on line (%d) in file %s\n", s, __LINE__, __FILE__);
		fprintf(info, "\n The %s was not opened on line (%d) in file %s\n", s, __LINE__, __FILE__);
		return 0;
	}

	// Write and Close f_wr if it is not NULL   
	if (f_wr)
	{
		if (heading == 1)
		{
			fprintf(f_wr, "Канал_ №\t\tFg(100)\t\tAg(100)\t\tAgm(155-185)\tFgm(155-185)\tAg(1450)\tAg(2900)\tAg(4350)\tAg(5800)\tAg(7250)\tAgm(1-8000)\tFgm(1-8000)\tM(155-185)\tM1(2-10)\tM2(10-20)\tM3(20-30)\tM4(30-40)\tM5(40-50)\tM6(50-60)\tM7(60-70)\tM8(70-80)\tM1g(2-10)\tM2g(10-20)\tM3g(20-30)\tM4g(30-40)\tM5g(40-50)\tM6g(50-60)\tM7g(60-70)\tM8g(70-80)\tM(1-5000)\n");
		};
		fprintf(f_wr, "Канал_ %d_%d\t\t%7.1f\t\t%7.4f\t\t%7.4f\t\t\t%7.1f\t", channelNumber+1, numberchannel, v[0], v[1], v[2], v[3]);
		for (i = 4; i <= 9; i++)	fprintf(f_wr, "\t\t%7.4f", v[i]);
		fprintf(f_wr, "\t\t%7.1f", v[10]);
		for (i = 11; i < (n_data - 2); i++)	fprintf(f_wr, "\t\t%7.4f", v[i]);
		fprintf(f_wr, "\t\t%7.2f\n", v[i]);

		err = fclose(f_wr);
		if (err == 0)
		{
			printf("\n The file %s was write and closed - Ok", s);
			fprintf(info, "\n The file %s was write and closed - Ok", s);
		}
		else
		{
			printf("\n The file %s was not closed on line (%d) in file %s\n", s, __LINE__, __FILE__);
			fprintf(info, "\n The file %s was not closed on line (%d) in file %s\n", s, __LINE__, __FILE__);
			return 0;
		}
	}
	else
	{
		printf("\n Error open file %s on line (%d) in file %s\n", s, __LINE__, __FILE__);
		fprintf(info, "\n Error open file %s on line (%d) in file %s\n", s, __LINE__, __FILE__);
		return 0;
	}
	return n_data;
}

//	Запись результатов расчета парамтеро спектра в строку текстового файла формат АЧХ
// v -  массив данных для записи в строку файла *s 
// n_data - число данных массива v
// heading - 1 - запись результатов с заголовком;  0 - нет, только результаты
// s - имя файла записи результатов
// channel - номер измерительного канала (0,1,2...8)
long Save_ACHX_file(double* v, long n_data, int heading, char* s, int channel)
{
	FILE* f_wr;
	double d = 0;
	short i;

	if (heading == 1)
	{
		// открываем новый файл с удаление предыдущей версии
		err = fopen_s(&f_wr, s, "w");
	}
	else
	{
		// добавляем занянеия в уже созданный файл
		err = fopen_s(&f_wr, s, "a+");
	};
	if (err == 0)
	{
		printf("\n The file %s was opened", s);
		fprintf(info, "\n The file %s was opened", s);
	}
	else
	{
		printf("\n The %s was not opened on line (%d) in file %s\n", s, __LINE__, __FILE__);
		fprintf(info, "\n The %s was not opened on line (%d) in file %s\n", s, __LINE__, __FILE__);
		return 0;
	}

	// Write and Close f_wr if it is not NULL   
	if (f_wr)
	{
		if (heading == 1)
		{
			fprintf(f_wr, "Канал_ №\t\tFg(100)\t\tAg(100)\t\tAgm(155-185)\tFgm(155-185)\tAg(1450)\tAg(2900)\tAg(4350)\tAg(5800)\tAg(7250)\tAgm(1-8000)\tFgm(1-8000)\tM(155-185)\tM1(2-10)\tM2(10-20)\tM3(20-30)\tM4(30-40)\tM5(40-50)\tM6(50-60)\tM7(60-70)\tM8(70-80)\tM1g(2-10)\tM2g(10-20)\tM3g(20-30)\tM4g(30-40)\tM5g(40-50)\tM6g(50-60)\tM7g(60-70)\tM8g(70-80)\tM(1-5000)\n");
		};
		fprintf(f_wr, "Канал_ № %d\t\t%7.1f\t\t%7.4f\t\t%7.4f\t\t\t%7.1f\t",numberchannel, v[0], v[1], v[2], v[3]);
		for (i = 4; i <= 9; i++)	fprintf(f_wr, "\t\t%7.4f", v[i]);
		fprintf(f_wr, "\t\t%7.1f", v[10]);
		for (i = 11; i < (n_data - 2); i++)	fprintf(f_wr, "\t\t%7.4f", v[i]);
		fprintf(f_wr, "\t\t%7.2f\n", v[i]);

		err = fclose(f_wr);
		if (err == 0)
		{
			printf("\n The file %s was write and closed - Ok", s);
			fprintf(info, "\n The file %s was write and closed - Ok", s);
		}
		else
		{
			printf("\n The file %s was not closed on line (%d) in file %s\n", s, __LINE__, __FILE__);
			fprintf(info, "\n The file %s was not closed on line (%d) in file %s\n", s, __LINE__, __FILE__);
			return 0;
		}
	}
	else
	{
		printf("\n Error open file %s on line (%d) in file %s\n", s, __LINE__, __FILE__);
		fprintf(info, "\n Error open file %s on line (%d) in file %s\n", s, __LINE__, __FILE__);
		return 0;
	}
	return n_data;
}

void whitespace(wchar_t* s)
{
	FILE* f_wr;
	err = _wfopen_s(&f_wr, s, L"a+");
	fprintf(f_wr, "\n");
	fclose(f_wr);
}

//	Запись результатов расчета параметров спектра в строку текстового файла формата Param
// v -  массив данных для записи в строку файла *s 
// n_data - число данных массива v
// heading - 1 - запись результатов с заголовком;  0 - нет, только результаты
// s - имя файла записи результатов
// channel - номер измерительного канала (0,1,2...8)
long Save_Param_file(double* v, long n_data, int heading, wchar_t* s, int channel)
{
	FILE* f_wr;
	double d = 0;
	short i;

	if (heading == 1)
	{
		// открываем новый файл с удаление предыдущей версии
		err = _wfopen_s(&f_wr, s, L"w");
	}
	else
	{
		// добавляем занянеия в уже созданный файл
		err = _wfopen_s(&f_wr, s, L"a+");
	};
	if (err == 0)
	{
		printf("\n The file %s was opened", s);
		fprintf(info, "\n The file %s was opened", s);
	}
	else
	{
		printf("\n The %s was not opened on line (%d) in file %s\n", s, __LINE__, __FILE__);
		fprintf(info, "\n The %s was not opened on line (%d) in file %s\n", s, __LINE__, __FILE__);
		return 0;
	}

	// Write and Close f_wr if it is not NULL   
	if (f_wr)
	{
		if (heading == 1)
		{
			fprintf(f_wr, "Канал_ №\t\tAg(100)\t\tAgm(155-185)\tFgm(155-185)\tAg(1450)\tAg(2900)\tAg(4350)\tAg(5800)\tAg(7250)\tAgm(1-8000)\tFgm(1-8000)\tM(155-185)\tM1(2-10)\tM2(10-20)\tM3(20-30)\tM4(30-40)\tM5(40-50)\tM6(50-60)\tM7(60-70)\tM8(70-80)\tM1g(2-10)\tM2g(10-20)\tM3g(20-30)\tM4g(30-40)\tM5g(40-50)\tM6g(50-60)\tM7g(60-70)\tM8g(70-80)\tM(1-5000)\n");
		};
		fprintf(f_wr, "Канал_ №\t\t%7.4f\t\t%7.4f\t\t\t%7.1f\t", v[0], v[1], v[2]);
		for (i = 3; i <= 8; i++)	fprintf(f_wr, "\t\t%7.4f", v[i]);
		fprintf(f_wr, "\t\t%7.1f", v[9]);
		for (i = 10; i < (n_data - 2); i++)	fprintf(f_wr, "\t\t%7.4f", v[i]);
		fprintf(f_wr, "\t\t%7.2f\n", v[i]);

		err = fclose(f_wr);
		if (err == 0)
		{
			printf("\n The file %s was write and closed - Ok", s);
			fprintf(info, "\n The file %s was write and closed - Ok", s);
		}
		else
		{
			printf("\n The file %s was not closed on line (%d) in file %s\n", s, __LINE__, __FILE__);
			fprintf(info, "\n The file %s was not closed on line (%d) in file %s\n", s, __LINE__, __FILE__);
			return 0;
		}
	}
	else
	{
		printf("\n Error open file %s on line (%d) in file %s\n", s, __LINE__, __FILE__);
		fprintf(info, "\n Error open file %s on line (%d) in file %s\n", s, __LINE__, __FILE__);
		return 0;
	}
	return n_data;
}