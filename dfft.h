#ifndef dfft
#define dfft
#include <string.h>
#include "QtWidgetsApplication1\QtWidgetsClassCase2.h"
#include "QtWidgetsApplication1\QtWidgetsClassCase3.h"
#include "QtWidgetsApplication1\QtWidgetsClass.h"
#include "QtWidgetsApplication1/QtWidgetsClassCase6.h"

int case2(std::wstring WorkPath, int Workregim, std::string modelwork);
int case3(std::wstring WorkPath1, QtWidgetsClassCase3 *tq, int parametr11, int parametr31, int parametr41, float parametr51);
int case4(std::wstring WorkPath, QtWidgetsClass *qt,int parametr1,int parametr2,int parametr3,int parametr4,float parametr5, int parametr6);
int case5(std::wstring WorkPathFileA, std::wstring WorkPathFileB, std::wstring scope, QtWidgetsClassCase6* io);
#endif