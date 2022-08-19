#define _DLLExport __attribute__((visibility("default"))) //定义该函数的dll

//代表c风格的
extern "C" int _DLLExport Add(int x, int y);

extern "C" int _DLLExport Max(int x, int y);