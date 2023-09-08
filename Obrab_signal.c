#include "Obrab_signal.h"

// ��������� ���������� �������� ���-320_Param_16���_���_16384_8��
// ��������� ���������������� �������� "MIC-FFT-ACHX" ��� "MIC-DFT-ACHX"
// 1. ������ ��������� ��� ��� ��� ��������� �� �������
// 2. ������ ������������ ���������� ��������� �� �������
// *raw -  ������ �������� ���������� ����
// FFT - ��������� ���������� ����
// *outCount - ����� �������� ���������� ���� *raw - �� ����� / ����� ������������ ��������� ���������� - �� ������
double* TBB320_Param_16000_16384_8(double* raw, ParamFFT ParFFT, uint32_t* outCount)
{
	uint32_t	i;					// ������� ������ ������� ������
	uint16_t	Kod_Program = 0;	// ��� �������� ������� ���������
	long		Kod_LINE = 0;		// ��� ������ ���������� ������� ���������

	// ����������� � ��������� ������������ ������ ��� ������������ ��� ������ ��������� ������� 
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
// -------------- 

	// ����������� � ��������� ������������ ������ ��� ������������ �������� ���������� ����
	double* gArray = (double*)malloc(sizeof(double) * (*outCount));
	if (!gArray)
	{
		*outCount = 2;	ret[0] = 1.0;	ret[1] = __LINE__;	return ret;
	}

	double* data;		// ������ ���������� ������� �������

	uint16_t
		VAD_number_Param = 29,	//����� ������������ ��������� ��������  
		VAD_step_garmonic = 50;	// ��� ������ ������������� �������� ������� 50 ��� 100 ��

	ParamAmplSpectr		ParSpec;	// ��������� �������� ������������ ���������� �� �������

#ifdef _WIN32 
	// ������������ �������� ���������� ���� *raw � ������ *gArray
	for (i = 0; i < *outCount; i++)		gArray[i] = raw[i];
#endif 


	//����������� � ��������� ������������ ������ ��� ������ ����������� ������� ������������ ���������� �� ������� (���������� ������ ���������)
	double* VAD_Param = (double*)malloc(sizeof(double) * VAD_number_Param);
	if (!VAD_Param)
	{
		free(VAD_Param);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
	};

	// 1. ������ ������� � �������������� ��������� ���, ���
	if (ParFFT.nBlocks < 1)
	{
		free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
	};

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

	// 9.2 ������ ������������ ���������� ��������� �� ������� �� ��������� ���-320 ���
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
		VAD_Param[28] = 0.0;	//��� ����������� ���������� ��������� castomAlgo.c

		free(gArray); free(ret);
		*outCount = VAD_number_Param;
		return VAD_Param;
	}
}

// ��������� ���������� �������� ���-1000-2_��� Param_32���_���_262144_1��
// ��������� ���������������� �������� "MIC-FFT-ACHX" ��� "MIC-DFT-ACHX"
// 1.������� ��������� ��� ��� ��� � ��������� ��������� �� �������
// 2. ������ ������������ ���������� ��������� ��������� �� �������
// *raw -  ������ �������� ���������� ����
// FFT - ��������� ���������� ����
// *outCount - ����� �������� ���������� ���� *raw - �� �����, ����� ������������ ��������� ���������� - �� ������
double* TBB1000_ACHX_Param_32000_262144_1(double * raw, ParamFFT ParFFT, uint32_t * outCount)
{
	uint32_t	i;					// ������� ������ ������� ������
	uint16_t	Kod_Program = 0;	// ��� �������� ������� ���������
	long		Kod_LINE = 0;		// ��� ������ ���������� ������� ���������

	// ����������� � ��������� ������������ ������ ��� ������������ ��� ������ ��������� ������� 
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
// -------------- 

	// ����������� � ��������� ������������ ������ ��� ������������ �������� ���������� ����
	double* gArray = (double*)malloc(sizeof(double) * (*outCount));
	if (!gArray)
	{
		*outCount = 2;	ret[0] = 1.0;	ret[1] = __LINE__;	return ret;
	}

	double* data;		// ������ ���������� ������� �������

	uint16_t
		MCM_MIC300_ACHX = 30,	//����� ������������ ��������� ��������  
		MCM_step_garmonic = 50;	// ��� ������ ������������� �������� ������� 50 ��� 100 ��

	ParamAmplSpectr		ParSpec;	// ��������� �������� ������������ ���������� �� �������

#ifdef _WIN32 
	// ������������ �������� ���������� ���� *raw � ������ *gArray
	for (i = 0; i < *outCount; i++)		gArray[i] = raw[i];
#endif 


	//����������� � ��������� ������������ ������ ��� ������ ����������� ������� ������������ ���������� �� ������� (���������� ������ ���������)
	double* MCM_data = (double*)malloc(sizeof(double) * MCM_MIC300_ACHX);
	if (!MCM_data)
	{
		free(MCM_data);	*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
	};

	// 1. ������ ������� � �������������� ��������� ���, ���
	if (ParFFT.nBlocks < 1)
	{
		free(gArray);	*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
	};

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


	// 9.2 ������ ������������ ���������� ��������� �� ������� �� ��������� ���-320 ���
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
			MCM_data[29] = 0.0;	//��� ����������� ���������� ��������� castomAlgo.c

			free(gArray); free(ret);
			*outCount = MCM_MIC300_ACHX;
			return MCM_data;
		}
}


// ��������� ������� ������������ ��������� �������������� ������� H1(f) ��� H2(f)
// 1.������� ��������� ��� ��� ��� � ��������� ��������� �� �������
// 2. ������ ������������ ���������� ��������� ��������� �� �������
// *F -  �� ����� - ������ �������� ������� ���� � ������ �������� *outCount / �� ������ - ����������� ������ ������� ���� � ������ �������� *outCount
// *X -  �� ����� - ������ �������� ������� ������� � ������ �������� *outCount / �� ������ - ������� ������������� � ������ �������� *outCount
// ParFFT - ��������� ���������� ����
// Type_H -  ��� �������������� ������������� ���: 1 - H1(f), 2 - H2(f)
// *outCount - �� ����� - ����� �������� ������� *F, *X / �� ������ ����� ������������ ��������� �������� ���, ��� � ����-��� �������������
// ���������� ������ �������� ��� ������� � ������ �������� *outCount
double* ACHX_function(double* F, double* X, ParamFFT ParFFT, int Type_H, uint32_t* outCount)
{
	double	* GFF,		// ��������� ��� ������� ���� ���� ������
			* CFX;		// �������������� ����� ��������� �� ��������� ������� ���� � ������� ���� ������

	double	* GXX,		// ��������� ��� ������� �������� ���� ������
			* QFX;		// ������ ����� ��������� �� ��������� ������� ���� � ������� ���� ������

	double	* ReF,		// �������������� ����� �� ������� ����
			* ImF,		// ������ ����� �� ������� ����
			* ReX,		// �������������� ����� �� ������� �������
			* ImX;		// ������ ����� �� ������� �������
	
	// ����������� � ��������� ������������ ������ ��� ������������ ��� ������ ��������� ������� 
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
// -------------- 

	// �������� ������������� ������ 
	if ((ParFFT.nBlocks < 1) || (ParFFT.npt > *outCount) )
	{
		*outCount = 2;	ret[0] = 3; ret[1] = __LINE__;	return ret;
	};

	// ��������� ������������ ������ ��� ��� ������� ���� 
	if (!(GFF = (double*)calloc(ParFFT.np, sizeof(double))))
	{
		*outCount = 2;	ret[0] = 1.0;	ret[1] = __LINE__;	return ret;
	};

	// ��������� ������������ ������ ��� ��� ������� �������
	if (!(GXX = (double*)calloc(ParFFT.np, sizeof(double))))
	{
		*outCount = 2;	ret[0] = 1.0;	ret[1] = __LINE__;	return ret;
	};

	// ��������� ������������ ������ ��� �������������� ����� �� ��������� ������� ������� ���� � �������
	if (!(CFX = (double*)calloc(ParFFT.np, sizeof(double))))
	{
		*outCount = 2;	ret[0] = 1.0;	ret[1] = __LINE__;	return ret;
	};
	// ��������� ������������ ������ ��� ������ ����� �� ��������� ������� ������� ���� � �������
	if (!(QFX = (double*)calloc(ParFFT.np, sizeof(double))))
	{
		*outCount = 2;	ret[0] = 1.0;	ret[1] = __LINE__;	return ret;
	};

	// ��������� ������������ ������ ��� Re ����� ������� ����
	if (!(ReF = (double*)calloc(ParFFT.npt, sizeof(double))))
	{
		*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
	};
	// ��������� ������������ ������ ��� Im ����� ������� ����
	if (!(ImF = (double*)calloc(ParFFT.npt, sizeof(double))))
	{
		*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
	};
	// ��������� ������������ ������ ��� Re ����� ������� �������
	if (!(ReX = (double*)calloc(ParFFT.npt, sizeof(double))))
	{
		*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
	};
	// ��������� ������������ ������ ��� Im ����� ������� �������
	if (!(ImX = (double*)calloc(ParFFT.npt, sizeof(double))))
	{
		*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
	};

	//����������� � ��������� ������������ ������ ��� ������������ ������� H(f)
	double* H = (double*)calloc(ParFFT.np, sizeof(double));
	if (!H)
	{
		*outCount = 2;	ret[0] = 1; ret[1] = __LINE__;	return ret;
	};

	// ������ ��� �������  
	uint32_t	i,n,
			point,
			point_n;
	
	double SPM = 1.0 / (2.0 * pow(ParFFT.np, 2) * ParFFT.dF * ParFFT.nBlocks);		// ����������� ��������� ������� ���
	double SM = 1.0 / (2.0 * pow(ParFFT.np, 2) * ParFFT.nBlocks);					// ����������� ��������� ������� C�


	for (i = 0; i < ParFFT.np; i++)		GFF[i] = GXX[i] = CFX[i] = QFX[i] = 0;

	// ������ ������������ ������� �� ����� ������ nBlocks ���������� ���� 
	for (point = 0, i = 1; i <= ParFFT.nBlocks; i++, point += ParFFT.Offset)
	{
		// ������������� ������� Re, Im ���������� ������� ���� *F 
		for (n = 0, point_n = point; (n < ParFFT.npt) && (point_n < ParFFT.Npt); n++, point_n++)
		{
			ReF[n] = F[point_n];	ImF[n] = 0.0;
			ReX[n] = X[point_n];	ImX[n] = 0.0;
		};
		// ������ �� ������� ���� �������� �����
		if (!Fft_transform(ReF, ImF, ParFFT.npt))
		{
			*outCount = 2; ret[0] = 3; ret[1] = __LINE__; return ret;
		};
		// ������ �� ������� ������� �������� �����
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
			H[n] = sqrt(CFX[n] * CFX[n] + QFX[n] * QFX[n]) / GFF[n];	// ������������ ������� H1-1
		}
		else
		{
			H[n] = GXX[n] / sqrt(CFX[n] * CFX[n] + QFX[n] * QFX[n]);	// ������������ ������� H2-1
		};
//		F[n] = GFF[n] * SM;			// ������������ �������� ������� ����
		F[n] = sqrt(GFF[n] * SM);	// ����������� ������ ������� ����

		X[n] = (CFX[n] * CFX[n] + QFX[n] * QFX[n]) / (GFF[n] * GXX[n]);	// ������� �������������
	};

	free(GFF); free(GXX); free(ReF); free(ReX); free(ImF); free(ImX);
	free(ret);
	*outCount = ParFFT.np;
	return H;
}


// ��������� ������� ���������� ��� �������������� �����
// *H - ������ �������� ��� ������� � ������ �������� *N_data
// *F - ������ ������������ ������� ������� ������� ���� � ������ �������� *N_data
// N_data -  ����� �������� c������ ��� ������� � ������� ����
// *Par -  ��������� �� ���������� ��������� �������� �������� ������ � ��������� ���������� ���
uint16_t ACHX_param(double* H, double* F, uint32_t N_data, ParamACHX* Par)
{
	uint32_t	i,
				i_low,		//����� ��������� ������ ������ �������� ��������� ���  
				i_high,		//����� ��������� ������� ������ �������� ��������� ���  
				N_d;		//����� �������� ��� � ������� ��������� 
	float		Porog;		// ��������� �������� ���������� ��������� �������� ������������ ������� ������� ����

	// ����������� ������� ������� �������� ��������� �� ������������ ������� ������� ������� ���� �� ��������:
	// ���������� ������������� �������� ������������ ������� ������ ������� ���� �� -20�� ��� � 10 ���
	for (Porog = F[1] / 10.0, i = 1; (i < N_data) && (F[i] > Porog); i++);
	Par->Fh = Par->dF * i; // ������� ������� ������� ����
	
	// ���������� �������� ������� ������� ������������ �������� ��������� 5���, ���� ������� ������� ������� ���� ������ 5���
	if (Par->Fh < Par->F_high)	Par->F_high = Par->Fh;

	// ����������� ������ ��������� ������ ������� �������� ���������
	i_low  = Par->F_low  / Par->dF;
	i_high = Par->F_high / Par->dF;
	N_d = i_high - i_low + 1;

	if (N_d <= 3)
	{
		__LINE__;
		return 4;
	}

	i = 159.15 / Par->dF;
	Par->Ag_160 = H[i];	//	 �������� ��� �� ������� 159,15 ��

	Par->Mx = 0;		// �������������� �������� 
	Par->CKO = 0;		// ���

	Par->Amax = 0;		// ������������ �������� ���
	Par->Amin = 1e10;	// ����������� �������� ���

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


// ��������� ������� �������������� ���������� �� �������
// *row - ������ ������ ��������� ���������� ����
// N_data -  ����� �������� ���������� ����
// *Par -  ��������� �� ���������� ��������� �������� ����������� ������ �������������� ���������� 
// *kod_line - ��������� �� ����� ������ ������ ���������
uint16_t statictica(const double* row, uint32_t N_data, ParamStatistic* Par, long* kod_line)
{

	if (N_data <= 3)
	{
		*kod_line = __LINE__;
		return 4;
	}

	uint32_t	i,
		N_d = N_data - 1;

	Par->Mx = 0;		// �������������� ��������
	Par->Disp = 0;		// ���������
	Par->CKO = 0;		// ���
	Par->CKZ = 0;		// ���
	Par->Ax = 0;		// ����������
	Par->Ex = 0;		// �������
	Par->Ampl = 0;		// ���������
	Par->Amax = 0;		// ������������ �� ������ ��������

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
	// ������ ������������� ����������� ����. ���-�. 
	Par->Pogr_MX = Par->CKO * sqrt(1.0 / N_data);
	Par->Pogr_Disp = Par->Disp * sqrt(2.0 / N_data);
	Par->Pogr_CKO = Par->CKO * sqrt(1.0 / (2.0 * N_data));
	Par->Pogr_CKZ = Par->CKZ * sqrt(1.0 / (2.0 * N_data));
	Par->Pogr_Ax = sqrt(15.0 / N_data);
	Par->Pogr_Ex = sqrt(96.0 / N_data);
	return 0;
}


// ��������� ������������ ����������� ����� ����� ����������
// *row - ������ ������ ��������� ���������� ����
// *row_pf - ������ ����������� ���������� ���������� ����
// nmax - ����� ����� �������� ���������� ����
// nmin - ����� ����� ������ ���������� ����
// N_data -  ����� �������� ���������� ����
// *kod_line - ��������� �� ����� ������ ������ ���������
uint16_t Make_File_raznoct(const double* row, double* row_pf, uint32_t nmax, uint32_t nmin, uint32_t N_data, long* kod_line)
{
	uint32_t	p;
	uint16_t	prog; // ������������ ��� �����������
	double* row_delta;	// ��������� ������ �������������� �������� 

	// ��������� ������������ ������ ��� �������� ���������� ���� � ��������� ���������� ��������
	if ((row_delta = (double*)malloc(sizeof(double) * N_data)) == NULL)
	{
		*kod_line = __LINE__; return 1;
	}
	// ���������� ���� ����� sing(x) � ������ ����� nmax
	if ((prog = Sing_Win(row, row_pf, nmax, N_data, kod_line)) != 0)
	{
		free(row_delta);	return prog;
	}
	else
	{
		// ������������� �������� ����������� ������� 
		for (p = 0; p < N_data; p++)
			row_delta[p] = row[p] - row_pf[p];
	}

	// ���������� ���� ����� sing(x) � ������ ����� nmin
	if ((prog = Sing_Win(row_delta, row_pf, nmin, N_data, kod_line)) != 0)
	{
		free(row_delta);  return prog;
	}

	free(row_delta);
	*kod_line = 0;
	return 0;
}

// ��������� ���������� ���������� ���� ����� sing(x) = sin(x) / x
// *v - ��������� �� ������ ���������� ���� 
// *v_win - ��������� �� ������ ����������� ����������
// n_win - ����� ����� ������������ ����� ���������� ����. ����� ����� ����� ���������� ���� 2*n_win+1 
// N_data - ����� ����� ���������� ����
// *kod_line - ��������� �� ����� ������ ������ ���������
uint16_t Sing_Win(const double* row, double* row_win, uint32_t n_win, uint32_t N_data, long* kod_line)
{
	double 	A,		// ������������� ����������� ���� sing(�) 
		* sing,		// ������ ������� ������������� ���������� ���� sing(�)
		* Vsing,	// ������ ���������� �� ���� sing(x) �������� ���������� ����
		y;      	// ��������������� �������� ���������� ����

	uint32_t
		n_sing = 2 * n_win, // ������� ������ ������� ������������� ���� sing(�)
		k, i,
		p, p1;

	// ��������� ������������ ������ ��� �������� �-��� �������� ����
	if ((sing = (double*)malloc(sizeof(double) * (n_sing + 1))) == NULL)
	{
		*kod_line = __LINE__;  return 1;
	}
	// ��������� ������������ ������ ��� �������� ������� ���������� ����
	if ((Vsing = (double*)malloc(sizeof(double) * (n_sing + 1))) == NULL)
	{
		*kod_line = __LINE__;  return 1;
	}
	// ������������� ������� ������������� ���������� ���� sing(x)
	for (i = n_win, k = 0; i > 0; i--, k++)
	{
		sing[k] = sin(M_PI * (double)i / (double)n_win) / (M_PI * (double)i / (double)n_win);
		sing[n_sing - k] = sing[k];
	};
	sing[n_win] = 1;

	// ������ �������������� ����������� �������� ���� sing(x)
	for (A = 0.0, i = 0; i <= n_sing; i++) A += sing[i];
	if (!A)
	{
		*kod_line = __LINE__;  return 5;
	}

	//-----------������������� ������� Vsing[] ��� ������ row_win[0] - �������� �������������� ��������
	//���������� ����� n_win+1 ��������� ������� Vsing[] �������� �������� ���������� ���� row[i]
	for (i = 0; i <= n_win; i++)	Vsing[i] = row[0];

	// ���������� ������ n_win ��������� ������� Vsing[] ������� �������� ���������� ���� row[i]
	for (p = 1, i = n_win + 1; (p <= n_win) && (i <= n_sing); p++, i++)   Vsing[i] = row[p];

	//---------- ������ �������� �������������� �������� ���������� ����
	for (y = 0.0, i = 0; i <= n_sing; i++)  y += sing[i] * Vsing[i];
	row_win[0] = y / A;

	//---------	������� ���� ���������� ���������� ����
	for (p = n_win + 1, p1 = 1; p < N_data; p++, p1++)
	{
		// ����� �������� �������������� ������ �� ���� �������� �����
		for (i = 0; i < n_sing; i++)  Vsing[i] = Vsing[i + 1];
		Vsing[n_sing] = row[p];	// ������������� ���������� �������� Vsing ����� �������� ���������� ����

		//---------- ������ ������ �������������� �������� ���������� ����
		for (y = 0.0, i = 0; i <= n_sing; i++)  y += sing[i] * Vsing[i];
		row_win[p1] = y / A;
	};

	//------ ���������� �������� � ����� ���������� ����
	for (p--; p1 < N_data; p1++, p--)
	{
		// ����� �������� �������������� ������ �� ���� �������� �����
		for (i = 0; i < n_sing; i++)  Vsing[i] = Vsing[i + 1];
		Vsing[n_sing] = row[p];	// ������������� ���������� �������� Vsing ��������� ��������� ���������� ���� row[p]
		//---------- ������ ������ �������������� �������� ���������� ����
		for (y = 0.0, i = 0; i <= n_sing; i++)  y += sing[i] * Vsing[i];
		row_win[p1] = y / A;
	};

	free(sing);
	free(Vsing);
	*kod_line = 0;
	return 0;
}

// ��������� ������� ������� �� ������� �� ��������� ��� ��� ���
// *gArr - ��������� �� ������ ���������� ����
// *data - ��������� �� ������ �������������� ������� 
// *Par - ��������� �� ���������� ��������� ���������� ���� 
// *kod_line - ��������� �� ����� ������ ������ ���������
uint16_t DFTAnalysis(double* gArr, double* data, ParamFFT Par, long* kod_line)
{
	uint32_t	n,
		i;
	uint32_t	point,
		point_n;

	double* Re;		// ������ �������������� ����� �������
	double* Im;		// ������ ������ ����� �������

	// ��������� ������������ ������ ��� ������� Re ����������� ������� ��
	if (!(Re = (double*)malloc(sizeof(double) * Par.npt)))
	{
		*kod_line = __LINE__;  return 1;
	};

	// ��������� ������������ ������ ��� ������� Im ����������� ������� ��
	if (!(Im = (double*)malloc(sizeof(double) * Par.npt)))
	{
		free(Re); *kod_line = __LINE__;  return 1;
	}

	// ������ �� �� ����� ������ nBlocks ���������� ���� 
	for (point = 0, i = 1; i <= Par.nBlocks; i++, point += Par.Offset)
	{
		// ������������� ������� Re, Im ���������� ���������� ���� 
		for (n = 0, point_n = point; (n < Par.npt) && (point_n < Par.Npt); n++, point_n++)
		{
			Re[n] = gArr[point_n];	Im[n] = 0.0;
		}

		// ������ ��������� ��� ��� ��� ��� �������� �����
		if (!Fft_transform(Re, Im, Par.npt))
		{
			free(Re); free(Im); *kod_line = __LINE__;  return 2;
		}

		// ������ �������� ������� ���� Type_spec �� ������
		Estimate_Sym_Spec(data, Re, Im, Par.np, Par.dF, Par.nBlocks, Par.TypeSpec);
	};
	// ������ ������������ ������� ��� �������� ��������
	if (Par.TypeSpec == 3)
		for (i = 0; i < Par.np; i++)	data[i] = sqrt(data[i]);

	free(Re);	free(Im);
	*kod_line = 0;
	return 0;
}


// ��������� ������������ ��������� ��������
// *Re - ��������� �� ������ ��������� �������� �������
// *Re - ��������� �� ������ �������������� ����� �������
// *Im - ��������� �� ������ ������ ����� �������
// np - ����� ����� � �������
// df - ���������� �� ������� � �������
// nBlocks -  ������ ����������
// Type_spec - ��� ��������������� �������
void Estimate_Sym_Spec(double* Sp, double* Re, double* Im, uint32_t np, double df, uint16_t nBlocks, uint16_t Type_spec)
{
	unsigned long 	i;
	double SPM = 1.0 / (2.0 * pow(np, 2) * df * nBlocks);
	double SM = 1.0 / (2.0 * pow(np, 2) * nBlocks);

	switch (Type_spec)
	{
	case 1:
		// ������ ������� �������� ������������ ��������� �������� ��� � ������������� �� ������
		for (i = 0; i < np; i++)
			Sp[i] += (Re[i] * Re[i] + Im[i] * Im[i]) * SPM;
		break;
	case 2: case 3:
		// ������ ������� �������� ������� �������� �� � ������������� �� ������
		for (i = 0; i < np; i++)
			Sp[i] += (Re[i] * Re[i] + Im[i] * Im[i]) * SM;
		break;
	}
}



// ��������� ������� ������������ ���������� �� ������� ���-320-2 �� ������� ��������
// *SpM - ��������� �� ������ ������������ �������� �������
//  Par - �������� ���������� �������� ��� ��� ���
// *Spec - ��������� �� ����������� ��������� ��������� ������������ ���������� �� ������� 
//  step -  ��� ������� �������� ������� 50 ��� 100 ��

uint16_t Diagnostic_Parameters_TBB320(double* SpM, ParamFFT Par, ParamAmplSpectr* Spec, uint16_t step, long* kod_line)
{
	uint16_t	prog;	// ������������ ��� �����������
	uint32_t	i,
				f;		// ������� ����������� ���������

	double	Mmax = 0,	// ������������ �������� ���������
		FMmax,		// ������� ����������� ��������� � �������
		fmin,		// ����������� ������� ��������� 
		fmax;		// ������������ ������� ��������� 

	if (step != 50 && step != 100)
	{
		*kod_line = __LINE__;  return 3;
	};

	//-------- ������ ��������� �� ������� 100 �� -----------
	FMmax = 100;
	fmin = FMmax - Par.win_dF;
	fmax = FMmax + Par.win_dF;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->A100 = sqrt(Mmax);

	//-------- ����� ������������ �������� � ������ [155; 185] �� -----------
	fmin = 155.0;
	fmax = 185.0;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->Agmax_155_185 = sqrt(Mmax);
	Spec->Fgmax_155_185 = FMmax;

	//-------- ������ ��������� 1-� ���������� ������� TBB-320-2 -----------
	FMmax = 1450;
	fmin = FMmax - Par.win_dF;
	fmax = FMmax + Par.win_dF;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->A1_1450 = Mmax;

	//-------- ������ ��������� 2-� ���������� ������� TBB-320-2 -----------
	FMmax = 2900;
	fmin = FMmax - 2 * Par.win_dF;
	fmax = FMmax + 2 * Par.win_dF;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->A2_2900 = Mmax;

	//-------- ������ ��������� 3-� ���������� ������� TBB-320-2 -----------
	FMmax = 4350;
	fmin = FMmax - 3 * Par.win_dF;
	fmax = FMmax + 3 * Par.win_dF;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->A3_4350 = Mmax;

	//-------- ������ ��������� 4-� ���������� ������� TBB-320-2 -----------
	FMmax = 5800;
	fmin = FMmax - 4 * Par.win_dF;
	fmax = FMmax + 4 * Par.win_dF;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->A4_5800 = Mmax;

	//-------- ������ ��������� 5-� ���������� ������� TBB-320-2 -----------
	FMmax = 7250;
	fmin = FMmax - 5 * Par.win_dF;
	fmax = FMmax + 5 * Par.win_dF;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->A5_7250 = Mmax;

	//-------- ����� ������������ ��������� � ���������� [1; Fmax] �� -----------
	fmin = 1.0;
	fmax = Par.Fmax;
	if ((prog = Evaluation_Ampl_Freq_in_Spectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax, &FMmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->Agmax = sqrt(Mmax);
	Spec->Fgmax = FMmax;

	//-------- ������ �������� ������� � ������ [155; 185] �� -----------
	fmin = 155.0;
	fmax = 185.0;
	if ((prog = Evaluation_Power_in_DiapazonSpectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->M_155_185 = Mmax;

	//-------- ������ �������� ������� � ������ [200; 1000) �� -----------
	fmin = 200.0;
	fmax = 1000.0 - Par.dF;
	if ((prog = Evaluation_Power_in_DiapazonSpectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->M[0] = Mmax;

	//-------- ������ �������� ������� � 15 ������� ������ �� 1000 �� -----------
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

	//-------- ������ ��������� �������� �������� �������, ������� 50 ��, � ������ [200; 1000) �� -----------
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


	//-------- ������ ��������� �������� �������� �������, ������� 50 ��, � 15 ������� �� 1000 �� -----------
	for (i = 1; i <= 15; i++)
	{
		Spec->Mg[i] = 0;
		for (f = i * 1000; f < (i + 1) * 1000; f += step)
		{
			fmin = f - i * Par.win_dF;
			fmax = f + i * Par.win_dF;

			if (fmax < Par.Fmax)
			{
				// ������ �������� ��������� ������������ ��������� � ������ ������ [fmin; fmax]
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



// ��������� ������� ��������������� ������������ ���������� �� ������� � ���������� �������� ��������� �������� Fm �� ���-320-2 �� ������� ��������
// *SpM - ��������� �� ������ ������������ �������� �������
//  Par - �������� ���������� �������� ��� ��� ���
// *Spec - ��������� �� ����������� ��������� ��������� ������������ ���������� �� ������� 
//  step -  ��� ������� �������� ������� 50 ��� 100 ��

uint16_t Diagnostic_Parameters_VarFm_TBB320(double* SpM, ParamFFT Par, ParamAmplSpectr* Spec, uint16_t step, long* kod_line)
{
	uint16_t	prog;	// ������������ ��� �����������
	uint32_t	i;
	
	double
		f,			// ������� ����������� ���������
		step_fm,		// ��������� �������� � ������� �� ��������� � ������� ��������� �������� ������� (0,5 ��� 1)
		Mmax,		// ������������ �������� ���������
		FMmax,		// ������� ����������� ��������� � �������
		fmin,		// ����������� ������� ��������� 
		fmax;		// ������������ ������� ��������� 

	if (step != 50 && step != 100)
	{
		*kod_line = __LINE__;  return 3;
	}
	else {
		step_fm = (double)step / 100.0;
	};


	//-------- ������ ������� � ��������� ��������� �������� � ������ [80;110] �� -----------
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

	//-------- ����� ������������ ��������� � ������ [155; 185] �� -----------
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

	//-------- ������ ������� � ��������� 1-� ���������� ������� TBB-320-2 -----------
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

	//-------- ������ ��������� 2-� ���������� ������� TBB-320-2 -----------
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

	//-------- ������ ��������� 3-� ���������� ������� TBB-320-2 -----------
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

	//-------- ������ ��������� 4-� ���������� ������� TBB-320-2 -----------
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

	//-------- ������ ��������� 5-� ���������� ������� TBB-320-2 -----------
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

	//-------- ����� ������������ ��������� � ��������� [1; Fmax] �� -----------
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

	//-------- ������ �������� ������� � ������ [155; 185] �� -----------
	fmin = 155.0;
	fmax = 185.0;
	if ((prog = Evaluation_Power_in_DiapazonSpectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->M_155_185 = Mmax;

	//-------- ������ �������� ������� � ������ [2fm; 10fm) �� -----------
	fmin = 2 * Spec->Fg_100;
	fmax = 10 * Spec->Fg_100 - Par.dF;
	if ((prog = Evaluation_Power_in_DiapazonSpectr(SpM, Par.np, Par.dF, fmin, fmax, &Mmax)) != 0)
	{
		*kod_line = __LINE__; return prog;
	};
	Spec->M[0] = Mmax;

	//-------- ������ �������� ������� � 15 ������� ������ �� 1000 �� -----------
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

	//-------- ������ ��������� �������� �������� �������, ������� fm ��� fm/2 ��, � ������ [2fm; 10fm) �� -----------
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


	//-------- ������ ��������� �������� �������� �������, ������� 50 ��, � 15 ������� �� 1000 �� -----------
	for (i = 1; i <= 15; i++)
	{
		Spec->Mg[i] = 0;
		for (f = i * 10; f < (i + 1) * 10; f += step_fm)
		{
			fmin = f * Spec->Fg_100 - i * Par.win_dF;
			fmax = f * Spec->Fg_100 + i * Par.win_dF;
			if (fmax < Par.Fmax)
			{
				// ������ �������� ��������� ������������ ��������� � ������ ������ [fmin; fmax]
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

	//-------- ������ �������� ������� � ������ [1; 5000) �� -----------
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





// ��������� ������ ����������� �������� � ������� ��������� � �������� ��������� ������ � ������� ��������
//*SpM -  ��������� �� ������ ������������ �������� ������� ��������������
// np - ����� ����� � �������
// dF - ���������� �� ������� � �������
// Flow - ������ ������� ��������� ������
// Fhigh - ������� ������� ��������� ������
// *Mmax -  ��������� �� ������������ �������� �������� ���������
// *FMmax - ��������� �� ������� ������������ ���������
uint16_t Evaluation_Ampl_Freq_in_Spectr(double* SpM, int np, double dF, double Flow, double Fhigh, double* Mmax, double* FMmax)
{
	long
		k,			// ����� ��������� � �������
		n_start,	// ��������� ����� ��������� � �������
		n_end;		// �������� ����� ��������� � �������

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

// ��������� ������ �������� ������� � �������� ��������� ������ �� ������� ��������
// *SpM - ��������� �� ������ ������������ �������� ������� ��������������
// np - ����� ����� � �������
// dF - ���������� �� ������� � �������
// Flow - ������ ������� ��������� ������
// Fhigh - ������� ������� ��������� ������
// *M - ��������� �� �������� �������� �������� ������� � �������� ��������� ������
uint16_t Evaluation_Power_in_DiapazonSpectr(double* SpM, int np, double dF, double Flow, double Fhigh, double* M)
{
	long	k,		// ����� ��������� � �������
		n_start,	// ��������� ����� ��������� � �������
		n_end;		// �������� ����� ��������� � �������

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
