#pragma once
#include <stdlib.h>
//#include <stdbool.h>
//#include <stddef.h>
#include <stdint.h>

#include "fft-real-pair.h"
#include "math.h"

#ifdef _WIN32 
#include <corecrt_math_defines.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// структура параметров статистики временного ряда
typedef struct
{
	double 	Mx;			// среднее значение
	double	Disp;		// дисперсия
	double	CKO;		// среднеквадратическое отклонение (за вычетом среднего значения)
	double	CKZ;		// среднеквадратическое значение (без вычета среднего значения
	double	Ax;			// ассиметрия
	double	Ex;			// эксцесс
	double  Ampl;       // амплитуда
	double  Amax;       // максимальное по модулю значение
	double	Pogr_MX;	// погрешность математического ожидания
	double	Pogr_Disp;  // погрешность дисперсии
	double	Pogr_CKO;   // погрешность скo
	double	Pogr_CKZ;   // погрешность скз
	double	Pogr_Ax;    // погрешность ассиметрии
	double	Pogr_Ex;    // погрешность эксцесса
}ParamStatistic;

// структура параметров статистики АЧХ системы
typedef struct
{
	float      dF;	    // Разрешение по частоте в спектре АЧХ, Гц
	float      Fmax;	// Верхняя частота спектра АЧХ, Гц

	float      Fh;		// Верхняя частота ударной силы, Гц
	
	float      F_low;	// Нижняя частота рабочего диапазона АЧХ, Гц
	float      F_high;	// Вержняя частота рабочего диапазона АЧХ, Гц

	float 	Mx;			// среднее значение АЧХ в рабочем диапазоне
	float	CKO;		// среднеквадратическое отклонение (за вычетом среднего значения) АЧХ в рабочем диапазоне
	float	Amax;		// максимальное значение АЧХ в рабочем диапазоне
	float	F_Amax;		// частота максимального значения АЧХ в рабочем диапазоне
	float   Amin;       // минимальное значение АЧХ в рабочем диапазоне
	float	F_Amin;		// частота минимального значения АЧХ в рабочем диапазоне
	float	Ag_160;		// значение АЧХ на базовой частоте калибровки 159,15 Гц

	float	G_plus;		// положительная неравномерность АЧХ в рабочем диапазоне, дБ
	float	G_minus;	// отрицательная неравномерность АЧХ в рабочем диапазоне, дБ
	float	G_max;		// максимальная по модулю неравномерность АЧХ в рабочем диапазоне, дБ

}ParamACHX;

// структура параметров амплитудного спектра
typedef struct
{
	double 	Fg_100;         // чатота магнитных вибраций статора, сноминальной частотой 100Гц
	double 	A100;           // СКЗ гармоники 100Гц

	double	Agmax_155_185;	// СКЗ гармоники максимальной амплитуды в полосе [155; 185] Гц
	double	Fgmax_155_185;	// Частота гаромники максимальной амплитуды в полосе [155; 185] Гц
	double	M_155_185;		// Мощность сигнала в полосе [155; 185] Гц

	double	A1_1450;		// СКЗ гармоники 1-й лопаточной частоты
	double	A2_2900;		// СКЗ гармоники 2-й лопаточной частоты
	double	A3_4350;		// СКЗ гармоники 3-й лопаточной частоты
	double  A4_5800;		// СКЗ гармоники 4-й лопаточной частоты
	double  A5_7250;		// СКЗ гармоники 5-й лопаточной частоты

	double	Agmax;			// СКЗ гармоники максимальной амплитуды в полосе [1; Fmax] Гц
	double	Fgmax;			// Частота гармоники максимальной амплитуды в полосе [1; Fmax] Гц

	double  M[16];			// Мощность сигнала в 16 полосах по 1кГц в диапазоне от 200Гц до 16кГц
	double  Mg[16];			// Мощность гармоник кратных 50Гц в 16 полосах по 1кГц в диапазоне от 200Гц до 16кГц

	double	M_1_5000;		// Мощность сигнала в полосе [1; 5000] Гц
}ParamAmplSpectr;

// Структура параметров расчета БПФ
typedef struct
{
	// Параметры, устанавленные в МСМ-204
	uint32_t	Npt;	    // Число значений временного ряда
	uint32_t     Fs;	        // Частота дисктеризации, Гц

	// Параметры инициализации алгоритмов ДПФ и БПФ
	uint32_t	n;			// 2^n = минимальное число точек в алгоритме БПФ 
	uint32_t    npt;	    // Число точек ДПФ или БПФ
	uint32_t    np;	        // Число значений в спектре
	uint32_t    Offset;	    // Число значений смещения следующей порции БПФ  
	uint16_t    TypeSpec;	// Тип рассчитываемого спектра 1 - СПМ, 2 - СМ, 3 - Амп (double)
	double      win_dF;	    // полоса поиска гармоник спектра +-win_dF Гц

	// Расчитываемые параметры
	double      dF;	        // Разрешение в спектре по частоте
	double      Fmax;	    // Максимальная частота в спектре, Гц
	uint16_t	nBlocks;	// Число независимых блоков усреднения
}ParamFFT;

// Программа реализация сценария ТВВ-320_Param_16кГц_БПФ_16384_8бл
double* TBB320_Param_16000_16384_8(double* raw, ParamFFT ParFFT, uint32_t* outCount);

// Программа реализация сценария ТВВ-1000-2_Param_32кГц_БПФ_262144_1бл
double* TBB1000_ACHX_Param_32000_262144_1(double* raw, ParamFFT ParFFT, uint32_t* outCount);

// Программа расчета Передаточной частотной характеристики системы
double* ACHX_function(double *a, double *b, ParamFFT ParFFT, int Type_H, uint32_t* outCount);

// Программа расчета параметров АЧХ калибровочного блока
uint16_t ACHX_param(double* H, double* F, uint32_t N_data, ParamACHX *Par);

// Расчет статистических параметров временного ряда
uint16_t statictica(const double* row, uint32_t N_data, ParamStatistic* Par, long* kod_line);

//  фильтрация временным окном Sing()
uint16_t Sing_Win(const double* row, double* row_win, uint32_t n_win, uint32_t N_data, long* kod_line);

//  Удаление тренда и расчет статистики фильтрованного временного ряда
uint16_t Make_File_raznoct(const double* row, double* row_pf, uint32_t nmax, uint32_t nmin, uint32_t N_data, long* kod_line);

// Программа расчета БПФ и ДПФ
uint16_t DFTAnalysis(double* gArr, double* data, ParamFFT Par, long* kod_line);

// Поблочное суммирование расчетных спектров 
void Estimate_Sym_Spec(double* Sp, double* Re, double* Im, uint32_t np, double df, uint16_t nBlocks, uint16_t Type_spec);

// Программа оценки максиальной амлитуды и частоты гармоники в заданном диапазоне частот в спектре мощности
uint16_t Evaluation_Ampl_Freq_in_Spectr(double* SpM, int np, double dF, double Flow, double Fhigh, double* MAmax, double* FMmax);

// Программа оценки мощности сигнала в заданном диапазоне частот по спектру мощности
uint16_t Evaluation_Power_in_DiapazonSpectr(double* SpM, int np, double dF, double Flow, double Fhigh, double* M);

// Программа расчета диагностических спектральных параметров ВА сигнала ТВВ-320-2 по спектру мощности в стационарном режиме вращения вала
uint16_t Diagnostic_Parameters_TBB320(double* SpM, ParamFFT Par, ParamAmplSpectr* Spec, uint16_t step, long* kod_line);

// Программа расчета диагностических спектральных параметров ВА сигнала ТВВ-320-2 по спектру мощности в нестационарном режиме вращения вала
uint16_t Diagnostic_Parameters_VarFm_TBB320(double* SpM, ParamFFT Par, ParamAmplSpectr* Spec, uint16_t step, long* kod_line);



#ifdef __cplusplus
}
#endif