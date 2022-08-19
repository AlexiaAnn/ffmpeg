#include <stdio.h> //引入C的库函数
#include "MathDll.h"

//宏定义
#define EXPORTBUILD

//相加
int _DLLExport Add(int x, int y)
{
    return x + y;
}

//取较大的值
int _DLLExport Max(int x, int y)
{
    return (x >= y) ? x : y;
}