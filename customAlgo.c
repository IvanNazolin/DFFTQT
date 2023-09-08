/*��������� castoAlgo.c - ��������� ��������� ������ ������������������ ������� �� 8-�� ���������������� ����������

* ������������� ��� ���������� � Linux � ����������� ��������� ������ castoAlgo.so � ������ ���-204

* � ������ MCM-204 ��� ������ ��������� ��������� � ���� "Data Type" ���������� Castomization � ���� "�astomization Parameter" - ��������� ��� ���������, �������� Monitoring
* � ��������� castoAlgo.c ��������� ��� �������� ���������� � ���������� customParams
* 1. "St�t" - ������ �������������� ���������� �������
* 2. "Sing" - �� ���������� ���������� ���� ����� sing() c ������ ����� nmax
* 3. "Trend" - ��������� ���������� ���������� ���� ����� ������ sing() c ������ ����� nmax � nmin ��� �������� ������
* 4. "FFT" ��� "DFT" -  ������ ������� �� ��������� ��� (���. ����-�����) � ����� ����� Npt = 2^n <= DataCount, ��� ��� ����������� �������������� ����� (�������� ���������) � ������ ����� Npt=DataCount 
* 5. "TrendFFT" ��� "TrendDFT" - ���������������� ���������� ���������� �.3 + �.4.
* 6. "SCADA_FFT1" ��� "SCADA_DFT1" - ������-� ��������� �����. (1 + 3 + 1 + 4 + Param) � �������� ��������� (��������������� � �������������) ����. �����.(14) � ������. �����.(45) ��� SCADA (����� 80)
* 7. "SCADA_FFT2" ��� "SCADA_DFT2" - ������-� ��������� �����. (1 + 4 + Param) � �������� ��������� (���������������) ����. �����. (7) � ������������ �����. (45 ��������) ��� SCADA (����� 80)
* 8. "MIC-FFT" ��� "MIC-DFT" - ���������������� ��������� ���������� (4 + Param) � �������� ������������ ���������� (28 ����������) ��� ��������� ��� (������ ��������� MIC-300) (����� 42)

* ����������� ��� � ���. 
* 1. ��������� ��� � ��� ����� �� ��������� ��������� Copyright(c) 2020 Project Nayuki. (MIT License)
* https://www.nayuki.io/page/free-small-fft-in-multiple-languages
* 2. ����� ����� ��� Npt = 2^n <= DataCountFFT (��������� ������� ��������).
* 3. ����� ����� ��� Npt = DataCountFFT ����� ����� ���������� ����.
* 4. ����� ���� ����������� �� ���� ���������� ��� � ��� = 1.
* 5. ����������� � ������� �� ������� ������������ ������ �������� ���������. �����������  - ���. 
* 6. �������� ��� � ������� �� ��� ��������� ������������ ������ �� ������������� ����� �����, �� ����������� ������ 2^n.
*
* ���������� � ��������� ����� �� ��������� ����������� ��� ��� "������", ����������� ������� �.�. 
* ���� �������� 12.07.2023�. ver.3.0 linix
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
	uint32_t	COUNT;				// ����� ��������, ������������ ���������� castomAlgo(...) � MCM-204
	uint32_t	i;					// ������� ������ ������� ������
	uint16_t	Kod_Program = 0;	// ������������ �������� ��������
	long		Kod_LINE = 0;		// ��� ������ ���������� ���������
	
	// ����������� � ��������� ������������ ������ ��� ������������ ��� ������ �/� 
	double* ret = (double*)malloc(sizeof(double) * 2);	
	ret[0] = 0.0;	//���������� ���������� ���������
	ret[1] = 0.0;	//���������� ���������� ���������
	
	// ������������ ���� ������ ret[0]
	// 1.0 - ������ ��������� ������������ ������ 
	// 2.0 - ������ �������������� ��������� ������
	// 3.0 - ������ ������� ���������� 
	// 4.0 - ������ ������������ ������ ��� �������������� ���������
	// 5.0 - ������. ����������� ���. �������� (������� �� ����, ������ �� �������������� �����)
	
	// ������������ ���� ������ ret[1]
 	// xxx.0 - ����� ������ ��������� ������������� ������

	// ����� ������������ �������� �/� =  ����� ����� ���������� ���� � ���������
	COUNT = devInfo.dataCount;

	// ����������� � ��������� ������������ ������ ��� ������������ (�������������) �������� ���������� ����
	double* gArray = (double*)malloc(sizeof(double) * (COUNT));
	if (!gArray)
	{
		*outCount = 2;	ret[0] = 1.0;	ret[1] = __LINE__;	return ret;
	}

#ifdef __linux__ 
	uint32_t* raw = (uint32_t*)rawData;			// ��������� �� ������ ��������� ������ rawData ���� �������

	raw = raw + chIndex * devInfo.dataCount;	// ��������� �� ������ ������� ��������� ������ ������ chIndex 

	double inputRange = devInfo.inputRange == B10 ? 10.0 : 1.25;
	double scalingFactor = inputRange/8388607.0*1000.0/devInfo.sensor.sensitivity;//for convert rawData to g
	
	// ������������� �������� rawData � ������� � ������ gArray �������� ��������� ����������� (G) 
	for(i = 0; i < COUNT; i++)
        gArray[i] = scalingFactor*(((raw[i] & 0x00800000) == 0x00800000) ? (int32_t)(raw[i] | 0xFF000000) : (int32_t)raw[i]);
#endif 

	//--------------- ��������� ������������ -------
	double
		*data,		// ������ ����������� ������� �� ���������� 1...5
		*MCM_data;	// ������ ����������� ������� �� ����������� 6, 7, 8, 9
	
	uint16_t
		nmax = 300,	// ����� ����� �������� ���� sing(x)
		nmin = 1,	// ����� ����� ������ ���� sing(x)
		MCM_SCADA = 80,			//����� ������������ �������� �������� castomAlgo(...) �� ���������� 6(������) � 7(�����������) ��� SCADA ����� 
		MCM_MIC300 = 42,		//����� ������������ �������� �������� castomAlgo(...) �� ��������y 8 (��� ��������� ���, ������ ����� ��������� ������ MIC-300�) 
		MCM_MIC300_ACHX = 43,	//����� ������������ �������� �������� castomAlgo(...) �� ��������y 9 (��� ��������� ���, ������ ����� ��������� ������ MIC-300�-���) 
		MCM_step_garmonic = 50;	// ��� ������ ������������� �������� ������� 50 ��� 100 ��
	
	ParamStatistic		StatPar,	// ���������� ���������� ��������� ������ �������������� ���������� �� ������� �� ���������
						StatPar_;	// ���������� ���������� ��������� ������ �������������� ���������� �� ������� ����� ���������
	ParamFFT			ParFFT;		// ���������� ���������� ��������� ���������� ������� ��� ��� ���
	ParamAmplSpectr		ParSpec;	// ���������� ���������� ��������� ��������� �������� ������������ ���������� �� �������

#ifdef _WIN32 
	// ������������ �������� ���������� ���� *raw � ������ *gArray
	for (i = 0; i < COUNT; i++)		gArray[i] = raw[i];
#endif 


	// -------------- 1. ���������������� �������� "Stat" ---------------------
	// ������ �������������� ���������� ��������� ���������� �� �������
	
	if (strcmp(customParams, "Stat") == 0)
	{
		// ����� ������������ ��������� �������� �� ��������� "Stat"
		COUNT = 14;
		*outCount = COUNT;

		// ��������� ������������ ������ ��� ���������� ������� �� ��������� "Stat"
		if ( !(data = (double*)malloc(sizeof(double) * COUNT)) )
		{
			free(gArray);	*outCount = 2;	ret[0] = 1.0;	ret[1] = __LINE__;	return ret;
		};

		// ������ �������������� ������������� �� �������
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

	// -------------- 2. ���������������� �������� "Sing" ---------------------	
	// ���������� ���������� �� �������� ��������� ����� Sing

	if (strcmp(customParams, "Sing") == 0)
	{
		// ����� ������������ ��������� �������� �� ��������� "Sing" = ����� �������� ���������� ����
		COUNT = devInfo.dataCount;
		*outCount = COUNT;

		// ��������� ������������ ������ ��� ���������� ������� �� ��������� "Sing"
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

	// -------------- 3. ���������������� �������� "Trend" ---------------------
	// �������� ������ �� ������� � ������� ���� ��������� ���� Sing � ������� � ����� ������ �����

	if (strcmp(customParams, "Trend") == 0)
	{
		// ����� ������������ ��������� �������� �� ��������� "Trend" = ����� �������� ���������� ����
		COUNT = devInfo.dataCount;
		*outCount = COUNT;
		
		// ��������� ������������ ������ ��� ���������� ������� �� ��������� "Trend"
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
	
	// -------------- 4. ���������������� �������� "FFT" ��� "DFT" ---------------------
	// ������ ������� �� ������� �� ��������� ��� ��� ���

	if (strcmp(customParams, "FFT") == 0 || strcmp(customParams, "DFT") == 0)
	{
		// ������������� ���������� ��������� ��� ��� ��� �� ���������� ������� customAlgo(...)
		ParFFT.TypeSpec = 3;						// ��� ��������������� ������� 1 - ���, 2 - ��, 3 - ��� (double)
		ParFFT.Fs = (uint32_t)devInfo.rate;			// ������� ������������� ������� ��������������, ��	

		if (strcmp(customParams, "FFT") == 0)
		{
			ParFFT.n = 8;		// ����������� ����� ����� ��� 2^8 = 256
			// �������� ������������� ����� ���������� ���� - ������ ���� ������ ����� ����� ��� 2^n = 256 
			if ((ParFFT.Npt = (uint32_t)pow(2, ParFFT.n)) > devInfo.dataCount)
			{
				free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
			};

			//�������������� ����������� ����� ����� ���������� ���� ��� ��������� ���
			for (; (ParFFT.Npt = (uint32_t)pow(2, ParFFT.n)) < devInfo.dataCount; ParFFT.n++);

			ParFFT.Npt /= 2;				// ������������ ����� ����� ���������� ����, ������� 2^n
			ParFFT.npt = ParFFT.Npt;		// ����� ����� ������� ��� = ����� ����� ���������� ����, ������� 2^n (1 ���� ���, ���������� � ������� �� ������� df = Fs / npt - ����� )
		}
		else	//---------- "DFT" -------------
		{
			ParFFT.Npt = devInfo.dataCount;		// ����� ����� ���������� ����, ����������� � ��������� ���
			ParFFT.npt = ParFFT.Npt;			// ����� ����� ��� = ����� ����� ���������� ���� (1 ���� ���, ���������� � ������� �� ������� df = Fs / npt - �����)
		};

		// ��������� ��������� �������
		ParFFT.np = ParFFT.npt / 2;					// ����� �������� � �������
		ParFFT.Offset = ParFFT.npt / 2;				// C������� ��������� ������ ���  
		ParFFT.nBlocks = ParFFT.Npt / ParFFT.Offset - 1;	// ����� ������ ����������
		ParFFT.dF = (float)ParFFT.Fs / (float)ParFFT.npt;			// ���������� � ������� �� �������  
		ParFFT.Fmax = ParFFT.dF * ParFFT.np;		// ������������ ������� � �������, ��  

		if (ParFFT.nBlocks < 1)
		{
			free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
		}

		// ����� ������������ ��������� �������� �� ��������� "FFT" ��� "DFT"
		COUNT = ParFFT.np;
		*outCount = COUNT;

		// ��������� ������������ ������ ��� ���������� ������� �� ��������� "FFT" ��� "DFT"
		if ( !(data = (double*)calloc(COUNT, sizeof(double))) )
		{
			free(gArray);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
		}

		// ������ ������� �� ��������� ����
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


	// -------------- 5. ���������������� �������� "TrendFFT" "TrendDFT" ---------------------
	// �������� ������ �� ������� � �������������� ��������� ���� � ������ ������� �� ������� �� ��������� ��� ��� ���
	
	if (strcmp(customParams, "TrendFFT") == 0 || strcmp(customParams, "TrendDFT") == 0)
	{

		// 5.1 �������� ������ ���������� ����
		
		// ��������� ������������ ������ ��� ���������� ������� ������
		if (!(data = (double*)malloc(sizeof(double) * devInfo.dataCount)))
		{
			free(gArray);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
		}
		
		// ��������� �������� ������ ���������� ����
		if ((Kod_Program = Make_File_raznoct(gArray, data, nmax, nmin, devInfo.dataCount, &Kod_LINE)) != 0)
		{
			free(gArray);	 free(data);	*outCount = 2;	ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		}
		else
		{
			// ��������������� ��������� ������� ���������� ���� �������� ���������� ���� ��� ������
			for (i = 0; i < devInfo.dataCount; i++)		gArray[i] = data[i];
			
			free(data);
		}
		
		// 5.2 ������ ������� ������� � �������������� ��������� ��� ��� ���
	
		// ������������� ���������� ��������� ��� ��� ��� �� ���������� ������� customAlgo(...)
		ParFFT.TypeSpec = 3;						// ��� ��������������� ������� 1 - ���, 2 - ��, 3 - ��� (double)
		ParFFT.Fs = (uint32_t)devInfo.rate;			// ������� ������������� ������� ��������������, ��	

		if (strcmp(customParams, "FFT") == 0)
		{
			ParFFT.n = 8;		// ����������� ����� ����� ��� 2^8 = 256
			
			// �������� ������������� ����� ���������� ���� - ������ ���� ������ ����� ����� ��� 2^n = 256 
			if ((ParFFT.Npt = (uint32_t)pow(2, ParFFT.n)) > devInfo.dataCount)
			{
				free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
			};

			//�������������� ����������� ������������ ����� �������� ���������� ����, ������������ � ��������� ���
			for (; (ParFFT.Npt = (uint32_t)pow(2, ParFFT.n)) < devInfo.dataCount; ParFFT.n++);

			ParFFT.Npt /= 2;				// ������������ ����� ����� ���������� ����, ������� 2^n
			ParFFT.npt = ParFFT.Npt;		// ����� ����� ��� = ����� ����� ���������� ����, ������� 2^n (1 ���� ���, ���������� � ������� �� ������� df = Fs / npt - ����� )
		}
		else	//---------- "DFT" -------------
		{

			ParFFT.Npt = devInfo.dataCount;		// ����� ����� ���������� ����, ����������� � ��������� ���
			ParFFT.npt = ParFFT.Npt;			// ����� ����� ��� = ����� ����� ���������� ���� (1 ���� ���, ���������� � ������� �� ������� df = Fs / npt - �����)
		};

		// ��������� ��������� �������
		ParFFT.np = ParFFT.npt / 2;					// ����� �������� � �������
		ParFFT.Offset = ParFFT.npt / 2;				// C������� ��������� ������ ���  
		ParFFT.nBlocks = ParFFT.Npt / ParFFT.Offset - 1;	// ����� ������ ����������
		ParFFT.dF = (float)ParFFT.Fs / (float)ParFFT.npt;			// ���������� � ������� �� �������  
		ParFFT.Fmax = ParFFT.dF * ParFFT.np;		// ������������ ������� � �������, ��  
		
		if (ParFFT.nBlocks < 1)
		{
			free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
		}

		// ����� ������������ ��������� �������� �� ��������� "TrendFFT" "TrendDFT" = ����� �������� �������
		COUNT = ParFFT.np;
		*outCount = COUNT;

		// ��������� ������������ ������ ��� ���������� ������� �������
		if (!(data = (double*)calloc(COUNT, sizeof(double))))
		{
			free(gArray);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
		}

		// ������ ������� �� ��������� ����
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

    // -------------- 6. ���������������� �������� "SCADA-FFT1" ��� "SCADA-DFT1"  ---------------------
	// ������ ����. ������������� ��������� �� �������
	// �������� ������ �� �������� � ������������� ��������� ����
	// ������ ����. ������������� ��������������� �� �������
	// ������� ��������� ��� ��� ��� ���������������� �� �������
	// ������ ������������� ���������� ���������������� �� �������

    if (strcmp(customParams, "SCADA-FFT1") == 0 || strcmp(customParams, "SCADA-DFT1") == 0)
	{
		// ����� ������������ ��������� �������� �� ��������� "SCADA-FFT1" ��� "SCADA-DFT1" 
		*outCount = MCM_SCADA;
		
		//	��������� ������������ ������ ��� ���������� ������� �� ��������� "SCADA-FFT1" ��� "SCADA-DFT1" 
        if (! (MCM_data = (double*)malloc(sizeof(double) * MCM_SCADA)) )
		{
			free(gArray);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
		};
		
		// 6.1 ������ �������������� ������������� ���������  �� �������
		if ((Kod_Program = statictica(gArray, devInfo.dataCount, &StatPar, &Kod_LINE)) != 0)
		{
			free(gArray);	*outCount = 2;	ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		};

		// 6.2 �������� ������ ���������� ����
		// ��������� ������������ ������� ��� ������� �������� ������ ���������� ����
		if (!(data = (double*)malloc(sizeof(double) * devInfo.dataCount)))
		{
			free(gArray);	*outCount = 2;	ret[0] = 1;	ret[1] = __LINE__;	return ret;
		}

		// �������� ������ ���������� ����
		if ((Kod_Program = Make_File_raznoct(gArray, data, nmax, nmin, devInfo.dataCount, &Kod_LINE)) != 0)
		{
			free(gArray); free(data);	*outCount = 2;	ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		}
		else
		{
			// ��������������� ��������� ������� ���������� ���� �������� ���������� ���� ��� ������
			for (i = 0; i < devInfo.dataCount; i++)		gArray[i] = data[i];
			
			free(data);
		}
		
		// 6.3 ������ �������������� ������������� ������� ����� �������� �����
		if ((Kod_Program = statictica(gArray, devInfo.dataCount, &StatPar_, &Kod_LINE)) != 0)
		{
			free(gArray);	*outCount = 2;	ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		};

		// 6.4. ������ ������� � �������������� ��������� ��� � ���
		
		// ������������� ���������� ��������� ��� � ��� �� ���������� ������� customAlgo(...)
		ParFFT.TypeSpec = 2;						// ��� ��������������� ������� 1 - ���, 2 - ��, 3 - ��� (double)
		ParFFT.Fs = (uint32_t)devInfo.rate;			// ������� ������������� ������� ��������������, ��	

		if (strcmp(customParams, "SCADA-FFT1") == 0)
		{
			ParFFT.n = 8;		// ����������� ����� ����� ��� 2^8 = 256
			// �������� ������������� ����� ���������� ���� - ������ ���� ������ ����� ����� ��� 2^n = 256 
			if ((ParFFT.Npt = (uint32_t)pow(2, ParFFT.n)) > devInfo.dataCount)
			{
				free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
			};

			//�������������� ����������� ������������� ����� �������� ���������� ����, ������������ � ��������� ���
			for (; (ParFFT.Npt = (uint32_t)pow(2, ParFFT.n)) < devInfo.dataCount; ParFFT.n++);

			ParFFT.Npt /= 2;				// ������������ ����� ����� ���������� ����, ������� 2^n
			ParFFT.npt = ParFFT.Npt;		// ����� ����� ��� = ����� ����� ���������� ����, ������� 2^n (1 ���� ���, ���������� � ������� �� ������� df = Fs / npt - ����� )
		}
		else	//---------- "SCADA-DFT1" -------------
		{			
			ParFFT.Npt = devInfo.dataCount;		// ����� ����� ���������� ����, ����������� � ��������� ���
			ParFFT.npt = ParFFT.Npt;			// ����� ����� ��� = ����� ����� ���������� ���� (1 ���� ���, ���������� � ������� �� ������� df = Fs / npt - �����)
		};

		// ��������� ��������� �������
		ParFFT.np = ParFFT.npt / 2;					// ����� �������� � �������
		ParFFT.Offset = ParFFT.npt / 2;				// C������� ��������� ������ ���  
		ParFFT.nBlocks = ParFFT.Npt / ParFFT.Offset - 1;	// ����� ������ ����������
		ParFFT.dF = (float)ParFFT.Fs / (float)ParFFT.npt;			// ���������� � ������� �� �������   
		ParFFT.Fmax = ParFFT.dF * ParFFT.np;		// ������������ ������� � �������, ��  

		if (ParFFT.nBlocks < 1)
		{
			free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
		}

		// ��������� ������������ ������ ��� ��������� ������� �������
		if (!(data = (double*)calloc(ParFFT.np, sizeof(double))))
		{
			free(gArray);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
		};
		// ������ ������� �� ��������� ����
		if ((Kod_Program = DFTAnalysis(gArray, data, ParFFT, &Kod_LINE)) != 0)
		{
			free(gArray);	free(data);		*outCount = 2;		ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		};

		// 6.5 ������ ������������ ����������

		ParFFT.win_dF = 2;							// +- ������ ������ ������ ���������, ��
		
		// ������ ������������ ���������� �� ��������� ���-320 Param
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
			MCM_data[79] = 0.0;	//��� ����������� ���������� ��������� castomAlgo.c

			free(gArray); free(ret);
   			return MCM_data;
		}
	}

	// -------------- 7. ���������������� �������� "SCADA-FFT2" ��� "SCADA-DFT2"  (���������� �������� ��� ����������)---------------------
	// ������ ����. ������������� ��������� �� �������
	// ������� ��������� ��� ��� ��� ��������� �� �������
	// ������ ������������� ���������� ��������� �� �������
	if (strcmp(customParams, "SCADA-FFT2") == 0 || strcmp(customParams, "SCADA-DFT2") == 0)
	{
		// ����� ������������ ��������� �������� �� ��������� "SCADA-FFT2" ��� "SCADA-DFT2" 
		*outCount = MCM_SCADA;
		
		//	��������� ������������ ������ ��� ������ ������������ ���������� �� ������� (���������� ������ ���������)
        if (!(MCM_data = (double*)malloc(sizeof(double) * MCM_SCADA)))
		{
			free(gArray);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
		};

		// 7.1 ������ �������������� ������������� ��������� �� �������
		if ((Kod_Program = statictica(gArray, devInfo.dataCount, &StatPar, &Kod_LINE)) != 0)
		{
			free(gArray);	*outCount = 2;	ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		};

		// 7.2. ������ ������� � �������������� ��������� ���, ���

		ParFFT.TypeSpec = 2;						// ��� ��������������� ������� 1 - ���, 2 - ��, 3 - ��� (double)
		
		// ������������� ���������� ��������� ��� � ��� �� ���������� ������� customAlgo(...)
		ParFFT.Fs = (uint32_t)devInfo.rate;			// ������� ������������� ������� ��������������, ��	

		if (strcmp(customParams, "SCADA-FFT2") == 0)
		{
			ParFFT.n = 8;		// ����������� ����� ����� ��� 2^8 = 256
			// �������� ������������� ����� ���������� ���� - ������ ���� ������ ����� ����� ��� 2^n = 256 
			if ((ParFFT.Npt = (uint32_t)pow(2, ParFFT.n)) > devInfo.dataCount)
			{
				free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
			};

			//��������������� ������������� ����� �������� ���������� ����, ������������ � ��������� ���
			for (; (ParFFT.Npt = (uint32_t)pow(2, ParFFT.n)) < devInfo.dataCount; ParFFT.n++);

			ParFFT.Npt /= 2;				// ������������ ����� ����� ���������� ����, ������� 2^n
			ParFFT.npt = ParFFT.Npt;		// ����� ����� ��� = ����� ����� ���������� ����, ������� 2^n (1 ���� ���, ���������� � ������� �� ������� df = Fs / npt - ����� )	
		}
		else	//---------- "SCADA-DFT2" -------------
		{
			ParFFT.Npt = devInfo.dataCount;		// ����� ����� ���������� ����, ����������� � ��������� ���
			ParFFT.npt = ParFFT.Npt;			// ����� ����� ��� = ����� ����� ���������� ���� (1 ���� ���, ���������� � ������� �� ������� df = Fs / npt - �����)
		};

		// ��������� ��������� �������
		ParFFT.np = ParFFT.npt / 2;					// ����� �������� � �������
		ParFFT.Offset = ParFFT.npt / 2;				// C������� ��������� ������ ���  
		ParFFT.nBlocks = ParFFT.Npt / ParFFT.Offset - 1;	// ����� ������ ����������
		ParFFT.dF = (float)ParFFT.Fs / (float)ParFFT.npt;			// ���������� � ������� �� �������  
		ParFFT.Fmax = ParFFT.dF * ParFFT.np;		// ������������ ������� � �������, ��  

		if (ParFFT.nBlocks < 1)
		{
			free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
		}

		// ��������� ������������ ������ ��� ��������� ������� �������
		if (!(data = (double*)calloc(ParFFT.np, sizeof(double))))
		{
			free(gArray);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
		};

		// ������ ������� �� ��������� ����
		if ((Kod_Program = DFTAnalysis(gArray, data, ParFFT, &Kod_LINE)) != 0)
		{
			free(gArray);	free(data);		*outCount = 2;		ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		};
		
		// 7.3 ������ ������������ ���������� �� �������
		
		ParFFT.win_dF = 2;							// +- ������ ������ ������ ���������, ��

		// ������ ������������ ���������� �� ��������� ���-320 Param
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
			MCM_data[79] = 0.0;	//��� ����������� ���������� ��������� castomAlgo.c

			free(gArray); free(ret);
			return MCM_data;
		}
	}

	// -------------- 8. ���������������� �������� "MIC-FFT" ��� "MIC-DFT"  ---------------------
	// ������� ��������� ��� ��� ��� ��������� �� �������
	// ������ ������������� ���������� ��������� �� �������
	
	if (strcmp(customParams, "MIC-FFT") == 0 || strcmp(customParams, "MIC-DFT") == 0)
	{
		*outCount = MCM_MIC300;
		//	��������� ������������ ������ ��� ������ ������������ ���������� �� ������� (��������� ������ ���������)
		if (!(MCM_data = (double*)malloc(sizeof(double) * MCM_MIC300)))
		{
			free(gArray);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
		};

		// 8.1. ������ ������� � �������������� ��������� ���, ���
		
		ParFFT.TypeSpec = 2;						// ��� ��������������� ������� 1 - ���, 2 - ��, 3 - ��� (double)

		// ������������� ���������� ��������� �� �� ���������� ������� customAlgo(...)
		ParFFT.Fs = (uint32_t)devInfo.rate;			// ������� ������������� ������� ��������������, ��	

		if (strcmp(customParams, "MIC-FFT") == 0)
		{
			// �������� ������������� ����� ���������� ���� - ������ ���� ������ ����� ����� ��� 2^12 = 4096 
			i = 12;
			if ((ParFFT.Npt = (uint32_t)pow(2, i)) > devInfo.dataCount)
			{
				free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
			}

			//��������������� ������������� ����� �������� ���������� ����, ������������ � ��������� ���
			for (; (ParFFT.Npt = (uint32_t)pow(2, i)) < devInfo.dataCount; i++);

			ParFFT.Npt /= 2;				// ������������ ����� �������� ���������� ����, ������� 2^n
			ParFFT.npt = ParFFT.Npt;		// ����� ����� ��� = ����� ����� ���������� ����, ������� 2^n (1 ���� ���, ���������� � ������� �� ������� df = Fs / npt - ����� )
		}
		else	//---------- "MIC-DFT" -------------
		{

			ParFFT.Npt = devInfo.dataCount;		// ����� ����� ���������� ����, ����������� � ��������� ���
			ParFFT.npt = ParFFT.Npt;			// ����� ����� ��� = ����� ����� ���������� ���� (1 ���� ���, ���������� � ������� �� ������� df = Fs / npt - �����)
		};

		// ��������� ��������� �������
		ParFFT.np = ParFFT.npt / 2;					// ����� �������� � �������
		ParFFT.Offset = ParFFT.npt / 2;				// C������� ��������� ������ ���  
		ParFFT.nBlocks = ParFFT.Npt / ParFFT.Offset - 1;	// ����� ������ ����������
		ParFFT.dF = (float)ParFFT.Fs / (float)ParFFT.npt;	// ���������� � ������� �� �������  
		ParFFT.Fmax = ParFFT.Fs / 2;				// ������������ ������� � �������, �� 
	
		if (ParFFT.nBlocks < 1)
		{
			free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
		}

		// ��������� ������������ ������ ��� ��������� ������� �������
		if (!(data = (double*)calloc(ParFFT.np, sizeof(double))))
		{
			free(gArray);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
		};

		// ������ ������� �� ��������� ����
		if ((Kod_Program = DFTAnalysis(gArray, data, ParFFT, &Kod_LINE)) != 0)
		{
			free(gArray);	free(data);		*outCount = 2;		ret[0] = Kod_Program;	ret[1] = Kod_LINE;	return ret;
		};

		// 8.2 ������ ������������ ���������� ��������� �� �������
		
		ParFFT.win_dF = 2;							// +- ������ ������ ������ ���������, ��

		// ������ ������������ ���������� �� ��������� ���-320 Param
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
			MCM_data[41] = 0.0;	//��� ����������� ���������� ��������� castomAlgo.c

			free(gArray); free(ret);
			return MCM_data;
		}
	}

//------------------- ����� ���������������� ���������� -----------------------------

	free(gArray);
	COUNT = 2;
	*outCount = COUNT;
	Kod_LINE = __LINE__;
	ret[0] = 3; ret[1] = Kod_LINE;
	return ret;
} 
