// stdafx.h: включаемый файл для стандартных системных включаемых файлов
// или включаемых файлов для конкретного проекта, которые часто используются, но
// не часто изменяются
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Исключите редко используемые компоненты из заголовков Windows
// Файлы заголовков Windows:
#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include <shellapi.h>
// Файлы заголовков C RunTime
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string>
#include <vector>
#ifdef _UNICODE 
typedef std::wstring TSTRING;
typedef std::vector<TSTRING> TSTRINGARRAY;
#else
typedef std::string TSTRING;
typedef std::vector<TSTRING> TSTRINGARRAY;
#endif
typedef std::wstring WSTRING;
typedef std::vector<WSTRING> WSTRINGARRAY;
typedef std::string ASTRING;
typedef std::vector<ASTRING> ASTRINGARRAY;
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// TODO: Установите здесь ссылки на дополнительные заголовки, требующиеся для программы
