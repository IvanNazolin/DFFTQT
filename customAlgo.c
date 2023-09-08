/*Программа castoAlgo.c - программа обработки данных виброакустического сигнала по 8-ти пользовательским алгоритмам

* Предназначена для компиляции в Linux с последующей загрузкой образа castoAlgo.so в модуль МСМ-204

* В модуле MCM-204 для выбора алгоритма обработки в поле "Data Type" выбирается Castomization в поле "Сastomization Parameter" - текстовое имя алгоритма, напирмер Monitoring
* В программе castoAlgo.c текстовое имя алгоритм содержится в переменной customParams
* 1. "Stаt" - расчет статистических параметров сигнала
* 2. "Sing" - НЧ фильтрация временного ряда окном sing() c числом точек nmax
* 3. "Trend" - Полосовая фильтрация временного ряда двумя окнами sing() c числом точек nmax и nmin для удаления тренда
* 4. "FFT" или "DFT" -  расчет спектра по алгоритму БПФ (алг. Кули-Тьюки) с число точек Npt = 2^n <= DataCount, или ДПФ дискретного преобразования Фурье (алгоритм Блюстейна) с числом точек Npt=DataCount 
* 5. "TrendFFT" или "TrendDFT" - последовательное выполнение алгоритмов п.3 + п.4.
* 6. "SCADA_FFT1" или "SCADA_DFT1" - послед-е выполение алгор. (1 + 3 + 1 + 4 + Param) с расчетом временных (нефильтрованных и фильтрованных) стат. парам.(14) и спектр. парам.(45) для SCADA (всего 80)
* 7. "SCADA_FFT2" или "SCADA_DFT2" - послед-е выполение алгор. (1 + 4 + Param) с расчетом временных (нефильтрованных) стат. парам. (7) и спектральных парам. (45 параметр) для SCADA (всего 80)
* 8. "MIC-FFT" или "MIC-DFT" - последовательное выполение алгоритмов (4 + Param) с расчетом спектральных параметров (28 параметров) для алгоритма ВАД (аналог обработки MIC-300) (всего 42)

* Особенности БПФ и ДПФ. 
* 1. Алгоритмы БПФ и ДПФ взяты из открытого источника Copyright(c) 2020 Project Nayuki. (MIT License)
* https://www.nayuki.io/page/free-small-fft-in-multiple-languages
* 2. Число точек БПФ Npt = 2^n <= DataCountFFT (ближайшее меньшее значение).
* 3. Число точек ДПФ Npt = DataCountFFT числу точек временного ряда.
* 4. Число блок усреденения во всех алгоритмах БПФ и БПФ = 1.
* 5. Разрешенние в спектре по частоте определяется только временем измерения. Ограничений  - нет. 
* 6. Алгоритм ДПФ в отличие от БПФ позволяет рассчитывать спектр по произвольному числу точек, на обязательно равным 2^n.
*
* Разработка и авторские права на программу принадлежат ООО НТЦ "Ресурс", разработчик Назолин А.Л. 
* Дата создания 12.07.2023г. ver.3.0 linix
*/

#include "customAlgo.h"
#include "math.h"
#include "string.h"
#include "fft-real-pair.h"
#include "Obrab_signal.h"


#ifdef __linux__ 
double* CustomAlgo(uint16_t chIndex, void *rawData, DeviceInfo devInfo, char* customParams, uint32_t* outCount)
#endif 

#ifdef _WIN32 
double* CustomAlgo(uint16_t chIndex, double* raw, DeviceInfo devInfo, char* customParams, uint32_t* outCount)
#endif 

{
	uint32_t	COUNT;				// число значений, возвращаемых программой castomAlgo(...) в MCM-204
	uint32_t	i;					// Текущий индекс массива данных
	uint16_t	Kod_Program = 0;	// Возвращаемое значение пограммы
	long		Kod_LINE = 0;		// Код ошибки выполнения программы
	
	// Определение и выделение динамической памяти под возвращаемый код работы п/п 
	double* ret = (double*)malloc(sizeof(double) * 2);	
	ret[0] = 0.0;	//нормальное завершение программы
	ret[1] = 0.0;	//нормальное завершение программы
	
	// Возвращаемые коды ошибки ret[0]
	// 1.0 - ошибка выделения динамической памяти 
	// 2.0 - ошибка математической обработки данных
	// 3.0 - ошибка задания параметров 
	// 4.0 - ошибка недостаточно данных для математической обработки
	// 5.0 - ошибка. Запрещенные мат. операции (деление на ноль, корень из отрицательного числа)
	
	// Возвращаемые коды ошибки ret[1]
 	// xxx.0 - Номер строки программы возникновения ошибки

	// Число возвращаемых значений п/п =  числу точек временного ряда к обработке
	COUNT = devInfo.dataCount;

	// Определение и выделение динамической памяти для дублирования (декодирования) значений временного ряда
	double* gArray = (double*)malloc(sizeof(double) * (COUNT));
	if (!gArray)
	{
		*outCount = 2;	ret[0] = 1.0;	ret[1] = __LINE__;	return ret;
	}

#ifdef __linux__ 
	uint32_t* raw = (uint32_t*)rawData;			// указатель на массив временных данных rawData всех каналов

	raw = raw + chIndex * devInfo.dataCount;	// указатель на начало массива временных данных канала chIndex 

	double inputRange = devInfo.inputRange == B10 ? 10.0 : 1.25;
	double scalingFactor = inputRange/8388607.0*1000.0/devInfo.sensor.sensitivity;//for convert rawData to g
	
	// Декодирование значений rawData с записью в массив gArray значений ускорения размерности (G) 
	for(i = 0; i < COUNT; i++)
        gArray[i] = scalingFactor*(((raw[i] & 0x00800000) == 0x00800000) ? (int32_t)(raw[i] | 0xFF000000) : (int32_t)raw[i]);
#endif 

	//--------------- Программы пользователя -------
	double
		*data,		// массив результатов расчета по алгоритмам 1...5
		*MCM_data;	// массив результатов расчета по алгоритмаму 6, 7, 8, 9
	
	uint16_t
		nmax = 300,	// число точек большого окна sing(x)
		nmin = 1,	// число точек малого окна sing(x)
		MCM_SCADA = 80,			//число возвращаемых значений функцией castomAlgo(...) по алгоритмам 6(полный) и 7(сокращенный) для SCADA АСДМГ 
		MCM_MIC300 = 42,		//число возвращаемых значений функцией castomAlgo(...) по алгоритмy 8 (для алгоритма ВАД, аналог файла обработки данных MIC-300М) 
		MCM_MIC300_ACHX = 43,	//число возвращаемых значений функцией castomAlgo(...) по алгоритмy 9 (для алгоритма ВАД, аналог файла обработки данных MIC-300М-АЧХ) 
		MCM_step_garmonic = 50;	// шаг оценки периодических гаромник спектра 50 или 100 Гц
	
	ParamStatistic		StatPar,	// Объявление переменной структуры оценки статистических параметров ВА сигнала до обработки
						StatPar_;	// Объявление переменной структуры оценки статистических параметров ВА сигнала после обработки
	ParamFFT			ParFFT;		// Объявление переменной структуры параметров расчета БПФ или ДПФ
	ParamAmplSpectr		ParSpec;	// Объявление переменной структуры расчетных значений спектральных параметров ВА сигнала

#ifdef _WIN32 
	// Дублирование значений временного ряда *raw в массив *gArray
	for (i = 0; i < COUNT; i++)		gArray[i] = raw[i];
#endif 


	// -------------- 1. Пользовательский алгоритм "Stat" ---------------------
	// Расчет статистических параметров временной реализации ВА сигнала
	
	if (strcmp(customParams, "Stat") == 0)
	{
		// Число возвращаемых расчетных значений по алгоритму "Stat"
		COUNT = 14;
		*outCount = COUNT;

		// Выделение динамической памяти под результаты расчета по алгоритму "Stat"
		if ( !(data = (double*)malloc(sizeof(double) * COUNT)) )
		{
			free(gArray);	*outCount = 2;	ret[0] = 1.0;	ret[1] = __LINE__;	return ret;
		};

		// Расчет статистических характеристик ВА сигнала
		if ((Kod_Program = statictica(gArray, devInfo.dataCount, &StatPar, &Kod_LINE)) != 0)
		{
			free(gArray);	*outCount = 2;	ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		}
		else
		{
			data[0] = StatPar.Mx;
			data[1] = StatPar.Disp;
			data[2] = StatPar.CKO;
			data[3] = StatPar.CKZ;
			data[4] = StatPar.Ax;
			data[5] = StatPar.Ex;
			data[6] = StatPar.Ampl;
			data[7] = StatPar.Amax;
			data[8] = StatPar.Pogr_MX;
			data[9] = StatPar.Pogr_Disp;
			data[10] = StatPar.Pogr_CKZ;
			data[11] = StatPar.Pogr_CKO;
			data[12] = StatPar.Pogr_Ax;
			data[13] = StatPar.Pogr_Ex;
			free(gArray); free(ret);
			return data;
		};
	};

	// -------------- 2. Пользовательский алгоритм "Sing" ---------------------	
	// Фильтрация реализации ВА сигнгала временным окном Sing

	if (strcmp(customParams, "Sing") == 0)
	{
		// Число возвращаемых расчетных значений по алгоритму "Sing" = числу значений временного ряда
		COUNT = devInfo.dataCount;
		*outCount = COUNT;

		// Выделение динамической памяти под результаты расчета по алгоритму "Sing"
		if ( !(data = (double*)malloc(sizeof(double) * COUNT)) )
		{
			free(gArray);	*outCount = 2;	ret[0] = 1;		ret[1] = __LINE__;	return ret;
		}

		if ((Kod_Program = Sing_Win(gArray, data, nmax, devInfo.dataCount, &Kod_LINE)) != 0)
		{
			free(gArray); free(data);	*outCount = 2;	ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		}
		else
		{
			free(gArray);	free(ret);	
			return data;
		};
	};

	// -------------- 3. Пользовательский алгоритм "Trend" ---------------------
	// Удаление тренда ВА сигнала с помощью двух временных окон Sing с большим и малым числом точек

	if (strcmp(customParams, "Trend") == 0)
	{
		// Число возвращаемых расчетных значений по алгоритму "Trend" = числу значений временного ряда
		COUNT = devInfo.dataCount;
		*outCount = COUNT;
		
		// Выделение динамической памяти под результаты расчета по алгоритму "Trend"
		if ( !(data = (double*)malloc(sizeof(double) * COUNT)) )
		{
			free(gArray);	*outCount = 2;	ret[0] = 1;	ret[1] = __LINE__;	return ret;
		}

		if ((Kod_Program = Make_File_raznoct(gArray, data, nmax, nmin, devInfo.dataCount, &Kod_LINE)) != 0)
		{
			free(gArray); free(data);	*outCount = 2;	ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		}
		else
		{
			free(gArray); free(ret);
			return data;
		}
	};
	
	// -------------- 4. Пользовательский алгоритм "FFT" или "DFT" ---------------------
	// Расчет спектра ВА сигнала по алгоритму БПФ или ДПФ

	if (strcmp(customParams, "FFT") == 0 || strcmp(customParams, "DFT") == 0)
	{
		// Инициализация параметров алгоритма БПФ или ДПФ из параметров функции customAlgo(...)
		ParFFT.TypeSpec = 3;						// Тип рассчитываемого спектра 1 - СПМ, 2 - СМ, 3 - Амп (double)
		ParFFT.Fs = (uint32_t)devInfo.rate;			// Частота дискретизации сигнала виброускорения, Гц	

		if (strcmp(customParams, "FFT") == 0)
		{
			ParFFT.n = 8;		// Минимальное число точек БПФ 2^8 = 256
			// Проверка достаточности точек временного ряда - должно быть больше числа точек БПФ 2^n = 256 
			if ((ParFFT.Npt = (uint32_t)pow(2, ParFFT.n)) > devInfo.dataCount)
			{
				free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
			};

			//Автоматическое определение числа точек временного ряда для алгоритма БПФ
			for (; (ParFFT.Npt = (uint32_t)pow(2, ParFFT.n)) < devInfo.dataCount; ParFFT.n++);

			ParFFT.Npt /= 2;				// Максимальное число точек временного ряда, кратных 2^n
			ParFFT.npt = ParFFT.Npt;		// Число точек расчета БПФ = числу точек временного ряда, кратных 2^n (1 блок БПФ, разрешение в спектре по частоте df = Fs / npt - любое )
		}
		else	//---------- "DFT" -------------
		{
			ParFFT.Npt = devInfo.dataCount;		// Число точек временного ряда, участвующих в алгоритме ДПФ
			ParFFT.npt = ParFFT.Npt;			// Число точек ДПФ = числу точек временного ряда (1 блок ДПФ, разрешение в спектре по частоте df = Fs / npt - любое)
		};

		// Расчетные параметры спектра
		ParFFT.np = ParFFT.npt / 2;					// Число значений в спектре
		ParFFT.Offset = ParFFT.npt / 2;				// Cмещение следующей порции БПФ  
		ParFFT.nBlocks = ParFFT.Npt / ParFFT.Offset - 1;	// Число блоков усреднения
		ParFFT.dF = (float)ParFFT.Fs / (float)ParFFT.npt;			// Разрешение в спектре по частоте  
		ParFFT.Fmax = ParFFT.dF * ParFFT.np;		// Максимальная частота в спектре, Гц  

		if (ParFFT.nBlocks < 1)
		{
			free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
		}

		// Число возвращаемых расчетных значений по алгоритму "FFT" или "DFT"
		COUNT = ParFFT.np;
		*outCount = COUNT;

		// Выделение динамической памяти под результаты расчета по алгоритму "FFT" или "DFT"
		if ( !(data = (double*)calloc(COUNT, sizeof(double))) )
		{
			free(gArray);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
		}

		// Расчет спектра по алгоритму ДБПФ
		if ( (Kod_Program = DFTAnalysis(gArray, data, ParFFT, &Kod_LINE)) != 0)
		{
			free(gArray);	free(data);	*outCount = 2;	ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		}
		else
		{
			free(gArray); free(ret);
			return data;
		}
	}


	// -------------- 5. Пользовательский алгоритм "TrendFFT" "TrendDFT" ---------------------
	// Удаление тренда ВА сигнала с использованием временных окон и расчет спектра ВА сигнала по алгоритму БПФ или ДПФ
	
	if (strcmp(customParams, "TrendFFT") == 0 || strcmp(customParams, "TrendDFT") == 0)
	{

		// 5.1 Удаление тренда временного ряда
		
		// Выделение динамической памяти под результаты расчета тренда
		if (!(data = (double*)malloc(sizeof(double) * devInfo.dataCount)))
		{
			free(gArray);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
		}
		
		// Программа удаления тренда временного ряда
		if ((Kod_Program = Make_File_raznoct(gArray, data, nmax, nmin, devInfo.dataCount, &Kod_LINE)) != 0)
		{
			free(gArray);	 free(data);	*outCount = 2;	ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		}
		else
		{
			// Переопределение исходного массива временного ряда значения временного ряда без тренда
			for (i = 0; i < devInfo.dataCount; i++)		gArray[i] = data[i];
			
			free(data);
		}
		
		// 5.2 Расчет спектра сигнала с использованием алгоритма БПФ или ДПФ
	
		// Инициализация параметров алгоритма БПФ или ДПФ из параметров функции customAlgo(...)
		ParFFT.TypeSpec = 3;						// Тип рассчитываемого спектра 1 - СПМ, 2 - СМ, 3 - Амп (double)
		ParFFT.Fs = (uint32_t)devInfo.rate;			// Частота дискретизации сигнала виброускорения, Гц	

		if (strcmp(customParams, "FFT") == 0)
		{
			ParFFT.n = 8;		// Минимальное число точек БПФ 2^8 = 256
			
			// Проверка достаточности точек временного ряда - должно быть больше числа точек БПФ 2^n = 256 
			if ((ParFFT.Npt = (uint32_t)pow(2, ParFFT.n)) > devInfo.dataCount)
			{
				free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
			};

			//Автоматическое определение масимального числа значений временного ряда, используемых в алгоритме БПФ
			for (; (ParFFT.Npt = (uint32_t)pow(2, ParFFT.n)) < devInfo.dataCount; ParFFT.n++);

			ParFFT.Npt /= 2;				// Максимальное число точек временного ряда, кратных 2^n
			ParFFT.npt = ParFFT.Npt;		// Число точек БПФ = числу точек временного ряда, кратных 2^n (1 блок БПФ, разрешение в спектре по частоте df = Fs / npt - любое )
		}
		else	//---------- "DFT" -------------
		{

			ParFFT.Npt = devInfo.dataCount;		// Число точек временного ряда, участвующих в алгоритме ДПФ
			ParFFT.npt = ParFFT.Npt;			// Число точек ДПФ = числу точек временного ряда (1 блок ДПФ, разрешение в спектре по частоте df = Fs / npt - любое)
		};

		// Расчетные параметры спектра
		ParFFT.np = ParFFT.npt / 2;					// Число значений в спектре
		ParFFT.Offset = ParFFT.npt / 2;				// Cмещение следующей порции БПФ  
		ParFFT.nBlocks = ParFFT.Npt / ParFFT.Offset - 1;	// Число блоков усреднения
		ParFFT.dF = (float)ParFFT.Fs / (float)ParFFT.npt;			// Разрешение в спектре по частоте  
		ParFFT.Fmax = ParFFT.dF * ParFFT.np;		// Максимальная частота в спектре, Гц  
		
		if (ParFFT.nBlocks < 1)
		{
			free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
		}

		// Число возвращаемых расчетных значений по алгоритму "TrendFFT" "TrendDFT" = числу значений спектра
		COUNT = ParFFT.np;
		*outCount = COUNT;

		// Выделение динамической памяти под результаты расчета спектра
		if (!(data = (double*)calloc(COUNT, sizeof(double))))
		{
			free(gArray);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
		}

		// Расчет спектра по алгоритму ДБПФ
		if ((Kod_Program = DFTAnalysis(gArray, data, ParFFT, &Kod_LINE)) != 0)
		{
			free(gArray);	free(data);	*outCount = 2;	ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		}
		else
		{
			free(gArray); free(ret);
			return data;
		}
	}

    // -------------- 6. Пользовательский алгоритм "SCADA-FFT1" или "SCADA-DFT1"  ---------------------
	// Расчет стат. характеристик исходного ВА сигнала
	// Удаление тренда ВА сиггнала с использование временных окон
	// Расчет стат. характеристик отфильтрованого ВА сигнала
	// Расчтет алгоритма БПФ или ДПФ отфильтрованного ВА сигнала
	// Расчет спетктральных параметров отфильтрованного ВА сигнала

    if (strcmp(customParams, "SCADA-FFT1") == 0 || strcmp(customParams, "SCADA-DFT1") == 0)
	{
		// Число возвращаемых расчетных значений по алгоритму "SCADA-FFT1" или "SCADA-DFT1" 
		*outCount = MCM_SCADA;
		
		//	Выделение динамической памяти под результаты расчета по алгоритму "SCADA-FFT1" или "SCADA-DFT1" 
        if (! (MCM_data = (double*)malloc(sizeof(double) * MCM_SCADA)) )
		{
			free(gArray);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
		};
		
		// 6.1 Расчет статистических характеристик исходного  ВА сигнала
		if ((Kod_Program = statictica(gArray, devInfo.dataCount, &StatPar, &Kod_LINE)) != 0)
		{
			free(gArray);	*outCount = 2;	ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		};

		// 6.2 Удаление тренда временного ряда
		// Выделение динамической памятьи под результ удаления тренда временного ряда
		if (!(data = (double*)malloc(sizeof(double) * devInfo.dataCount)))
		{
			free(gArray);	*outCount = 2;	ret[0] = 1;	ret[1] = __LINE__;	return ret;
		}

		// Удаление тренда временного ряда
		if ((Kod_Program = Make_File_raznoct(gArray, data, nmax, nmin, devInfo.dataCount, &Kod_LINE)) != 0)
		{
			free(gArray); free(data);	*outCount = 2;	ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		}
		else
		{
			// Переопределение исходного массива временного ряда значения временного ряда без тренда
			for (i = 0; i < devInfo.dataCount; i++)		gArray[i] = data[i];
			
			free(data);
		}
		
		// 6.3 Расчет статистических характеристик сигнала после удаления треда
		if ((Kod_Program = statictica(gArray, devInfo.dataCount, &StatPar_, &Kod_LINE)) != 0)
		{
			free(gArray);	*outCount = 2;	ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		};

		// 6.4. Расчет спектра с использованием алгоритма БПФ и БПФ
		
		// Инициализация параметров алгоритма ДПФ и БПФ из параметров функции customAlgo(...)
		ParFFT.TypeSpec = 2;						// Тип рассчитываемого спектра 1 - СПМ, 2 - СМ, 3 - Амп (double)
		ParFFT.Fs = (uint32_t)devInfo.rate;			// Частота дискретизации сигнала виброускорения, Гц	

		if (strcmp(customParams, "SCADA-FFT1") == 0)
		{
			ParFFT.n = 8;		// Минимальное число точек БПФ 2^8 = 256
			// Проверка достаточности точек временного ряда - должно быть больше числа точек БПФ 2^n = 256 
			if ((ParFFT.Npt = (uint32_t)pow(2, ParFFT.n)) > devInfo.dataCount)
			{
				free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
			};

			//Автоматическое определение максимального числа значений временного ряда, используемых в алгоритме БПФ
			for (; (ParFFT.Npt = (uint32_t)pow(2, ParFFT.n)) < devInfo.dataCount; ParFFT.n++);

			ParFFT.Npt /= 2;				// Максимальное число точек временного ряда, кратных 2^n
			ParFFT.npt = ParFFT.Npt;		// Число точек БПФ = числу точек временного ряда, кратных 2^n (1 блок БПФ, разрешение в спектре по частоте df = Fs / npt - любое )
		}
		else	//---------- "SCADA-DFT1" -------------
		{			
			ParFFT.Npt = devInfo.dataCount;		// Число точек временного ряда, участвующих в алгоритме ДПФ
			ParFFT.npt = ParFFT.Npt;			// Число точек ДПФ = числу точек временного ряда (1 блок ДПФ, разрешение в спектре по частоте df = Fs / npt - любое)
		};

		// Расчетные параметры спектра
		ParFFT.np = ParFFT.npt / 2;					// Число значений в спектре
		ParFFT.Offset = ParFFT.npt / 2;				// Cмещение следующей порции БПФ  
		ParFFT.nBlocks = ParFFT.Npt / ParFFT.Offset - 1;	// Число блоков усреднения
		ParFFT.dF = (float)ParFFT.Fs / (float)ParFFT.npt;			// Разрешение в спектре по частоте   
		ParFFT.Fmax = ParFFT.dF * ParFFT.np;		// Максимальная частота в спектре, Гц  

		if (ParFFT.nBlocks < 1)
		{
			free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
		}

		// Выделение динамической памяти под результат расчета спектра
		if (!(data = (double*)calloc(ParFFT.np, sizeof(double))))
		{
			free(gArray);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
		};
		// Расчет спектра по алгоритму ДБПФ
		if ((Kod_Program = DFTAnalysis(gArray, data, ParFFT, &Kod_LINE)) != 0)
		{
			free(gArray);	free(data);		*outCount = 2;		ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		};

		// 6.5 Расчет спектральных параметров

		ParFFT.win_dF = 2;							// +- ширина полосы поиска гармоники, Гц
		
		// Расчет спектральных параметров по алгоритму ТВВ-320 Param
		if ((Kod_Program = Diagnostic_Parameters_TBB320(data, ParFFT, &ParSpec, MCM_step_garmonic, &Kod_LINE)) != 0)
		{
			free(gArray);	free(data);		free(MCM_data);		*outCount = 2;		ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		}
		else
		{
			if (strcmp(customParams, "SCADA-FFT1") == 0)
				MCM_data[0] = 6.0;
			else
				MCM_data[0] = 6.1;
			MCM_data[1] = devInfo.dataCount;
			MCM_data[2] = devInfo.rate;
			MCM_data[3] = devInfo.inputRange;
			MCM_data[4] = devInfo.sensor.sensitivity;

			MCM_data[5] = StatPar.Pogr_MX;
			MCM_data[6] = StatPar.Pogr_Disp;
			MCM_data[7] = StatPar.Pogr_CKO;
			MCM_data[8] = StatPar.Pogr_CKZ;
			MCM_data[9] = StatPar.Pogr_Ax;
			MCM_data[10] = StatPar.Pogr_Ex;

			MCM_data[11] = nmax;
			MCM_data[12] = nmin;

			if ((ParFFT.npt & (ParFFT.npt - 1)) == 0)  // Is power of 2
				MCM_data[13] = 0; // Fft_transformRadix2(real, imag, n);
			else  // More complicated algorithm for arbitrary sizes
				MCM_data[13] = 1; // Fft_transformBluestein(real, imag, n);

			MCM_data[14] = ParFFT.npt;
			MCM_data[15] = ParFFT.nBlocks;
			MCM_data[16] = ParFFT.Offset;
			MCM_data[17] = ParFFT.Fmax;
			MCM_data[18] = ParFFT.dF;

			MCM_data[19] = ParFFT.win_dF;
			MCM_data[20] = MCM_step_garmonic;

			MCM_data[21] = StatPar.Mx;
			MCM_data[22] = StatPar.CKO;
			MCM_data[23] = StatPar.CKZ;
			MCM_data[24] = StatPar.Ax;
			MCM_data[25] = StatPar.Ex;
			MCM_data[26] = StatPar.Ampl;
			MCM_data[27] = StatPar.Amax;

			MCM_data[28] = StatPar_.Mx;
			MCM_data[29] = StatPar_.CKO;
			MCM_data[30] = StatPar_.CKZ;
			MCM_data[31] = StatPar_.Ax;
			MCM_data[32] = StatPar_.Ex;
			MCM_data[33] = StatPar_.Ampl;
			MCM_data[34] = StatPar_.Amax;

			MCM_data[35] = ParSpec.A100;
			MCM_data[36] = ParSpec.Agmax_155_185;
			MCM_data[37] = ParSpec.Fgmax_155_185;
			MCM_data[38] = ParSpec.A1_1450;
			MCM_data[39] = ParSpec.A2_2900;
			MCM_data[40] = ParSpec.A3_4350;
			MCM_data[41] = ParSpec.A4_5800;
			MCM_data[42] = ParSpec.A5_7250;
			MCM_data[43] = ParSpec.Agmax;
			MCM_data[44] = ParSpec.Fgmax;
			MCM_data[45] = sqrt(ParSpec.M_155_185);
			
			for (i = 0; i <= 15; i++)
			{
				MCM_data[46 + i] = sqrt(ParSpec.M[i]);
				MCM_data[62 + i] = sqrt(ParSpec.Mg[i]);
			}
			
			MCM_data[78] = sqrt(ParSpec.M_1_5000);
			MCM_data[79] = 0.0;	//код нормального завершения программы castomAlgo.c

			free(gArray); free(ret);
   			return MCM_data;
		}
	}

	// -------------- 7. Пользовательский алгоритм "SCADA-FFT2" или "SCADA-DFT2"  (упрощенный алгоритм без фильтрации)---------------------
	// Расчет стат. характеристик исходного ВА сигнала
	// Расчтет алгоритма БПФ или ДПФ исходного ВА сигнала
	// Расчет спетктральных параметров исходного ВА сигнала
	if (strcmp(customParams, "SCADA-FFT2") == 0 || strcmp(customParams, "SCADA-DFT2") == 0)
	{
		// Число возвращаемых расчетных значений по алгоритму "SCADA-FFT2" или "SCADA-DFT2" 
		*outCount = MCM_SCADA;
		
		//	Выделение динамической памяти под массив спектральных параметров ВА сигнала (результаты работы программы)
        if (!(MCM_data = (double*)malloc(sizeof(double) * MCM_SCADA)))
		{
			free(gArray);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
		};

		// 7.1 Расчет статистических характеристик исходного ВА сигнала
		if ((Kod_Program = statictica(gArray, devInfo.dataCount, &StatPar, &Kod_LINE)) != 0)
		{
			free(gArray);	*outCount = 2;	ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		};

		// 7.2. Расчет спектра с использованием алгоритма ДПФ, БПФ

		ParFFT.TypeSpec = 2;						// Тип рассчитываемого спектра 1 - СПМ, 2 - СМ, 3 - Амп (double)
		
		// Инициализация параметров алгоритма БПФ и ДПФ из параметров функции customAlgo(...)
		ParFFT.Fs = (uint32_t)devInfo.rate;			// Частота дискретизации сигнала виброускорения, Гц	

		if (strcmp(customParams, "SCADA-FFT2") == 0)
		{
			ParFFT.n = 8;		// Минимальное число точек БПФ 2^8 = 256
			// Проверка достаточности точек временного ряда - должно быть больше числа точек БПФ 2^n = 256 
			if ((ParFFT.Npt = (uint32_t)pow(2, ParFFT.n)) > devInfo.dataCount)
			{
				free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
			};

			//Автоопределение максимального числа значений временного ряда, используемых в алгоритме БПФ
			for (; (ParFFT.Npt = (uint32_t)pow(2, ParFFT.n)) < devInfo.dataCount; ParFFT.n++);

			ParFFT.Npt /= 2;				// Максимальное число точек временного ряда, кратных 2^n
			ParFFT.npt = ParFFT.Npt;		// Число точек БПФ = числу точек временного ряда, кратных 2^n (1 блок БПФ, разрешение в спектре по частоте df = Fs / npt - любое )	
		}
		else	//---------- "SCADA-DFT2" -------------
		{
			ParFFT.Npt = devInfo.dataCount;		// Число точек временного ряда, участвующих в алгоритме ДПФ
			ParFFT.npt = ParFFT.Npt;			// Число точек ДПФ = числу точек временного ряда (1 блок ДПФ, разрешение в спектре по частоте df = Fs / npt - любое)
		};

		// Расчетные параметры спектра
		ParFFT.np = ParFFT.npt / 2;					// Число значений в спектре
		ParFFT.Offset = ParFFT.npt / 2;				// Cмещение следующей порции БПФ  
		ParFFT.nBlocks = ParFFT.Npt / ParFFT.Offset - 1;	// Число блоков усреднения
		ParFFT.dF = (float)ParFFT.Fs / (float)ParFFT.npt;			// Разрешение в спектре по частоте  
		ParFFT.Fmax = ParFFT.dF * ParFFT.np;		// Максимальная частота в спектре, Гц  

		if (ParFFT.nBlocks < 1)
		{
			free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
		}

		// Выделение динамической памяти под результат расчета спектра
		if (!(data = (double*)calloc(ParFFT.np, sizeof(double))))
		{
			free(gArray);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
		};

		// Расчет спектра по алгоритму ДБПФ
		if ((Kod_Program = DFTAnalysis(gArray, data, ParFFT, &Kod_LINE)) != 0)
		{
			free(gArray);	free(data);		*outCount = 2;		ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		};
		
		// 7.3 Расчет спектральных параметров ВА сигнала
		
		ParFFT.win_dF = 2;							// +- ширина полосы поиска гармоники, Гц

		// Расчет спектральных параметров по алгоритму ТВВ-320 Param
		if ((Kod_Program = Diagnostic_Parameters_TBB320(data, ParFFT, &ParSpec, MCM_step_garmonic, &Kod_LINE)) != 0)
		{
			free(gArray);	free(data);		free(MCM_data);		*outCount = 2;		ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		}
		else
		{
			if (strcmp(customParams, "SCADA-FFT2") == 0)
				MCM_data[0] = 7.0;
			else
				MCM_data[0] = 7.1;
			MCM_data[1] = devInfo.dataCount;
			MCM_data[2] = devInfo.rate;
			MCM_data[3] = devInfo.inputRange;
			MCM_data[4] = devInfo.sensor.sensitivity;

			MCM_data[5] = StatPar.Pogr_MX;
			MCM_data[6] = StatPar.Pogr_Disp;
			MCM_data[7] = StatPar.Pogr_CKO;
			MCM_data[8] = StatPar.Pogr_CKZ;
			MCM_data[9] = StatPar.Pogr_Ax;
			MCM_data[10] = StatPar.Pogr_Ex;

			MCM_data[11] = nmax;
			MCM_data[12] = nmin;

			if ((ParFFT.npt & (ParFFT.npt - 1)) == 0)  // Is power of 2
				MCM_data[13] = 0; // Fft_transformRadix2(real, imag, n);
			else  // More complicated algorithm for arbitrary sizes
				MCM_data[13] = 1; // Fft_transformBluestein(real, imag, n);

			MCM_data[14] = ParFFT.npt;
			MCM_data[15] = ParFFT.nBlocks;
			MCM_data[16] = ParFFT.Offset;
			MCM_data[17] = ParFFT.Fmax;
			MCM_data[18] = ParFFT.dF;

			MCM_data[19] = ParFFT.win_dF;
			MCM_data[20] = MCM_step_garmonic;

			MCM_data[21] = StatPar.Mx;
			MCM_data[22] = StatPar.CKO;
			MCM_data[23] = StatPar.CKZ;
			MCM_data[24] = StatPar.Ax;
			MCM_data[25] = StatPar.Ex;
			MCM_data[26] = StatPar.Ampl;
			MCM_data[27] = StatPar.Amax;

			MCM_data[28] = -1;
			MCM_data[29] = -1;
			MCM_data[30] = -1;
			MCM_data[31] = -1;
			MCM_data[32] = -1;
			MCM_data[33] = -1;
			MCM_data[34] = -1;

			MCM_data[35] = ParSpec.A100;
			MCM_data[36] = ParSpec.Agmax_155_185;
			MCM_data[37] = ParSpec.Fgmax_155_185;
			MCM_data[38] = ParSpec.A1_1450;
			MCM_data[39] = ParSpec.A2_2900;
			MCM_data[40] = ParSpec.A3_4350;
			MCM_data[41] = ParSpec.A4_5800;
			MCM_data[42] = ParSpec.A5_7250;
			MCM_data[43] = ParSpec.Agmax;
			MCM_data[44] = ParSpec.Fgmax;
			MCM_data[45] = sqrt(ParSpec.M_155_185);

			for (i = 0; i <= 15; i++)
			{
				MCM_data[46 + i] = sqrt(ParSpec.M[i]);
				MCM_data[62 + i] = sqrt(ParSpec.Mg[i]);
			}

			MCM_data[78] = sqrt(ParSpec.M_1_5000);
			MCM_data[79] = 0.0;	//код нормального завершения программы castomAlgo.c

			free(gArray); free(ret);
			return MCM_data;
		}
	}

	// -------------- 8. Пользовательский алгоритм "MIC-FFT" или "MIC-DFT"  ---------------------
	// Расчтет алгоритма БПФ или ДПФ исходного ВА сигнала
	// Расчет спетктральных параметров исходного ВА сигнала
	
	if (strcmp(customParams, "MIC-FFT") == 0 || strcmp(customParams, "MIC-DFT") == 0)
	{
		*outCount = MCM_MIC300;
		//	Выделение динамической памяти под массив спектральных параметров ВА сигнала (результат работы программы)
		if (!(MCM_data = (double*)malloc(sizeof(double) * MCM_MIC300)))
		{
			free(gArray);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
		};

		// 8.1. Расчет спектра с использованием алгоритма БПФ, ДПФ
		
		ParFFT.TypeSpec = 2;						// Тип рассчитываемого спектра 1 - СПМ, 2 - СМ, 3 - Амп (double)

		// Инициализация параметров алгоритма ПФ из параметров функции customAlgo(...)
		ParFFT.Fs = (uint32_t)devInfo.rate;			// Частота дискретизации сигнала виброускорения, Гц	

		if (strcmp(customParams, "MIC-FFT") == 0)
		{
			// Проверка достаточности точек временного ряда - должно быть больше числа точек БПФ 2^12 = 4096 
			i = 12;
			if ((ParFFT.Npt = (uint32_t)pow(2, i)) > devInfo.dataCount)
			{
				free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
			}

			//Автоопределение максимального числа значений временного ряда, используемых в алгоритме БПФ
			for (; (ParFFT.Npt = (uint32_t)pow(2, i)) < devInfo.dataCount; i++);

			ParFFT.Npt /= 2;				// Максимальное число значений временного ряда, кратных 2^n
			ParFFT.npt = ParFFT.Npt;		// Число точек БПФ = числу точек временного ряда, кратных 2^n (1 блок БПФ, разрешение в спектре по частоте df = Fs / npt - любое )
		}
		else	//---------- "MIC-DFT" -------------
		{

			ParFFT.Npt = devInfo.dataCount;		// Число точек временного ряда, участвующих в алгоритме ДПФ
			ParFFT.npt = ParFFT.Npt;			// Число точек ДПФ = числу точек временного ряда (1 блок ДПФ, разрешение в спектре по частоте df = Fs / npt - любое)
		};

		// Расчетные параметры спектра
		ParFFT.np = ParFFT.npt / 2;					// Число значений в спектре
		ParFFT.Offset = ParFFT.npt / 2;				// Cмещение следующей порции БПФ  
		ParFFT.nBlocks = ParFFT.Npt / ParFFT.Offset - 1;	// Число блоков усреднения
		ParFFT.dF = (float)ParFFT.Fs / (float)ParFFT.npt;	// Разрешение в спектре по частоте  
		ParFFT.Fmax = ParFFT.Fs / 2;				// Максимальная частота в спектре, Гц 
	
		if (ParFFT.nBlocks < 1)
		{
			free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
		}

		// Выделение динамической памяти под результат расчета спектра
		if (!(data = (double*)calloc(ParFFT.np, sizeof(double))))
		{
			free(gArray);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
		};

		// Расчет спектра по алгоритму ДБПФ
		if ((Kod_Program = DFTAnalysis(gArray, data, ParFFT, &Kod_LINE)) != 0)
		{
			free(gArray);	free(data);		*outCount = 2;		ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		};

		// 8.2 Расчет спектральных параметров исходного ВА сигнала
		
		ParFFT.win_dF = 2;							// +- ширина полосы поиска гармоники, Гц

		// Расчет спектральных параметров по алгоритму ТВВ-320 Param
		if ((Kod_Program = Diagnostic_Parameters_TBB320(data, ParFFT, &ParSpec, MCM_step_garmonic, &Kod_LINE)) != 0)
		{
			free(gArray);	free(data);		free(MCM_data);		*outCount = 2;		ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		}
		else
		{
			if (strcmp(customParams, "MIC-FFT") == 0)
				MCM_data[0] = 8.0;
			else
				MCM_data[0] = 8.1;
			MCM_data[1] = devInfo.dataCount;
			MCM_data[2] = devInfo.rate;
			MCM_data[3] = devInfo.inputRange;
			MCM_data[4] = devInfo.sensor.sensitivity;

			if ((ParFFT.npt & (ParFFT.npt - 1)) == 0)  // Is power of 2
				MCM_data[5] = 0; // Fft_transformRadix2(real, imag, n);
			else  // More complicated algorithm for arbitrary sizes
				MCM_data[5] = 1; // Fft_transformBluestein(real, imag, n);

			MCM_data[6] = ParFFT.npt;
			MCM_data[7] = ParFFT.nBlocks;
			MCM_data[8] = ParFFT.Offset;
			MCM_data[9] = ParFFT.Fmax;
			MCM_data[10] = ParFFT.dF;

			MCM_data[11] = ParFFT.win_dF;
			MCM_data[12] = MCM_step_garmonic;

			MCM_data[13] = ParSpec.A100;
			MCM_data[14] = ParSpec.Agmax_155_185;
			MCM_data[15] = ParSpec.Fgmax_155_185;
			MCM_data[16] = ParSpec.A1_1450;
			MCM_data[17] = ParSpec.A2_2900;
			MCM_data[18] = ParSpec.A3_4350;
			MCM_data[19] = ParSpec.A4_5800;
			MCM_data[20] = ParSpec.A5_7250;
			MCM_data[21] = ParSpec.Agmax;
			MCM_data[22] = ParSpec.Fgmax;
			MCM_data[23] = ParSpec.M_155_185;
			for (i = 0; i <= 7; i++)
			{
				MCM_data[24 + i] = ParSpec.M[i];
				MCM_data[32 + i] = ParSpec.Mg[i];
			}
			MCM_data[40] = ParSpec.M_1_5000;
			MCM_data[41] = 0.0;	//код нормального завершения программы castomAlgo.c

			free(gArray); free(ret);
			return MCM_data;
		}
	}

//------------------- Конец пользовательских алгоритмов -----------------------------

	free(gArray);
	COUNT = 2;
	*outCount = COUNT;
	Kod_LINE = __LINE__;
	ret[0] = 3; ret[1] = Kod_LINE;
	return ret;
} 
