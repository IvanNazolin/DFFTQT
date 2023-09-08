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

// ��������� ���������� ���������� ���������� ����
typedef struct
{
	double 	Mx;			// ������� ��������
	double	Disp;		// ���������
	double	CKO;		// �������������������� ���������� (�� ������� �������� ��������)
	double	CKZ;		// �������������������� �������� (��� ������ �������� ��������
	double	Ax;			// ����������
	double	Ex;			// �������
	double  Ampl;       // ���������
	double  Amax;       // ������������ �� ������ ��������
	double	Pogr_MX;	// ����������� ��������������� ��������
	double	Pogr_Disp;  // ����������� ���������
	double	Pogr_CKO;   // ����������� ��o
	double	Pogr_CKZ;   // ����������� ���
	double	Pogr_Ax;    // ����������� ����������
	double	Pogr_Ex;    // ����������� ��������
}ParamStatistic;

// ��������� ���������� ���������� ��� �������
typedef struct
{
	float      dF;	    // ���������� �� ������� � ������� ���, ��
	float      Fmax;	// ������� ������� ������� ���, ��

	float      Fh;		// ������� ������� ������� ����, ��
	
	float      F_low;	// ������ ������� �������� ��������� ���, ��
	float      F_high;	// ������� ������� �������� ��������� ���, ��

	float 	Mx;			// ������� �������� ��� � ������� ���������
	float	CKO;		// �������������������� ���������� (�� ������� �������� ��������) ��� � ������� ���������
	float	Amax;		// ������������ �������� ��� � ������� ���������
	float	F_Amax;		// ������� ������������� �������� ��� � ������� ���������
	float   Amin;       // ����������� �������� ��� � ������� ���������
	float	F_Amin;		// ������� ������������ �������� ��� � ������� ���������
	float	Ag_160;		// �������� ��� �� ������� ������� ���������� 159,15 ��

	float	G_plus;		// ������������� ��������������� ��� � ������� ���������, ��
	float	G_minus;	// ������������� ��������������� ��� � ������� ���������, ��
	float	G_max;		// ������������ �� ������ ��������������� ��� � ������� ���������, ��

}ParamACHX;

// ��������� ���������� ������������ �������
typedef struct
{
	double 	Fg_100;         // ������ ��������� �������� �������, ������������ �������� 100��
	double 	A100;           // ��� ��������� 100��

	double	Agmax_155_185;	// ��� ��������� ������������ ��������� � ������ [155; 185] ��
	double	Fgmax_155_185;	// ������� ��������� ������������ ��������� � ������ [155; 185] ��
	double	M_155_185;		// �������� ������� � ������ [155; 185] ��

	double	A1_1450;		// ��� ��������� 1-� ���������� �������
	double	A2_2900;		// ��� ��������� 2-� ���������� �������
	double	A3_4350;		// ��� ��������� 3-� ���������� �������
	double  A4_5800;		// ��� ��������� 4-� ���������� �������
	double  A5_7250;		// ��� ��������� 5-� ���������� �������

	double	Agmax;			// ��� ��������� ������������ ��������� � ������ [1; Fmax] ��
	double	Fgmax;			// ������� ��������� ������������ ��������� � ������ [1; Fmax] ��

	double  M[16];			// �������� ������� � 16 ������� �� 1��� � ��������� �� 200�� �� 16���
	double  Mg[16];			// �������� �������� ������� 50�� � 16 ������� �� 1��� � ��������� �� 200�� �� 16���

	double	M_1_5000;		// �������� ������� � ������ [1; 5000] ��
}ParamAmplSpectr;

// ��������� ���������� ������� ���
typedef struct
{
	// ���������, ������������� � ���-204
	uint32_t	Npt;	    // ����� �������� ���������� ����
	uint32_t     Fs;	        // ������� �������������, ��

	// ��������� ������������� ���������� ��� � ���
	uint32_t	n;			// 2^n = ����������� ����� ����� � ��������� ��� 
	uint32_t    npt;	    // ����� ����� ��� ��� ���
	uint32_t    np;	        // ����� �������� � �������
	uint32_t    Offset;	    // ����� �������� �������� ��������� ������ ���  
	uint16_t    TypeSpec;	// ��� ��������������� ������� 1 - ���, 2 - ��, 3 - ��� (double)
	double      win_dF;	    // ������ ������ �������� ������� +-win_dF ��

	// ������������� ���������
	double      dF;	        // ���������� � ������� �� �������
	double      Fmax;	    // ������������ ������� � �������, ��
	uint16_t	nBlocks;	// ����� ����������� ������ ����������
}ParamFFT;

// ��������� ���������� �������� ���-320_Param_16���_���_16384_8��
double* TBB320_Param_16000_16384_8(double* raw, ParamFFT ParFFT, uint32_t* outCount);

// ��������� ���������� �������� ���-1000-2_Param_32���_���_262144_1��
double* TBB1000_ACHX_Param_32000_262144_1(double* raw, ParamFFT ParFFT, uint32_t* outCount);

// ��������� ������� ������������ ��������� �������������� �������
double* ACHX_function(double *a, double *b, ParamFFT ParFFT, int Type_H, uint32_t* outCount);

// ��������� ������� ���������� ��� �������������� �����
uint16_t ACHX_param(double* H, double* F, uint32_t N_data, ParamACHX *Par);

// ������ �������������� ���������� ���������� ����
uint16_t statictica(const double* row, uint32_t N_data, ParamStatistic* Par, long* kod_line);

//  ���������� ��������� ����� Sing()
uint16_t Sing_Win(const double* row, double* row_win, uint32_t n_win, uint32_t N_data, long* kod_line);

//  �������� ������ � ������ ���������� �������������� ���������� ����
uint16_t Make_File_raznoct(const double* row, double* row_pf, uint32_t nmax, uint32_t nmin, uint32_t N_data, long* kod_line);

// ��������� ������� ��� � ���
uint16_t DFTAnalysis(double* gArr, double* data, ParamFFT Par, long* kod_line);

// ��������� ������������ ��������� �������� 
void Estimate_Sym_Spec(double* Sp, double* Re, double* Im, uint32_t np, double df, uint16_t nBlocks, uint16_t Type_spec);

// ��������� ������ ����������� �������� � ������� ��������� � �������� ��������� ������ � ������� ��������
uint16_t Evaluation_Ampl_Freq_in_Spectr(double* SpM, int np, double dF, double Flow, double Fhigh, double* MAmax, double* FMmax);

// ��������� ������ �������� ������� � �������� ��������� ������ �� ������� ��������
uint16_t Evaluation_Power_in_DiapazonSpectr(double* SpM, int np, double dF, double Flow, double Fhigh, double* M);

// ��������� ������� ��������������� ������������ ���������� �� ������� ���-320-2 �� ������� �������� � ������������ ������ �������� ����
uint16_t Diagnostic_Parameters_TBB320(double* SpM, ParamFFT Par, ParamAmplSpectr* Spec, uint16_t step, long* kod_line);

// ��������� ������� ��������������� ������������ ���������� �� ������� ���-320-2 �� ������� �������� � �������������� ������ �������� ����
uint16_t Diagnostic_Parameters_VarFm_TBB320(double* SpM, ParamFFT Par, ParamAmplSpectr* Spec, uint16_t step, long* kod_line);



#ifdef __cplusplus
}
#endif