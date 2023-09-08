#include "Obrab_signal.h"

// Программа реализация сценария ТВВ-320_Param_16кГц_БПФ_16384_8бл
// Реализует пользовательский алгоритм "MIC-FFT-ACHX" или "MIC-DFT-ACHX"
// 1. Расчет алгоритма БПФ или ДПФ исходного ВА сигнала
// 2. Расчет спектральных параметров исходного ВА сигнала
// *raw -  массив значений временного ряда
// FFT - структура параметров ДБПФ
// *outCount - число значений временного ряда *raw - на входе / число возвращаемых расчетных параметров - на выходе
double* TBB320_Param_16000_16384_8(double* raw, ParamFFT ParFFT, uint32_t* outCount)
{
	uint32_t	i;					// Текущий индекс массива данных
	uint16_t	Kod_Program = 0;	// Код возврата функций программы
	long		Kod_LINE = 0;		// Код ошибки выполнения функций программы

	// Определение и выделение динамической памяти под возвращаемый код ошибки выполения функции 
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
// -------------- 

	// Определение и выделение динамической памяти для дублирования значений временного ряда
	double* gArray = (double*)malloc(sizeof(double) * (*outCount));
	if (!gArray)
	{
		*outCount = 2;	ret[0] = 1.0;	ret[1] = __LINE__;	return ret;
	}

	double* data;		// массив результата расчета спектра

	uint16_t
		VAD_number_Param = 29,	//число возвращаемых расчетных значений  
		VAD_step_garmonic = 50;	// шаг оценки периодических гаромник спектра 50 или 100 Гц

	ParamAmplSpectr		ParSpec;	// Расчетные значений спектральных параметров ВА сигнала

#ifdef _WIN32 
	// Дублирование значений временного ряда *raw в массив *gArray
	for (i = 0; i < *outCount; i++)		gArray[i] = raw[i];
#endif 


	//Определение и выделение динамической памяти под массив результатов расчета спектральных параметров ВА сигнала (результаты работы программы)
	double* VAD_Param = (double*)malloc(sizeof(double) * VAD_number_Param);
	if (!VAD_Param)
	{
		free(VAD_Param);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
	};

	// 1. Расчет спектра с использованием алгоритма БПФ, ДПФ
	if (ParFFT.nBlocks < 1)
	{
		free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
	};

	// выделение дианимческой памяти под результат расчета спектра 
	if (!(data = (double*)calloc(ParFFT.np, sizeof(double))))
	{
		free(gArray);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
	};

	// расчет спектра по алгоритму ДБПФ
	if ((Kod_Program = DFTAnalysis(gArray, data, ParFFT, &Kod_LINE)) != 0)
	{
		free(gArray);	free(data);		*outCount = 2;		ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
	};

	// 9.2 Расчет спектральных параметров исходного ВА сигнала по алгоритму ТВВ-320 АЧХ
	if ((Kod_Program = Diagnostic_Parameters_TBB320(data, ParFFT, &ParSpec, VAD_step_garmonic, &Kod_LINE)) != 0)
	{
		free(gArray);	free(data);		free(VAD_Param);		*outCount = 2;		ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
	}
	else
	{
		VAD_Param[0] = ParSpec.A100;
		VAD_Param[1] = ParSpec.Agmax_155_185;
		VAD_Param[2] = ParSpec.Fgmax_155_185;
		VAD_Param[3] = ParSpec.A1_1450;
		VAD_Param[4] = ParSpec.A2_2900;
		VAD_Param[5] = ParSpec.A3_4350;
		VAD_Param[6] = ParSpec.A4_5800;
		VAD_Param[7] = ParSpec.A5_7250;
		VAD_Param[8] = ParSpec.Agmax;
		VAD_Param[9] = ParSpec.Fgmax;
		VAD_Param[10] = ParSpec.M_155_185;
		for (i = 0; i <= 7; i++)
		{
			VAD_Param[11 + i] = ParSpec.M[i];
			VAD_Param[19 + i] = ParSpec.Mg[i];
		}
		VAD_Param[27] = ParSpec.M_1_5000;
		VAD_Param[28] = 0.0;	//код нормального завершения программы castomAlgo.c

		free(gArray); free(ret);
		*outCount = VAD_number_Param;
		return VAD_Param;
	}
}

// Программа реализация сценария ТВВ-1000-2_АЧХ Param_32кГц_БПФ_262144_1бл
// Реализует пользовательский алгоритм "MIC-FFT-ACHX" или "MIC-DFT-ACHX"
// 1.Расчтет алгоритма БПФ или ДПФ в фрагменте исходного ВА сигнала
// 2. Расчет спектральных параметров фрагмента исходного ВА сигнала
// *raw -  массив значений временного ряда
// FFT - структура параметров ДБПФ
// *outCount - число значений временного ряда *raw - на входе, число возвращаемых расчетных параметров - на выходе
double* TBB1000_ACHX_Param_32000_262144_1(double * raw, ParamFFT ParFFT, uint32_t * outCount)
{
	uint32_t	i;					// Текущий индекс массива данных
	uint16_t	Kod_Program = 0;	// Код возврата функций программы
	long		Kod_LINE = 0;		// Код ошибки выполнения функций программы

	// Определение и выделение динамической памяти под возвращаемый код ошибки выполения функции 
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
// -------------- 

	// Определение и выделение динамической памяти для дублирования значений временного ряда
	double* gArray = (double*)malloc(sizeof(double) * (*outCount));
	if (!gArray)
	{
		*outCount = 2;	ret[0] = 1.0;	ret[1] = __LINE__;	return ret;
	}

	double* data;		// массив результата расчета спектра

	uint16_t
		MCM_MIC300_ACHX = 30,	//число возвращаемых расчетных значений  
		MCM_step_garmonic = 50;	// шаг оценки периодических гаромник спектра 50 или 100 Гц

	ParamAmplSpectr		ParSpec;	// Расчетные значений спектральных параметров ВА сигнала

#ifdef _WIN32 
	// Дублирование значений временного ряда *raw в массив *gArray
	for (i = 0; i < *outCount; i++)		gArray[i] = raw[i];
#endif 


	//Определение и выделение динамической памяти под массив результатов расчета спектральных параметров ВА сигнала (результаты работы программы)
	double* MCM_data = (double*)malloc(sizeof(double) * MCM_MIC300_ACHX);
	if (!MCM_data)
	{
		free(MCM_data);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
	};

	// 1. Расчет спектра с использованием алгоритма БПФ, ДПФ
	if (ParFFT.nBlocks < 1)
	{
		free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
	};

	// выделение динамической памяти под результат расчета спектра 
	if (!(data = (double*)calloc(ParFFT.np, sizeof(double))))
	{
		free(gArray);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
	};

	// расчет спектра по алгоритму ДБПФ
	if ((Kod_Program = DFTAnalysis(gArray, data, ParFFT, &Kod_LINE)) != 0)
	{
		free(gArray);	free(data);		*outCount = 2;		ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
	};


	// 9.2 Расчет спектральных параметров исходного ВА сигнала по алгоритму ТВВ-320 АЧХ
	if ((Kod_Program = Diagnostic_Parameters_VarFm_TBB320(data, ParFFT, &ParSpec, MCM_step_garmonic, &Kod_LINE)) != 0)
	{
		free(gArray);	free(data);		free(MCM_data);		*outCount = 2;		ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
	}
	else
	{
			MCM_data[0] = ParSpec.Fg_100;
			MCM_data[1] = ParSpec.A100;
			MCM_data[2] = ParSpec.Agmax_155_185;
			MCM_data[3] = ParSpec.Fgmax_155_185;
			MCM_data[4] = ParSpec.A1_1450;
			MCM_data[5] = ParSpec.A2_2900;
			MCM_data[6] = ParSpec.A3_4350;
			MCM_data[7] = ParSpec.A4_5800;
			MCM_data[8] = ParSpec.A5_7250;
			MCM_data[9] = ParSpec.Agmax;
			MCM_data[10] = ParSpec.Fgmax;
			MCM_data[11] = ParSpec.M_155_185;
			for (i = 0; i <= 7; i++)
			{
				MCM_data[12 + i] = ParSpec.M[i];
				MCM_data[20 + i] = ParSpec.Mg[i];
			}
			MCM_data[28] = ParSpec.M_1_5000;
			MCM_data[29] = 0.0;	//код нормального завершения программы castomAlgo.c

			free(gArray); free(ret);
			*outCount = MCM_MIC300_ACHX;
			return MCM_data;
		}
}


// Программа расчета Передаточной частотной характеристики системы H1(f) или H2(f)
// 1.Расчтет алгоритма БПФ или ДПФ в фрагменте исходного ВА сигнала
// 2. Расчет спектральных параметров фрагмента исходного ВА сигнала
// *F -  на входе - массив значений сигнала силы с числом значений *outCount / на выходе - амплитудный спектр сигнала силы с числом значений *outCount
// *X -  на входе - массив значений сигнала реакции с числом значений *outCount / на выходе - функция когерентности с числом значений *outCount
// ParFFT - структура параметров ДБПФ
// Type_H -  тип рассчитываемой характеристки АЧХ: 1 - H1(f), 2 - H2(f)
// *outCount - на входе - число значений сигнала *F, *X / на выходе число возвращаемых расчетных значений АЧХ, СПМ и функ-ции когерентности
// Возвращает массив значений АЧХ системы с числом значений *outCount
double* ACHX_function(double* F, double* X, ParamFFT ParFFT, int Type_H, uint32_t* outCount)
{
	double	* GFF,		// суммарная СПМ сигнала силы всех блоков
			* CFX;		// действительная часть суммарной СП взаимного спектра силы и реакции всех блоков

	double	* GXX,		// суммарная СПМ сигнала реакцции всех блоков
			* QFX;		// мнимая часть суммарной СП взаимного спектра силы и реакции всех блоков

	double	* ReF,		// действительная часть СП сигнала силы
			* ImF,		// мнимая часть СП сигнала силы
			* ReX,		// действительная часть СП сигнала реакции
			* ImX;		// мнимая часть СП сигнала реакции
	
	// Определение и выделение динамической памяти под возвращаемый код ошибки выполения функции 
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
// -------------- 

	// Проверка достаточности данных 
	if ((ParFFT.nBlocks < 1) || (ParFFT.npt > *outCount) )
	{
		*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
	};

	// Выделение динамической памяти для СПМ сигнала силы 
	if (!(GFF = (double*)calloc(ParFFT.np, sizeof(double))))
	{
		*outCount = 2;	ret[0] = 1.0;	ret[1] = __LINE__;	return ret;
	};

	// Выделение динамической памяти для СПМ сигнала реакции
	if (!(GXX = (double*)calloc(ParFFT.np, sizeof(double))))
	{
		*outCount = 2;	ret[0] = 1.0;	ret[1] = __LINE__;	return ret;
	};

	// Выделение динамической памяти для действительной части СП взаимного спектра сигнала силы и реакции
	if (!(CFX = (double*)calloc(ParFFT.np, sizeof(double))))
	{
		*outCount = 2;	ret[0] = 1.0;	ret[1] = __LINE__;	return ret;
	};
	// Выделение динамической памяти для мнимой части СП взаимного спектра сигнала силы и реакции
	if (!(QFX = (double*)calloc(ParFFT.np, sizeof(double))))
	{
		*outCount = 2;	ret[0] = 1.0;	ret[1] = __LINE__;	return ret;
	};

	// Выделение динамической памяти для Re части спектра силы
	if (!(ReF = (double*)calloc(ParFFT.npt, sizeof(double))))
	{
		*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
	};
	// Выделение динамической памяти для Im части спектра силы
	if (!(ImF = (double*)calloc(ParFFT.npt, sizeof(double))))
	{
		*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
	};
	// Выделение динамической памяти для Re части спектра реакции
	if (!(ReX = (double*)calloc(ParFFT.npt, sizeof(double))))
	{
		*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
	};
	// Выделение динамической памяти для Im части спектра реакции
	if (!(ImX = (double*)calloc(ParFFT.npt, sizeof(double))))
	{
		*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
	};

	//Определение и выделение динамической памяти под передаточную функцию H(f)
	double* H = (double*)calloc(ParFFT.np, sizeof(double));
	if (!H)
	{
		*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
	};

	// Расчет АЧХ системы  
	uint32_t	i,n,
			point,
			point_n;
	
	double SPM = 1.0 / (2.0 * pow(ParFFT.np, 2) * ParFFT.dF * ParFFT.nBlocks);		// Нормирующий множитель расчета СПМ
	double SM = 1.0 / (2.0 * pow(ParFFT.np, 2) * ParFFT.nBlocks);					// Нормирующий множитель расчета CМ


	for (i = 0; i < ParFFT.np; i++)		GFF[i] = GXX[i] = CFX[i] = QFX[i] = 0;

	// Расчет Комплексного спектра по числу блоков nBlocks временного ряда 
	for (point = 0, i = 1; i <= ParFFT.nBlocks; i++, point += ParFFT.Offset)
	{
		// Инициализация буферов Re, Im значениями сигнала силы *F 
		for (n = 0, point_n = point; (n < ParFFT.npt) && (point_n < ParFFT.Npt); n++, point_n++)
		{
			ReF[n] = F[point_n];	ImF[n] = 0.0;
			ReX[n] = X[point_n];	ImX[n] = 0.0;
		};
		// Расчет СП сигнала силы текущего блока
		if (!Fft_transform(ReF, ImF, ParFFT.npt))
		{
			*outCount = 2; ret[0] = 3; ret[1] = __LINE__; return ret;
		};
		// Расчет СП сигнала реакции текущего блока
		if (!Fft_transform(ReX, ImX, ParFFT.npt))
		{
			*outCount = 2; ret[0] = 3; ret[1] = __LINE__; return ret;
		};

		for (n = 0; n < ParFFT.np; n++)
		{
			GFF[n] += ReF[n] * ReF[n] + ImF[n] * ImF[n];
			GXX[n] += ReX[n] * ReX[n] + ImX[n] * ImX[n];
			
			CFX[n] += ReF[n] * ReX[n] + ImF[n] * ImX[n];
			QFX[n] += ImF[n] * ReX[n] - ImX[n] * ReF[n];
		};
	};
	for (n = 0; n < ParFFT.np; n++)
	{
		if (Type_H == 1)
		{
			H[n] = sqrt(CFX[n] * CFX[n] + QFX[n] * QFX[n]) / GFF[n];	// Передаточная функция H1-1
		}
		else
		{
			H[n] = GXX[n] / sqrt(CFX[n] * CFX[n] + QFX[n] * QFX[n]);	// Передаточная функция H2-1
		};
//		F[n] = GFF[n] * SM;			// Спектральная мощность сигнала силы
		F[n] = sqrt(GFF[n] * SM);	// Амплитудный спектр сигнала силы

		X[n] = (CFX[n] * CFX[n] + QFX[n] * QFX[n]) / (GFF[n] * GXX[n]);	// Функция когерентности
	};

	free(GFF); free(GXX); free(ReF); free(ReX); free(ImF); free(ImX);
	free(ret);
	*outCount = ParFFT.np;
	return H;
}


// Программа расчета параметров АЧХ калибровочного блока
// *H - массив значений АЧХ системы с числом значений *N_data
// *F - массив амплитудного спеткра сигнала ударной силы с числом значений *N_data
// N_data -  число значений cпектра АЧХ системы и спектра силы
// *Par -  указатель на переменную структуры хранения исходных данных и расчетных параметров АЧХ
uint16_t ACHX_param(double* H, double* F, uint32_t N_data, ParamACHX* Par)
{
	uint32_t	i,
				i_low,		//номер гармоники нижней чатоты рабочего диапазона АЧХ  
				i_high,		//номер гармоники верхней чатоты рабочего диапазона АЧХ  
				N_d;		//число гармоник АЧХ в рабочем диапазоне 
	float		Porog;		// пороговое значение уменьшения амплитуды гармоник амплитудного спектра ударной силы

	// Определение верхней частоты рабочего диапазона из амплитудного спектра сигнала ударной силы по критерию:
	// Уменьшение максимального значения Амплитудного спектра сигнал ударной силы на -20дБ или в 10 раз
	for (Porog = F[1] / 10.0, i = 1; (i < N_data) && (F[i] > Porog); i++);
	Par->Fh = Par->dF * i; // вержняя частота ударной силы
	
	// Уменьшение значения верхней частота стандартного рабочего диапазона 5кГц, если верхняя частота ударной силы меньше 5кГц
	if (Par->Fh < Par->F_high)	Par->F_high = Par->Fh;

	// определение номера гармоники нижней частоты рабочего диапазона
	i_low  = Par->F_low  / Par->dF;
	i_high = Par->F_high / Par->dF;
	N_d = i_high - i_low + 1;

	if (N_d <= 3)
	{
		__LINE__;
		return 4;
	}

	i = 159.15 / Par->dF;
	Par->Ag_160 = H[i];	//	 значение АЧХ на частоте 159,15 Гц

	Par->Mx = 0;		// математическое ожидание 
	Par->CKO = 0;		// СКО

	Par->Amax = 0;		// максимальное значение АЧХ
	Par->Amin = 1e10;	// минимальное значение АЧХ

	for (i = i_low; i < i_high; i++)
	{
		Par->Mx += H[i] / N_d;
		if (H[i] > Par->Amax)
		{
			Par->Amax = H[i];
			Par->F_Amax = i;
		}
		if (H[i] < Par->Amin)
		{
			Par->Amin = H[i];
			Par->F_Amin = i;
		};
	};
	
	Par->G_plus  = 20 * log10(Par->Amax/Par->Ag_160);
	Par->G_minus = 20 * log10(Par->Amin/Par->Ag_160);
	
	if (fabs(Par->G_plus) >= fabs(Par->G_minus))
		Par->G_max = fabs(Par->G_plus);
	else
		Par->G_max = fabs(Par->G_minus);

	for (i = i_low; i < i_high; i++)
	{
		Par->CKO += (H[i] - Par->Mx) * (H[i] - Par->Mx) / N_d;
	};

	Par->CKO = pow(Par->CKO, 0.5);

	return 0;
}


// Программа расчета статистических параметров ВА сигнала
// *row - массив данных исходного временного ряда
// N_data -  число значений временного ряда
// *Par -  указатель на переменную структуры хранения результатов оценки статистических параметров 
// *kod_line - указатель на номер строки ошибки программы
uint16_t statictica(const double* row, uint32_t N_data, ParamStatistic* Par, long* kod_line)
{

	if (N_data <= 3)
	{
		*kod_line = __LINE__;
		return 4;
	}

	uint32_t	i,
		N_d = N_data - 1;

	Par->Mx = 0;		// математическое ожидание
	Par->Disp = 0;		// дисперсия
	Par->CKO = 0;		// СКО
	Par->CKZ = 0;		// СКЗ
	Par->Ax = 0;		// ассиметрия
	Par->Ex = 0;		// эксцесс
	Par->Ampl = 0;		// амплитуда
	Par->Amax = 0;		// максимальное по модулю значение

	double Xmax = -100000.0;
	double Xmin = 100000.0;
	for (i = 0; i < N_data; i++)
	{
		Par->Mx += row[i] / N_data;
		if (row[i] > Xmax) Xmax = row[i];
		if (row[i] < Xmin) Xmin = row[i];
	};
	Par->Ampl = (Xmax - Xmin) / 2.0;

	Xmax = fabs(Xmax);
	Xmin = fabs(Xmin);
	if (Xmax > Xmin)
		Par->Amax = Xmax;
	else
		Par->Amax = Xmin;

	for (i = 0; i < N_data; i++)
	{
		Par->Disp += (row[i] - Par->Mx) * (row[i] - Par->Mx) / N_d;
		Par->CKZ += row[i] * row[i] / N_data;
	};

	Par->CKO = pow(Par->Disp, 0.5);
	Par->CKZ = pow(Par->CKZ, 0.5);

	for (i = 0; i < N_data; i++)
	{
		Par->Ax += pow(row[i] - Par->Mx, 3.0) / (N_data * pow(Par->CKO, 3));
		Par->Ex += pow(row[i] - Par->Mx, 4.0) / (N_data * pow(Par->CKO, 4));
	};
	// расчет коэффициентов погрешности стат. хар-к. 
	Par->Pogr_MX = Par->CKO * sqrt(1.0 / N_data);
	Par->Pogr_Disp = Par->Disp * sqrt(2.0 / N_data);
	Par->Pogr_CKO = Par->CKO * sqrt(1.0 / (2.0 * N_data));
	Par->Pogr_CKZ = Par->CKZ * sqrt(1.0 / (2.0 * N_data));
	Par->Pogr_Ax = sqrt(15.0 / N_data);
	Par->Pogr_Ex = sqrt(96.0 / N_data);
	return 0;
}


// Программа формирования разностного файла после фильтрации
// *row - массив данных исходного временного ряда
// *row_pf - массив результатов фильтрации временного ряда
// nmax - число точек большого временного окна
// nmin - число точек малого временного окна
// N_data -  число значений временного ряда
// *kod_line - указатель на номер строки ошибки программы
uint16_t Make_File_raznoct(const double* row, double* row_pf, uint32_t nmax, uint32_t nmin, uint32_t N_data, long* kod_line)
{
	uint32_t	p;
	uint16_t	prog; // возвращаемый код подпрограмм
	double* row_delta;	// внутрений массив промежутотчных значений 

	// Выделение динамической памяти под значения временного окна с начальным обнулением значений
	if ((row_delta = (double*)malloc(sizeof(double) * N_data)) == NULL)
	{
		*kod_line = __LINE__; return 1;
	}
	// Фильтрация ряда окном sing(x) с числом точек nmax
	if ((prog = Sing_Win(row, row_pf, nmax, N_data, kod_line)) != 0)
	{
		free(row_delta);	return prog;
	}
	else
	{
		// Инициализация значений внутреннего массива 
		for (p = 0; p < N_data; p++)
			row_delta[p] = row[p] - row_pf[p];
	}

	// Фильтрация ряда окном sing(x) с числом точек nmin
	if ((prog = Sing_Win(row_delta, row_pf, nmin, N_data, kod_line)) != 0)
	{
		free(row_delta);  return prog;
	}

	free(row_delta);
	*kod_line = 0;
	return 0;
}

// Программа фильтрации временного ряда окном sing(x) = sin(x) / x
// *v - указатель на массив временного ряда 
// *v_win - указатель на массив результатов фильтрации
// n_win - число точек симметричной части временного окна. Общее число точек временного окна 2*n_win+1 
// N_data - число точек временного ряда
// *kod_line - указатель на номер строки ошибки программы
uint16_t Sing_Win(const double* row, double* row_win, uint32_t n_win, uint32_t N_data, long* kod_line)
{
	double 	A,		// нормировочный коэффициент окна sing(х) 
		* sing,		// массив весовых коэффициентов временного окна sing(х)
		* Vsing,	// массив умножаемых на окно sing(x) значений временного ряда
		y;      	// отфильтрованное значение временного ряда

	uint32_t
		n_sing = 2 * n_win, // Верхний индекс массива коэффициентов окна sing(х)
		k, i,
		p, p1;

	// Выделение динамической памяти под значения к-тов весового окна
	if ((sing = (double*)malloc(sizeof(double) * (n_sing + 1))) == NULL)
	{
		*kod_line = __LINE__;  return 1;
	}
	// Выделение динамической памяти под значения участка временного ряда
	if ((Vsing = (double*)malloc(sizeof(double) * (n_sing + 1))) == NULL)
	{
		*kod_line = __LINE__;  return 1;
	}
	// Инициализация весовых коэффициентов временного окна sing(x)
	for (i = n_win, k = 0; i > 0; i--, k++)
	{
		sing[k] = sin(M_PI * (double)i / (double)n_win) / (M_PI * (double)i / (double)n_win);
		sing[n_sing - k] = sing[k];
	};
	sing[n_win] = 1;

	// Расчет нормировочного коэффицента весового окна sing(x)
	for (A = 0.0, i = 0; i <= n_sing; i++) A += sing[i];
	if (!A)
	{
		*kod_line = __LINE__;  return 5;
	}

	//-----------Инициализация массива Vsing[] для расчет row_win[0] - нулевого фильтрованного значения
	//Присвоение левым n_win+1 элементам массива Vsing[] нулевого значения временного ряда row[i]
	for (i = 0; i <= n_win; i++)	Vsing[i] = row[0];

	// Присвоение правым n_win элементам массива Vsing[] текущих значений временного ряда row[i]
	for (p = 1, i = n_win + 1; (p <= n_win) && (i <= n_sing); p++, i++)   Vsing[i] = row[p];

	//---------- Расчет нулевого фильтрованного значения временного ряда
	for (y = 0.0, i = 0; i <= n_sing; i++)  y += sing[i] * Vsing[i];
	row_win[0] = y / A;

	//---------	Большой цикл фильтрации временного ряда
	for (p = n_win + 1, p1 = 1; p < N_data; p++, p1++)
	{
		// сдвиг значений обрабатываемых данных на одно значение влево
		for (i = 0; i < n_sing; i++)  Vsing[i] = Vsing[i + 1];
		Vsing[n_sing] = row[p];	// доопределение последнего значения Vsing новым значеним временного ряда

		//---------- Расчет нового фильтрованного значения временного ряда
		for (y = 0.0, i = 0; i <= n_sing; i++)  y += sing[i] * Vsing[i];
		row_win[p1] = y / A;
	};

	//------ Фильтрация значений в конце временного ряда
	for (p--; p1 < N_data; p1++, p--)
	{
		// сдвиг значений обрабатываемых данных на одно значение влево
		for (i = 0; i < n_sing; i++)  Vsing[i] = Vsing[i + 1];
		Vsing[n_sing] = row[p];	// доопределение последнего значения Vsing последним значением временного ряда row[p]
		//---------- Расчет нового фильтрованного значения временного ряда
		for (y = 0.0, i = 0; i <= n_sing; i++)  y += sing[i] * Vsing[i];
		row_win[p1] = y / A;
	};

	free(sing);
	free(Vsing);
	*kod_line = 0;
	return 0;
}

// Программа расчета спектра ВА сигнала по алгоритму БПФ или ДПФ
// *gArr - указатель на массив временного ряда
// *data - указатель на массив одностороннего спектра 
// *Par - указатель на переменную структуры параметров ДБПФ 
// *kod_line - указатель на номер строки ошибки программы
uint16_t DFTAnalysis(double* gArr, double* data, ParamFFT Par, long* kod_line)
{
	uint32_t	n,
		i;
	uint32_t	point,
		point_n;

	double* Re;		// массив действительной части спектра
	double* Im;		// массив мнимой части спектра

	// Выделение динамической памяти для массива Re результатов расчета ПФ
	if (!(Re = (double*)malloc(sizeof(double) * Par.npt)))
	{
		*kod_line = __LINE__;  return 1;
	};

	// Выделение динамической памяти для массива Im результатов расчета ПФ
	if (!(Im = (double*)malloc(sizeof(double) * Par.npt)))
	{
		free(Re); *kod_line = __LINE__;  return 1;
	}

	// Расчет ПФ по числу блоков nBlocks временного ряда 
	for (point = 0, i = 1; i <= Par.nBlocks; i++, point += Par.Offset)
	{
		// Инициализация буферов Re, Im значениями временного ряда 
		for (n = 0, point_n = point; (n < Par.npt) && (point_n < Par.Npt); n++, point_n++)
		{
			Re[n] = gArr[point_n];	Im[n] = 0.0;
		}

		// Расчет алгоритма БПФ или БПФ для текущего блока
		if (!Fft_transform(Re, Im, Par.npt))
		{
			free(Re); free(Im); *kod_line = __LINE__;  return 2;
		}

		// Расчет среднего спектра типа Type_spec по блокам
		Estimate_Sym_Spec(data, Re, Im, Par.np, Par.dF, Par.nBlocks, Par.TypeSpec);
	};
	// Расчет Амплитудного спектра СКЗ значений гармоник
	if (Par.TypeSpec == 3)
		for (i = 0; i < Par.np; i++)	data[i] = sqrt(data[i]);

	free(Re);	free(Im);
	*kod_line = 0;
	return 0;
}


// Поблочное суммирование расчетных спектров
// *Re - указатель на массив расчетных значений спектра
// *Re - указатель на массив действительной части спектра
// *Im - указатель на массив мнимой части спектра
// np - число точек в спектре
// df - разрешение по частоте в спектре
// nBlocks -  блоков усреднения
// Type_spec - тип рассчитываемого спектра
void Estimate_Sym_Spec(double* Sp, double* Re, double* Im, uint32_t np, double df, uint16_t nBlocks, uint16_t Type_spec)
{
	unsigned long 	i;
	double SPM = 1.0 / (2.0 * pow(np, 2) * df * nBlocks);
	double SM = 1.0 / (2.0 * pow(np, 2) * nBlocks);

	switch (Type_spec)
	{
	case 1:
		// Расчет текущих значений спектральной плотности мощности СПМ с суммированием по блокам
		for (i = 0; i < np; i++)
			Sp[i] += (Re[i] * Re[i] + Im[i] * Im[i]) * SPM;
		break;
	case 2: case 3:
		// Расчет текущих значений спектра мощности СМ с суммированием по блокам
		for (i = 0; i < np; i++)
			Sp[i] += (Re[i] * Re[i] + Im[i] * Im[i]) * SM;
		break;
	}
}



// Программа расчета спектральных параметров ВА сигнала ТВВ-320-2 по спектру мощности
// *SpM - указатель на массив спектральной мощности сигнала
//  Par - значения параметров расчтета БПФ или ДПФ
// *Spec - указатель на заполняемую структуру расчетных спектральных параметров ВА сигнала 
//  step -  шаг кратных гармоник спектра 50 или 100 Гц

uint16_t Diagnostic_Parameters_TBB320(double* SpM, ParamFFT Par, ParamAmplSpectr* Spec, uint16_t step, long* kod_line)
{
	uint16_t	prog;	// возвращаемый код подпрограмм
	uint32_t	i,
				f;		// частота оцениваемой гармоники

	double	Mmax = 0,	// максимальное значение гармоники
		FMmax,		// Частота максиальной гармоники в спектре
		fmin,		// минимальная частота диапазона 
		fmax;		// максимальная частота диапазона 

	if (step != 50 && step != 100)
	{
		*kod_line = __LINE__;  return 3;
	};

	//-------- Оценка амплитуды на частоте 100 Гц -----------
	FMmax = 100;
	fmin = FMmax - Par.win_dF;
	fmax = FMmax + Par.win_dF;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->A100 = sqrt(Mmax);

	//-------- Поиск максимальной амплиуды в полосе [155; 185] Гц -----------
	fmin = 155.0;
	fmax = 185.0;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->Agmax_155_185 = sqrt(Mmax);
	Spec->Fgmax_155_185 = FMmax;

	//-------- Оценка амплитуды 1-й лопаточной частоты TBB-320-2 -----------
	FMmax = 1450;
	fmin = FMmax - Par.win_dF;
	fmax = FMmax + Par.win_dF;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->A1_1450 = Mmax;

	//-------- Оценка амплитуды 2-й лопаточной частоты TBB-320-2 -----------
	FMmax = 2900;
	fmin = FMmax - 2 * Par.win_dF;
	fmax = FMmax + 2 * Par.win_dF;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->A2_2900 = Mmax;

	//-------- Оценка амплитуды 3-й лопаточной частоты TBB-320-2 -----------
	FMmax = 4350;
	fmin = FMmax - 3 * Par.win_dF;
	fmax = FMmax + 3 * Par.win_dF;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->A3_4350 = Mmax;

	//-------- Оценка амплитуды 4-й лопаточной частоты TBB-320-2 -----------
	FMmax = 5800;
	fmin = FMmax - 4 * Par.win_dF;
	fmax = FMmax + 4 * Par.win_dF;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->A4_5800 = Mmax;

	//-------- Оценка амплитуды 5-й лопаточной частоты TBB-320-2 -----------
	FMmax = 7250;
	fmin = FMmax - 5 * Par.win_dF;
	fmax = FMmax + 5 * Par.win_dF;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->A5_7250 = Mmax;

	//-------- Поиск максимальной амплитуды в диапазонге [1; Fmax] Гц -----------
	fmin = 1.0;
	fmax = Par.Fmax;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->Agmax = sqrt(Mmax);
	Spec->Fgmax = FMmax;

	//-------- Оценка мощности сигнала в полосе [155; 185] Гц -----------
	fmin = 155.0;
	fmax = 185.0;
	if ((prog = Evaluation_Power_in_DiapazonSpectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->M_155_185 = Mmax;

	//-------- Оценка мощности сигнала в полосе [200; 1000) Гц -----------
	fmin = 200.0;
	fmax = 1000.0 - Par.dF;
	if ((prog = Evaluation_Power_in_DiapazonSpectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->M[0] = Mmax;

	//-------- Оценка мощности сигнала в 15 полосах частот по 1000 Гц -----------
	for (i = 1; i <= 15; i++)
	{
		Mmax = 0;
		fmin = i * 1000.0;
		fmax = (i + 1) * 1000.0 - Par.dF;
		if (fmax < Par.Fmax)
		{
			if ((prog = Evaluation_Power_in_DiapazonSpectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax)) != 0)
			{
				*kod_line = __LINE__; return prog;
			};
			switch (i)
			{
			case 1:
				Mmax -= Spec->A1_1450;	break;
			case 2:
				Mmax -= Spec->A2_2900;	break;
			case 4:
				Mmax -= Spec->A3_4350;	break;
			case 5:
				Mmax -= Spec->A4_5800;	break;
			case 7:
				Mmax -= Spec->A5_7250;	break;
			};
		}
		else
			Mmax = 0;

		if (Mmax < 0)
		{
			*kod_line = __LINE__; return 5;
		};
		Spec->M[i] = Mmax;
	};

	//-------- Оценка суммарной мощности гармоник спектра, кратных 50 Гц, в полосе [200; 1000) Гц -----------
	fmin = 200.0;
	fmax = 1000.0;
	Spec->Mg[0] = 0;
	for (f = 200; f < 1000; f += step)
	{
		fmin = f - Par.win_dF;
		fmax = f + Par.win_dF;
		if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
		{
			*kod_line = __LINE__; return prog;
		};
		Spec->Mg[0] += Mmax;
	}
	if (Spec->Mg[0] < 0)
	{
		*kod_line = __LINE__; return 5;
	};


	//-------- Оценка суммарной мощности гармоник спектра, кратных 50 Гц, в 15 полосах по 1000 Гц -----------
	for (i = 1; i <= 15; i++)
	{
		Spec->Mg[i] = 0;
		for (f = i * 1000; f < (i + 1) * 1000; f += step)
		{
			fmin = f - i * Par.win_dF;
			fmax = f + i * Par.win_dF;

			if (fmax < Par.Fmax)
			{
				// Расчет мощности гармоники максимальной амплитуды в полосе частот [fmin; fmax]
				if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
				{
					*kod_line = __LINE__; return prog;
				};
				Spec->Mg[i] += Mmax;
			}
			else
				Spec->Mg[i] = 0;
		}
		if (fmax < Par.Fmax)
		{
			switch (i)
			{
			case 1:
				if (step == 50)
					Spec->Mg[i] -= Spec->A1_1450;
				break;
			case 2:
				Spec->Mg[i] -= Spec->A2_2900;
				break;
			case 4:
				if (step == 50)
					Spec->Mg[i] -= Spec->A3_4350;
				break;
			case 5:
				Spec->Mg[i] -= Spec->A4_5800,2;	
				break;
			case 7:
				if (step == 50)
					Spec->Mg[i] -= Spec->A5_7250;
				break;
			};
		};
		if (Spec->Mg[i] < 0)
		{
			*kod_line = __LINE__; return 5;
		};
	}

	//-------- Evaluation Power signal in diapazone [1; 5000) GH frequency -----------
	Mmax = 0;
	fmin = 1.0;
	fmax = 5000.0 - Par.dF;
	if ((prog = Evaluation_Power_in_DiapazonSpectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->M_1_5000 = Mmax;

	Spec->A1_1450 = sqrt(Spec->A1_1450);
	Spec->A2_2900 = sqrt(Spec->A2_2900);
	Spec->A3_4350 = sqrt(Spec->A3_4350);
	Spec->A4_5800 = sqrt(Spec->A4_5800);
	Spec->A5_7250 = sqrt(Spec->A5_7250);
	return 0;
}



// Программа расчета диагностических спектральных параметров ВА сигнала с переменной частотой магнитных вибраций Fm ТГ ТВВ-320-2 по спектру мощности
// *SpM - указатель на массив спектральной мощности сигнала
//  Par - значения параметров расчтета БПФ или ДПФ
// *Spec - указатель на заполняемую структуру расчетных спектральных параметров ВА сигнала 
//  step -  шаг кратных гармоник спектра 50 или 100 Гц

uint16_t Diagnostic_Parameters_VarFm_TBB320(double* SpM, ParamFFT Par, ParamAmplSpectr* Spec, uint16_t step, long* kod_line)
{
	uint16_t	prog;	// возвращаемый код подпрограмм
	uint32_t	i;
	
	double
		f,			// частота оцениваемой гармоники
		step_fm,		// кратность гармоник в спектре по отношению к частоте магнитных вибраций статора (0,5 или 1)
		Mmax,		// максимальное значение гармоники
		FMmax,		// Частота максиальной гармоники в спектре
		fmin,		// минимальная частота диапазона 
		fmax;		// максимальная частота диапазона 

	if (step != 50 && step != 100)
	{
		*kod_line = __LINE__;  return 3;
	}
	else {
		step_fm = (double)step / 100.0;
	};


	//-------- Оценка частоты и амплитуды магнитной вибрации в полосе [80;110] Гц -----------
	Mmax = 0;
	FMmax = 100;
	fmin = 80;
	fmax = 110;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->Fg_100 = FMmax;
	fmin = FMmax - Par.win_dF; 
	fmax = FMmax + Par.win_dF;
	if ((prog = Evaluation_Power_in_DiapazonSpectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->A100 = sqrt(Mmax);

	//-------- Поиск максимальной амплитуды в полосе [155; 185] Гц -----------
	fmin = 155.0;
	fmax = 185.0;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->Fgmax_155_185 = FMmax;
	fmin = FMmax - Par.win_dF;
	fmax = FMmax + Par.win_dF;
	if ((prog = Evaluation_Power_in_DiapazonSpectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->Agmax_155_185 = sqrt(Mmax);

	//-------- Оценка частоты и амплитуды 1-й лопаточной частоты TBB-320-2 -----------
	FMmax = 14.5 * Spec->Fg_100;
	fmin = FMmax - Par.win_dF;
	fmax = FMmax + Par.win_dF;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	fmin = FMmax - Par.win_dF;
	fmax = FMmax + Par.win_dF;
	if ((prog = Evaluation_Power_in_DiapazonSpectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->A1_1450 = Mmax;

	//-------- Оценка амплитуды 2-й лопаточной частоты TBB-320-2 -----------
	FMmax = 29 * Spec->Fg_100;
	fmin = FMmax - 2 * Par.win_dF;
	fmax = FMmax + 2 * Par.win_dF;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	fmin = FMmax - Par.win_dF;
	fmax = FMmax + Par.win_dF;
	if ((prog = Evaluation_Power_in_DiapazonSpectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->A2_2900 = Mmax;

	//-------- Оценка амплитуды 3-й лопаточной частоты TBB-320-2 -----------
	FMmax = 43.5 * Spec->Fg_100;
	fmin = FMmax - 3 * Par.win_dF;
	fmax = FMmax + 3 * Par.win_dF;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};	
	fmin = FMmax - Par.win_dF;
	fmax = FMmax + Par.win_dF;
	if ((prog = Evaluation_Power_in_DiapazonSpectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->A3_4350 = Mmax;

	//-------- Оценка амплитуды 4-й лопаточной частоты TBB-320-2 -----------
	FMmax = 58 * Spec->Fg_100;
	fmin = FMmax - 4 * Par.win_dF;
	fmax = FMmax + 4 * Par.win_dF;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	fmin = FMmax - Par.win_dF;
	fmax = FMmax + Par.win_dF;
	if ((prog = Evaluation_Power_in_DiapazonSpectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->A4_5800 = Mmax;

	//-------- Оценка амплитуды 5-й лопаточной частоты TBB-320-2 -----------
	FMmax = 72.5 * Spec->Fg_100;
	fmin = FMmax - 5 * Par.win_dF;
	fmax = FMmax + 5 * Par.win_dF;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	fmin = FMmax - Par.win_dF;
	fmax = FMmax + Par.win_dF;
	if ((prog = Evaluation_Power_in_DiapazonSpectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->A5_7250 = Mmax;

	//-------- Поиск максимальной амплитуды в диапазоне [1; Fmax] Гц -----------
	fmin = 1.0;
	fmax = Par.Fmax;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->Fgmax = FMmax;
	fmin = FMmax - Par.win_dF;
	fmax = FMmax + Par.win_dF;
	if ((prog = Evaluation_Power_in_DiapazonSpectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->Agmax = sqrt(Mmax);

	//-------- Оценка мощности сигнала в полосе [155; 185] Гц -----------
	fmin = 155.0;
	fmax = 185.0;
	if ((prog = Evaluation_Power_in_DiapazonSpectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->M_155_185 = Mmax;

	//-------- Оценка мощности сигнала в полосе [2fm; 10fm) Гц -----------
	fmin = 2 * Spec->Fg_100;
	fmax = 10 * Spec->Fg_100 - Par.dF;
	if ((prog = Evaluation_Power_in_DiapazonSpectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->M[0] = Mmax;

	//-------- Оценка мощности сигнала в 15 полосах частот по 1000 Гц -----------
	for (i = 1; i <= 15; i++)
	{
		Mmax = 0;
		fmin = i * 10 * Spec->Fg_100;
		fmax = (i + 1) * 10 * Spec->Fg_100 - Par.dF;
		if (fmax < Par.Fmax)
		{
			if ((prog = Evaluation_Power_in_DiapazonSpectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax)) != 0)
			{
				*kod_line = __LINE__; return prog;
			};
			switch (i)
			{
			case 1:
				Mmax -= Spec->A1_1450;	break;
			case 2:
				Mmax -= Spec->A2_2900;	break;
			case 4:
				Mmax -= Spec->A3_4350;	break;
			case 5:
				Mmax -= Spec->A4_5800;	break;
			case 7:
				Mmax -= Spec->A5_7250;	break;
			};
		}
		else
			Mmax = 0;

		if (Mmax < 0)
		{
			*kod_line = __LINE__; return 5;
		};
		Spec->M[i] = Mmax;
	};

	//-------- Оценка суммарной мощности гармоник спектра, кратных fm или fm/2 Гц, в полосе [2fm; 10fm) Гц -----------
	Spec->Mg[0] = 0;
	for (f = 2; f < 10; f += step_fm)
	{
		fmin = f * Spec->Fg_100 - Par.win_dF;
		fmax = f * Spec->Fg_100 + Par.win_dF;
		if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
		{
			*kod_line = __LINE__; return prog;
		};
		fmin = FMmax - Par.win_dF;
		fmax = FMmax + Par.win_dF;
		if ((prog = Evaluation_Power_in_DiapazonSpectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax)) != 0)
		{
			*kod_line = __LINE__; return prog;
		};
		Spec->Mg[0] += Mmax;
	}
	if (Spec->Mg[0] < 0)
	{
		*kod_line = __LINE__; return 5;
	};


	//-------- Оценка суммарной мощности гармоник спектра, кратных 50 Гц, в 15 полосах по 1000 Гц -----------
	for (i = 1; i <= 15; i++)
	{
		Spec->Mg[i] = 0;
		for (f = i * 10; f < (i + 1) * 10; f += step_fm)
		{
			fmin = f * Spec->Fg_100 - i * Par.win_dF;
			fmax = f * Spec->Fg_100 + i * Par.win_dF;
			if (fmax < Par.Fmax)
			{
				// Расчет мощности гармоники максимальной амплитуды в полосе частот [fmin; fmax]
				if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
				{
					*kod_line = __LINE__; return prog;
				};
				fmin = FMmax - Par.win_dF;
				fmax = FMmax + Par.win_dF;
				if ((prog = Evaluation_Power_in_DiapazonSpectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax)) != 0)
				{
					*kod_line = __LINE__; return prog;
				};

				Spec->Mg[i] += Mmax;
			}
			else
				Spec->Mg[i] = 0;
		}
		if (fmax < Par.Fmax)
		{
			switch (i)
			{
			case 1:
				if (step == 50)
					Spec->Mg[i] -= Spec->A1_1450;
				break;
			case 2:
				Spec->Mg[i] -= Spec->A2_2900;
				break;
			case 4:
				if (step == 50)
					Spec->Mg[i] -= Spec->A3_4350;
				break;
			case 5:
				Spec->Mg[i] -= Spec->A4_5800, 2;
				break;
			case 7:
				if (step == 50)
					Spec->Mg[i] -= Spec->A5_7250;
				break;
			};
		};
		if (Spec->Mg[i] < 0)
		{
			*kod_line = __LINE__; return 5;
		};
	}

	//-------- Расчет мощности сигнала в полосе [1; 5000) Гц -----------
	Mmax = 0;
	fmin = 1.0;
	fmax = 5000.0 - Par.dF;
	if ((prog = Evaluation_Power_in_DiapazonSpectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->M_1_5000 = Mmax;

	Spec->A1_1450 = sqrt(Spec->A1_1450);
	Spec->A2_2900 = sqrt(Spec->A2_2900);
	Spec->A3_4350 = sqrt(Spec->A3_4350);
	Spec->A4_5800 = sqrt(Spec->A4_5800);
	Spec->A5_7250 = sqrt(Spec->A5_7250);
	return 0;
}





// Программа оценки максиальной амлитуды и частоты гармоники в заданном диапазоне частот в спектре мощности
//*SpM -  указатель на массив спектральной мощности сигнала виброускорения
// np - число точек в спектре
// dF - разрешение по частоте в спектре
// Flow - нижняя частота диапазона частот
// Fhigh - верхняя частота диапазона частот
// *Mmax -  указатель на максимальное значение мощности гармоники
// *FMmax - указатель на частоту максимальной гармоники
uint16_t Evaluation_Ampl_Freq_in_Spectr(double* SpM, int np, double dF, double Flow, double Fhigh, double* Mmax, double* FMmax)
{
	long
		k,			// номер гармоники в спектре
		n_start,	// начальный номер гармоники в спектре
		n_end;		// конечный номер гармоники в спектре

	n_start = (long)(Flow / dF);
	n_end = (long)(Fhigh / dF);
	if ((n_start > np) || (n_end > np))
	{
		return 3;
	}

	*Mmax = 0;
	*FMmax = 0;
	for (k = n_start; k <= n_end; k++)
		if (SpM[k] > *Mmax)
		{
			*Mmax = SpM[k];
			*FMmax = k;
		}
	*FMmax *= dF;
	if (*Mmax < 0)
	{
		return 5;
	}
	else
	{
		return 0;
	};
}

// Программа оценки мощности сигнала в заданном диапазоне частот по спектру мощности
// *SpM - указатель на массив спектральной мощности сигнала виброускорения
// np - число точек в спектре
// dF - разрешение по частоте в спектре
// Flow - нижняя частота диапазона частот
// Fhigh - верхняя частота диапазона частот
// *M - указатель на сумарное значение мощности сигнала в заданном диапазоне частот
uint16_t Evaluation_Power_in_DiapazonSpectr(double* SpM, int np, double dF, double Flow, double Fhigh, double* M)
{
	long	k,		// номер гармоники в спектре
		n_start,	// начальный номер гармоники в спектре
		n_end;		// конечный номер гармоники в спектре

	n_start = (long)(Flow / dF);
	n_end = (long)(Fhigh / dF);
	if ((n_start > np) || (n_end > np)) return 3;
	*M = 0;
	for (k = n_start; k <= n_end; k++)
		*M += SpM[k];
	if (*M < 0)
	{
		return 5;
	}
	else
	{
		return 0;
	};
}
