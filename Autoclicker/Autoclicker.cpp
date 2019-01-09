// Autoclicker.cpp: Определяет точку входа для приложения.
//Чтобы включить тестовый фунционал нужно определить макрос NOT_FUNCIONAL

#include "stdafx.h"
#include "Autoclicker.h"
#define MAX_LOADSTRING 100
#define MAX_MILISECONDS 1000 // Максимальный интервал кликов 
#define MIN_MILISECONDS 1 // Минимальный интервал кликов 
#define VK_A 0x41
#define VK_B 0x42
#define VK_C 0x43
#define VK_D 0x44
#define VK_E 0x45
#define VK_F 0x46
#define VK_G 0x47
#define VK_H 0x48
#define VK_I 0x49
#define VK_J 0x4A
#define VK_K 0x4B
#define VK_L 0x4C
#define VK_M 0x4D
#define VK_N 0x4E
#define VK_O 0x4F
#define VK_P 0x50
#define VK_Q 0x51
#define VK_R 0x52
#define VK_S 0x53
#define VK_T 0x54
#define VK_U 0x55
#define VK_V 0x56
#define VK_W 0x57 
#define VK_X 0x56
#define VK_Y 0x59
#define VK_Z 0x5A
// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
TCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
TCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
struct ProgrammParameters {
	HWND GlobalHWND = NULL;
	#ifdef APLICATION_HAS_A_TOOLBAR
	HWND hRootToolBar = NULL;
	#endif
	#ifdef APLICATION_HAS_A_STATUSBAR
	HWND hRootStatusBar = NULL;
	#endif
	#ifdef APLICATION_HAS_A_TOOLBAR
	HIMAGELIST hImageListForRootToolbar = NULL;
	#endif
	bool GlobalLeftButtonClick = true; // щёлкать левой кнопкой мыши если выбран переключатель IDC_RADIO_LEFT_BUTTON_CLICK в IDD_SETTINGS 
	bool GlobalRightButtonClick = false;// щёлкать правой кнопкой мыши если выбран переключатель IDC_RADIO_RIGHT_BUTTON_CLICK в IDD_SETTINGS 
	bool ClickMouse = false; // эта переменная становиться true если нажато Ctrl + Alt + Shift + C, и false при повторном нажатии и отвечает за то, будут ли происходить щелчки мышью или нет
	LRESULT GlobalMiliSecondsOfClicks = 10; // глобальная переменная которая содержит интервал кликов
	static enum TimerIds {
		TimerClickId
	}; // в этом перечислении сожержаться идентификаторы таймеров, идентификатр таймера TimerClickId связан с таймером который отправляет сообщение WM_TIMER в соотвествии с интервалом GlobalMiliSecondsOfClicks
	bool TheApplicationHasATrayIcon = false;// эта переменная равна true если приложение имеет иконку в трее
	WORD MaxMiliSeconds = 1000;// максимальное значение интервала кликов
	HHOOK hHookReadMiliSecondsOfClicks = NULL;
	bool HookReadMilisecondsOfClicksIsRegistred = false;
} PP;
BOOL FlipToTray(HWND hWnd, NOTIFYICONDATA *nf);// эта функция сворачивает окно в трей
BOOL UnflipFromTray(HWND hWnd, NOTIFYICONDATA *nf);// эта функция разворачивает окно из трея
// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
#ifdef APLICATION_HAS_A_TOOLBAR
HWND CreateRootToolBar(HWND hWndParent);
#endif
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK PopupWindowShowMiliSecondsWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SettingsDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL ShowPopupMenu(HWND hWnd, HMENU *hm);// эта функция показывает всплывающее меню, она вызываеться если щёлкнуть два раза левой кнопкой мыши по иконке этого приложения в трее
WSTRINGARRAY BreakAStringIntoAnArrayOfStringsByCharacter(WSTRING StringToBreak, WCHAR CharacterToBreak);// функция разбивает строку на массив строк по символу CharacterToBreak
WSTRING DeleteTwoCharactersInARow(WSTRING StringForDeleteTwoCharactersInARow, WCHAR SymbolForDelete);
LRESULT CALLBACK KeyboardProc(_In_ int code, _In_ WPARAM wParam, LPARAM lParam);
VOID CALLBACK TimerProc(_In_ HWND, _In_ UINT, _In_ UINT_PTR, _In_ DWORD);
bool CheckBit(unsigned word, unsigned bit);// функция проверяет установлен ли бит под определённым номером
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    //UNREFERENCED_PARAMETER(lpCmdLine);
	/*INPUT ip;*/

	// Pause for 5 seconds.
	

	// Set up a generic keyboard event.
	//ip.type = INPUT_KEYBOARD;
	//ip.ki.wScan = 0; // hardware scan code for key
	//ip.ki.time = 0;
	//ip.ki.dwExtraInfo = 0;

	//// Press the "A" key
	//ip.ki.wVk = VK_SHIFT; // virtual-key code for the "a" key
	//ip.ki.dwFlags = 0; // 0 for key press
	//SendInput(1, &ip, sizeof(INPUT));
	//ip.ki.wVk = VK_A;
	//SendInput(1, &ip, sizeof(INPUT));
	//ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
	//SendInput(1, &ip, sizeof(INPUT));
	//ip.ki.wVk = VK_SHIFT;
	//SendInput(1, &ip, sizeof(INPUT));

    // TODO: разместите код здесь.
	WSTRINGARRAY CommandLineStrings = BreakAStringIntoAnArrayOfStringsByCharacter(lpCmdLine, L' ');
	for (size_t i = 0; i < CommandLineStrings.size(); i++){
		if(CommandLineStrings[i] == L"--flip-to-tray"){
			PP.TheApplicationHasATrayIcon = true;
			continue;
		}
		else if (CommandLineStrings[i] == L"--click-the-right-mouse-button") {
			PP.GlobalLeftButtonClick = false;
			PP.GlobalRightButtonClick = true;
			continue;
		}
		else if (CommandLineStrings[i] == L"--set-miliseconds-of-clicks") {
			if(i != CommandLineStrings.size() - 1) {
				int MiliSecondsOfClicks = 1;
				try {
					MiliSecondsOfClicks = _wtoi(CommandLineStrings[i + 1].c_str());
				}
				catch (std::exception &e) {
					UNREFERENCED_PARAMETER(e);
					MessageBox(NULL, _TEXT("Неверное значение аргумента командной строки --set-miliseconds-of-clicks, оно должно быть числом!"), _TEXT("Ошибка! Неверное значение аргумента командной строки --set-miliseconds-of-clicks."), MB_ICONERROR | MB_OK);
					return -1;
				}
				if (MiliSecondsOfClicks < 1) {
					MessageBox(NULL, _TEXT("Значение аргумента командной строки --set-miliseconds-of-clicks не может быть меньше 1"), _TEXT("Ошибка! Неверное значение аргумента командной строки --set-miliseconds-of-clicks."), MB_OK | MB_ICONERROR);
					return -1;
				}
				else if (MiliSecondsOfClicks > PP.MaxMiliSeconds) {
					WCHAR ErrorText[100];
					wsprintfW(ErrorText, L"Слишком большое значение аргумента --set-miliseconds-of-clicks, оно не может быть больше чем %u", PP.MaxMiliSeconds);
					MessageBoxW(NULL, ErrorText, L"Слишком большое значение аргумента командной строки --set-miliseconds-of-clicks.", MB_OK | MB_ICONERROR);
					return -1;
				}
				else {
					PP.GlobalMiliSecondsOfClicks = MiliSecondsOfClicks;
					i++;
				}
			}
			else {
				MessageBox(NULL, _TEXT("Недостаточно аргументов командной строки, возможно вы забали указать значение для аргумента --set-miliseconds-of-clicks."), _TEXT("Ошибка! Недостаточно аргументов командной строки!"), MB_OK | MB_ICONERROR);
				return -1;
			}
			continue;
		}
		else if (CommandLineStrings[i] == L"--set-max-miliseconds") {
			if (i != CommandLineStrings.size() - 1) {
				int MaxMiliSeconds = 0;
				try {
					MaxMiliSeconds = _wtoi(CommandLineStrings[i + 1].c_str());
				}
				catch (std::exception &e) {
					UNREFERENCED_PARAMETER(e);
					MessageBox(NULL, _TEXT("Неверное значение аргумента командной строки --set-max-miliseconds, оно должно быть числом!"), _TEXT("Ошибка! Неверное значение аргумента командной строки --set-max-miliseconds."), MB_ICONERROR | MB_OK);
					return -1;
				}
				if (MaxMiliSeconds < 1000) {
					MessageBox(NULL, _TEXT("Значение аргумента командной строки --set-max-miliseconds не может быть меньше 1000"), _TEXT("Ошибка! Неверное значение аргумента командной строки --set-max-miliseconds."), MB_OK | MB_ICONERROR);
					return -1;
				}
				else if (MaxMiliSeconds > USHRT_MAX) {
					WCHAR ErrorText[100];
					wsprintfW(ErrorText, L"Слишком большое значение аргумента --set-max-miliseconds, оно не может быть больше чем %u", USHRT_MAX);
					MessageBoxW(NULL, ErrorText, L"Слишком большое значение аргумента командной строки --set-max-miliseconds.", MB_OK | MB_ICONERROR);
					return -1;
				}
				else {
					PP.MaxMiliSeconds = MaxMiliSeconds;
					i++;
				}
			}
			else {
				MessageBox(NULL, _TEXT("Недостаточно аргументов командной строки, возможно вы забали указать значение для аргумента --set-max-miliseconds."), _TEXT("Ошибка! Недостаточно аргументов командной строки!"), MB_OK | MB_ICONERROR);
				return -1;
			}
		}
		else {
			WSTRING ErrorTextString = L"Неверный аргумент командной строки: \"";
			ErrorTextString += CommandLineStrings[i] + L"\"!";
			MessageBoxW(NULL, ErrorTextString.c_str(), L"Ошибка! Неврный аргумент командной строки!", MB_OK | MB_ICONERROR);
			return -1;
		}
	}
    // Инициализация глобальных строк
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_AUTOCLICKER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_AUTOCLICKER));
    MSG msg;
    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0)){
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int) msg.wParam;
}
BOOL FlipToTray(HWND hWnd, NOTIFYICONDATA *nf) {
	BOOL SNIS = Shell_NotifyIcon(NIM_ADD, nf);
	ShowWindow(hWnd, SW_MINIMIZE);
	if (SNIS)ShowWindow(hWnd, SW_HIDE);
	return SNIS;
}
BOOL UnflipFromTray(HWND hWnd, NOTIFYICONDATA *nf) {
	BOOL SNIS = Shell_NotifyIcon(NIM_DELETE, nf);
	ShowWindow(hWnd, SW_SHOW);
	ShowWindow(hWnd, SW_RESTORE);
	return SNIS;
}
//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  НАЗНАЧЕНИЕ: регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance){
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_AUTOCLICKER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_AUTOCLICKER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_AUTOCLICKER));
	return RegisterClassEx(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   НАЗНАЧЕНИЕ: сохраняет обработку экземпляра и создает главное окно.
//
//   КОММЕНТАРИИ:
//
//        В данной функции дескриптор экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится на экран главное окно программы.
//
#ifdef APLICATION_HAS_A_TOOLBAR
HWND CreateRootToolBar(HWND hWndParent) {
	// Declare and initialize local constants.
	const int ImageListID = 0;
	const int numButtons = 2;
	const int bitmapSize = 16;
	const DWORD buttonStyles = BTNS_AUTOSIZE | TBSTYLE_CHECK | TBSTYLE_GROUP;
	// Create the toolbar.
	HWND hWndToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, WS_CHILD | TBSTYLE_WRAPABLE | WS_VISIBLE, 0, 0, 0, 0, hWndParent, NULL, hInst, NULL);
	if (hWndToolbar == NULL)return NULL;
	// Create the image list.
	PP.hImageListForRootToolbar = ImageList_Create(bitmapSize, bitmapSize,   // Dimensions of individual bitmaps.
		ILC_COLOR24 | ILC_MASK,   // Ensures transparent background.
		numButtons, 0);

	// Set the image list.
	SendMessage(hWndToolbar, TB_SETIMAGELIST, (WPARAM)ImageListID, (LPARAM)PP.hImageListForRootToolbar);
	// Load the button images.
	SendMessage(hWndToolbar, TB_LOADIMAGES, (WPARAM)IDB_STD_SMALL_COLOR, (LPARAM)HINST_COMMCTRL);
	// Initialize button info.
	// IDM_NEW, IDM_OPEN, and IDM_SAVE are application-defined command constants.
	TBBUTTON tbButtons[numButtons] =
	{
		{ MAKELONG(MAKEINTRESOURCE(IDI_LEFT_MOUSE_BUTTON_CLICK),  ImageListID), IDC_RADIO_LEFT_BUTTON_CLICK,  TBSTATE_ENABLED, buttonStyles, { 0 }, 0, (INT_PTR)_TEXT("Кликать левой") },
	{ MAKELONG(MAKEINTRESOURCE(IDI_RIGHT_MOUSE_BUTTON_CLICK), ImageListID), IDC_RADIO_RIGHT_BUTTON_CLICK, TBSTATE_ENABLED, buttonStyles, { 0 }, 0, (INT_PTR)_TEXT("Кликать правой") },
	};
	// Add buttons.
	SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
	SendMessage(hWndToolbar, TB_ADDBUTTONS, (WPARAM)numButtons, (LPARAM)&tbButtons);
	// Resize the toolbar, and then show it.
	SendMessage(hWndToolbar, TB_AUTOSIZE, 0, 0);
	ShowWindow(hWndToolbar, TRUE);
	UpdateWindow(hWndToolbar);
	//ImageList_Destroy(g_hImageList);
	return hWndToolbar;
}
#endif
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить дескриптор экземпляра в глобальной переменной
   HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_MINIMIZE, CW_USEDEFAULT, 0,
	   500, 500, nullptr, nullptr, hInstance, nullptr);
   if (!hWnd){
	   DWORD RHKError = GetLastError();
	   LPTSTR BufferForFormatMessage = nullptr;
	   DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
	   if (FMResult == 0) {
		   MessageBox(hWnd, _TEXT("Ошибка! Не удалось создать главное окно!"), _TEXT("Ошибка создания нлвного окна!"), MB_OK | MB_ICONERROR);
		   MessageBox(hWnd, _TEXT("Не удалось узнать причину ошибки создания главного окна!"), _TEXT("Мне не удалось причину возникновения ошибки!"), MB_OK | MB_ICONERROR);
	   }
	   else {
		   MessageBox(hWnd, BufferForFormatMessage, _TEXT("Ошибка создания главного окна!"), MB_OK | MB_ICONERROR);
	   }
	   if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
		   MessageBox(hWnd, _TEXT("Не удалось освободить буфуер, при обработке ошибки создания главного окна!"), _TEXT("Ошибка освобождения буфера!"), MB_OK | MB_ICONERROR);
	   }
      return FALSE;
   }
   if (PP.TheApplicationHasATrayIcon)SendMessage(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, NULL);
   else{
	   ShowWindow(hWnd, nCmdShow);
	   if (UpdateWindow(hWnd) == NULL) {
		   DWORD RHKError = GetLastError();
		   LPTSTR BufferForFormatMessage = nullptr;
		   DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
		   if (FMResult == 0) {
			   MessageBox(hWnd, _TEXT("Ошибка выполнения функции UpdateWindow!"), _TEXT("Ошибка выполнения функции!"), MB_OK | MB_ICONERROR);
			   MessageBox(hWnd, _TEXT("Не удалось узнать причину ошибки выполнения функции UpdateWindow!"), _TEXT("Мне не удалось причину возникновения ошибки!"), MB_OK | MB_ICONERROR);
		   }
		   else {
			   MessageBox(hWnd, BufferForFormatMessage, _TEXT("Ошибка выполнения функции UpdateWindow!"), MB_OK | MB_ICONERROR);
		   }
		   if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
			   MessageBox(hWnd, _TEXT("Не удалось освободить буфуер, при обработке ошибки выполнения функции UpdateWindow!"), _TEXT("Ошибка освобождения буфера!"), MB_OK | MB_ICONERROR);
		   }
	   } 
   }
   #ifdef APLICATION_HAS_A_TOOLBAR
   PP.hRootToolBar = CreateRootToolBar(hWnd);
   if (PP.hRootToolBar == NULL) {
	   DWORD RHKError = GetLastError();
	   LPTSTR BufferForFormatMessage = nullptr;
	   DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
	   if (FMResult == 0) {
		   MessageBox(hWnd, _TEXT("Ошибка! Не удалось создать панель иструментов для главного окна!"), _TEXT("Ошибка создания панели инструментов!"), MB_OK | MB_ICONERROR);
		   MessageBox(hWnd, _TEXT("Не удалось узнать причину ошибки создания панели инструментов для главного окна!"), _TEXT("Мне не удалось причину возникновения ошибки!"), MB_OK | MB_ICONERROR);
	   }
	   else {
		   MessageBox(hWnd, BufferForFormatMessage, _TEXT("Ошибка создания панели инструментов для главного окна!"), MB_OK | MB_ICONERROR);
	   }
	   if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
		   MessageBox(hWnd, _TEXT("Не удалось освободить буфуер, при обработке ошибки создания панели инструментов для главного окна!"), _TEXT("Ошибка освобождения буфера!"), MB_OK | MB_ICONERROR);
	   }
   }
   else {
	   if(PP.GlobalLeftButtonClick)SendMessage(PP.hRootToolBar, TB_SETSTATE, IDC_RADIO_LEFT_BUTTON_CLICK, TBSTATE_CHECKED | TBSTATE_ENABLED);
	   else SendMessage(PP.hRootToolBar, TB_SETSTATE, IDC_RADIO_RIGHT_BUTTON_CLICK, TBSTATE_CHECKED | TBSTATE_ENABLED);
   }
   #endif
   PP.GlobalHWND = hWnd;
   return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  НАЗНАЧЕНИЕ:  обрабатывает сообщения в главном окне.
//
//  WM_COMMAND — обработать меню приложения
//  WM_PAINT — отрисовать главное окно
//  WM_DESTROY — отправить сообщение о выходе и вернуться
//
//

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	
	static struct HotKeys {// структура описывает состояние горячих клавиш и их идентификаторы
		enum HotKeysList{
			HotKeyClickStartId, HotKeySettingsId, HotKeyCloseId, HotKeyRightButtonClickId, HotKeyLeftButtonClickId, HotKeySetIntervalOfClicksId
		}; // перечисление горячих клавиш, HotKeyClickStartId - идентификатор горячей клавиши которая отвечает за присваивания переменной ClickMouse значения true и установки таймера с идентификатором TimerIds::TimerClickId,
		   //HotKeySettingsId - идентификатор горячей клавиши которая отвечает за показ диалога настроек;
		   //HotKeyRightButtonClick - горячая клавиша с этим идентификатором, установит значение переменных PP.GlobalLeftButtonClick = false; PP.GlobalRightButtonClick = true;
		   //HotKeyLeftButtonClick - горячая клавиша с этим идентификатором, установит значение переменных PP.GlobalLeftButtonClick = true; PP.GlobalRightButtonClick = false;
		bool HotKeyClickStartIsRegistred = false;// была ли горячая клавиша с ИД HotKeyClickStartId зарегистрированна
		bool HotKeySettingsIsRegistred = false;// была ли горячая клавиша с ИД HotKeySettingsId зарегистрированна
		bool HotKeyCloseIsRegistred = false;// была ли горячая клавиша с ИД HotKeyCloseId зарегистрированна
		bool HotKeyRightButtonClickIsRegistred = false;// была ли горячая клавиша с ИД HotKeyRightButtonClickId зарегистрированна
		bool HotKeyLeftButtonClickIsRegistred = false;// была ли горячая клавиша с ИД HotKeyLeftButtonClickId зарегистрированна
		bool HotKeySetIntervalOfClicksIsRegistred = false;// была ли горячая клавиша с ИД HotKeySetIntervalOfClicksId зарегистрированна
	} HK;
	RECT rect;
	static HFONT hFont;
	static NOTIFYICONDATA nf;
	static HWND hRootStatusBar;
	static HWND hRootToolBarStatus;
	TCHAR FontName[7] = _TEXT("Tahoma");
	LOGFONT lf;
    switch (message){
		case WM_MESSAGE_FROM_TRAY:{// обработка сообщений от иконки в трее
			switch (lParam) {
				case WM_LBUTTONDBLCLK:// если по иконке в трее кликнули два раза
					PP.TheApplicationHasATrayIcon = !UnflipFromTray(hWnd, &nf);// показать окно и убрать иконку из трея 
					break;
				case WM_RBUTTONDOWN:{// если была нажата правая кнокпа мыши
					HMENU hCMenu = CreatePopupMenu();
					TCHAR TextForMenuPunkt[200];
					LoadString(hInst, IDS_SETTINGS, TextForMenuPunkt, 200);
					AppendMenu(hCMenu, MF_ENABLED | MF_STRING, ID_SETTINGS, TextForMenuPunkt);
					LoadString(hInst, IDS_ABOUT, TextForMenuPunkt, 200);
					AppendMenu(hCMenu, MF_ENABLED | MF_STRING, IDM_ABOUT, TextForMenuPunkt);
					LoadString(hInst, IDS_QUIT, TextForMenuPunkt, 200);
					AppendMenu(hCMenu, MF_ENABLED | MF_STRING, IDM_EXIT, TextForMenuPunkt);
					ShowPopupMenu(hWnd, &hCMenu);// показать всплывающее меню
					DestroyMenu(hCMenu);
					break;
				}	
			}
			break;
		}
		case WM_SYSCOMMAND:{
			switch (wParam) {
				case SC_MINIMIZE:// обработка нажатии кнопки "Свернуть окно"
					PP.TheApplicationHasATrayIcon = FlipToTray(hWnd, &nf);
					break;
				default:// обработка всех остальных сообщений
					DefWindowProc(hWnd, message, wParam, lParam);
					break;
			}
			break;
		}
		case WM_COMMAND:{
				int wmId = LOWORD(wParam);
				// Разобрать выбор в меню:
				switch (wmId){
					#ifdef APLICATION_HAS_A_TOOLBAR
					case IDC_RADIO_LEFT_BUTTON_CLICK:
						PP.GlobalRightButtonClick = false;
						PP.GlobalLeftButtonClick = true;
						break;
					case IDC_RADIO_RIGHT_BUTTON_CLICK:
						PP.GlobalRightButtonClick = true;
						PP.GlobalLeftButtonClick = false;
						break;
					#endif
					case IDM_ABOUT:
						DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
						break;
					case IDM_EXIT:
						DestroyWindow(hWnd);
						break;
					case ID_SETTINGS:
						DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGS), hWnd, SettingsDlgProc);
						InvalidateRect(hWnd, NULL, TRUE);
						break;
					default:
						return DefWindowProc(hWnd, message, wParam, lParam);
				}
				break;
		}
		case WM_CREATE:{
			if (RegisterHotKey(hWnd, HotKeys::HotKeyClickStartId, MOD_CONTROL | MOD_ALT | MOD_SHIFT, VK_C) == 0) {
				DWORD RHKError = GetLastError();
				LPTSTR BufferForFormatMessage = nullptr;
				DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
				if (FMResult == 0) {
					MessageBox(hWnd, _TEXT("Ошибка! Не удалось зарегистрировать горячую клавишу Ctrl+Alt+Shift+C"), _TEXT("Не удалось зарегистрировать горячую клавишу!"), MB_OK | MB_ICONERROR);
					MessageBox(hWnd, _TEXT("Не удалось узнать причину ошибки регистрации горячей клавиши Ctrl+Alt+Shift+C!"), _TEXT("Мне не удалось причину возникновения ошибки"), MB_OK | MB_ICONERROR);
				}
				else {
					MessageBox(hWnd, BufferForFormatMessage, _TEXT("Не удалось зарегистрировать горячую клавишу Ctrl+Alt+Shift+C!"), MB_OK | MB_ICONERROR);
				}
				if (LocalFree((HLOCAL)BufferForFormatMessage) != 0){
					MessageBox(hWnd, _TEXT("Не удалось освободить буфуер при обработке ошибки регистрации горячей клавиши Ctrl+Alt+Shift+C!"), _TEXT("Ошибка!"), MB_OK | MB_ICONERROR);
				}
			}
			else {
				HK.HotKeyClickStartIsRegistred = true;
			}
			if (RegisterHotKey(hWnd, HotKeys::HotKeySettingsId, MOD_CONTROL | MOD_ALT | MOD_SHIFT, VK_S) == 0) {
				DWORD RHKError = GetLastError();
				LPTSTR BufferForFormatMessage = nullptr;
				DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
				if (FMResult == 0) {
					MessageBox(hWnd, _TEXT("Ошибка! Не удалось зарегистрировать горячую клавишу Ctrl+Alt+Shift+S"), _TEXT("Не удалось зарегистрировать горячую клавишу!"), MB_OK | MB_ICONERROR);
					MessageBox(hWnd, _TEXT("Не удалось узнать причину ошибки регистрации горячей клавиши Ctrl+Alt+Shift+S!"), _TEXT("Мне не удалось причину возникновения ошибки"), MB_OK | MB_ICONERROR);
				}
				else {
					MessageBox(hWnd, BufferForFormatMessage, _TEXT("Не удалось зарегистрировать горячую клавишу Ctrl+Alt+Shift+S!"), MB_OK | MB_ICONERROR);
				}
				if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
					MessageBox(hWnd, _TEXT("Не удалось освободить буфуер при обработке ошибки регистрации горячей клавиши Ctrl+Alt+Shift+S!"), _TEXT("Ошибка!"), MB_OK | MB_ICONERROR);
				}
			}
			else {
				HK.HotKeySettingsIsRegistred = true;
			}
			if (RegisterHotKey(hWnd, HotKeys::HotKeyCloseId, MOD_CONTROL | MOD_ALT | MOD_SHIFT, VK_Q) == 0) {
				DWORD RHKError = GetLastError();
				LPTSTR BufferForFormatMessage = nullptr;
				DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
				if (FMResult == 0) {
					MessageBox(hWnd, _TEXT("Ошибка! Не удалось зарегистрировать горячую клавишу Ctrl+Alt+Shift+Q"), _TEXT("Не удалось зарегистрировать горячую клавишу!"), MB_OK | MB_ICONERROR);
					MessageBox(hWnd, _TEXT("Не удалось узнать причину ошибки регистрации горячей клавиши Ctrl+Alt+Shift+Q!"), _TEXT("Мне не удалось причину возникновения ошибки!"), MB_OK | MB_ICONERROR);
				}
				else {
					MessageBox(hWnd, BufferForFormatMessage, _TEXT("Не удалось зарегистрировать горячую клавишу Ctrl+Alt+Shift+Q!"), MB_OK | MB_ICONERROR);
				}
				if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
					MessageBox(hWnd, _TEXT("Не удалось освободить буфуер при обработке ошибки регистрации горячей клавиши Ctrl+Alt+Shift+Q!"), _TEXT("Ошибка!"), MB_OK | MB_ICONERROR);
				}
			}
			else {
				HK.HotKeyCloseIsRegistred = true;
			}
			if (RegisterHotKey(hWnd, HotKeys::HotKeyRightButtonClickId, MOD_CONTROL | MOD_ALT | MOD_SHIFT, VK_R) == 0) {
				DWORD RHKError = GetLastError();
				LPTSTR BufferForFormatMessage = nullptr;
				DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
				if (FMResult == 0) {
					MessageBox(hWnd, _TEXT("Ошибка! Не удалось зарегистрировать горячую клавишу Ctrl+Alt+Shift+R"), _TEXT("Не удалось зарегистрировать горячую клавишу!"), MB_OK | MB_ICONERROR);
					MessageBox(hWnd, _TEXT("Не удалось узнать причину ошибки регистрации горячей клавиши Ctrl+Alt+Shift+R!"), _TEXT("Мне не удалось причину возникновения ошибки!"), MB_OK | MB_ICONERROR);
				}
				else {
					MessageBox(hWnd, BufferForFormatMessage, _TEXT("Не удалось зарегистрировать горячую клавишу Ctrl+Alt+Shift+R!"), MB_OK | MB_ICONERROR);
				}
				if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
					MessageBox(hWnd, _TEXT("Не удалось освободить буфуер при обработке ошибки регистрации горячей клавиши Ctrl+Alt+Shift+R!"), _TEXT("Ошибка!"), MB_OK | MB_ICONERROR);
				}
			}
			else {
				HK.HotKeyRightButtonClickIsRegistred = true;
			}
			if (RegisterHotKey(hWnd, HotKeys::HotKeyLeftButtonClickId, MOD_CONTROL | MOD_ALT | MOD_SHIFT, VK_E) == 0) {
				DWORD RHKError = GetLastError();
				LPTSTR BufferForFormatMessage = nullptr;
				DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
				if (FMResult == 0) {
					MessageBox(hWnd, _TEXT("Ошибка! Не удалось зарегистрировать горячую клавишу Ctrl+Alt+Shift+L"), _TEXT("Не удалось зарегистрировать горячую клавишу!"), MB_OK | MB_ICONERROR);
					MessageBox(hWnd, _TEXT("Не удалось узнать причину ошибки регистрации горячей клавиши Ctrl+Alt+Shift+L!"), _TEXT("Мне не удалось причину возникновения ошибки!"), MB_OK | MB_ICONERROR);
				}
				else {
					MessageBox(hWnd, BufferForFormatMessage, _TEXT("Не удалось зарегистрировать горячую клавишу Ctrl+Alt+Shift+L!"), MB_OK | MB_ICONERROR);
				}
				if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
					MessageBox(hWnd, _TEXT("Не удалось освободить буфуер при обработке ошибки регистрации горячей клавиши Ctrl+Alt+Shift+L!"), _TEXT("Ошибка!"), MB_OK | MB_ICONERROR);
				}
			}
			else {
				HK.HotKeyLeftButtonClickIsRegistred = true;
			}
			if (RegisterHotKey(hWnd, HotKeys::HotKeySetIntervalOfClicksId, MOD_CONTROL | MOD_ALT | MOD_SHIFT, VK_Z) == 0) {
				DWORD RHKError = GetLastError();
				LPTSTR BufferForFormatMessage = nullptr;
				DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
				if (FMResult == 0) {
					MessageBox(hWnd, _TEXT("Ошибка! Не удалось зарегистрировать горячую клавишу Ctrl+Alt+Shift+Z"), _TEXT("Не удалось зарегистрировать горячую клавишу!"), MB_OK | MB_ICONERROR);
					MessageBox(hWnd, _TEXT("Не удалось узнать причину ошибки регистрации горячей клавиши Ctrl+Alt+Shift+Z!"), _TEXT("Мне не удалось причину возникновения ошибки!"), MB_OK | MB_ICONERROR);
				}
				else {
					MessageBox(hWnd, BufferForFormatMessage, _TEXT("Не удалось зарегистрировать горячую клавишу Ctrl+Alt+Shift+Z!"), MB_OK | MB_ICONERROR);
				}
				if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
					MessageBox(hWnd, _TEXT("Не удалось освободить буфуер при обработке ошибки регистрации горячей клавиши Ctrl+Alt+Shift+Z!"), _TEXT("Ошибка!"), MB_OK | MB_ICONERROR);
				}
			}
			else {
				HK.HotKeySetIntervalOfClicksIsRegistred = true;
			}
			memset(&lf, 0, sizeof(lf));
			lf.lfHeight = 20;
			lf.lfCharSet = RUSSIAN_CHARSET;
			lstrcpy(lf.lfFaceName, FontName);
			hFont = CreateFontIndirect(&lf);
			nf.cbSize = sizeof(NOTIFYICONDATA);
			nf.hWnd = hWnd;
			nf.uID = NULL;
			lstrcpy(nf.szTip, szTitle);
			lstrcpy(nf.szInfoTitle, szTitle);
			nf.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
			nf.uCallbackMessage = WM_MESSAGE_FROM_TRAY;
			nf.hIcon = nf.hBalloonIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_AUTOCLICKER));
			break;
		}
		case WM_PAINT:{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			TCHAR szText[200];
			// TODO: Добавьте сюда любой код прорисовки, использующий HDC...
			wsprintf(szText, _TEXT("Интервал кликов: %u\n Чтобы запустить автокликер нажмите Ctrl + Alt + Shift + C\n Чтбы открыть настройки нажмете Ctrl + Alt + Shift + S\0"), PP.GlobalMiliSecondsOfClicks);
			GetClientRect(hWnd, &rect);
			SelectObject(hdc, hFont);
			DrawText(hdc, szText, -1, &rect, DT_CENTER | DT_VCENTER);
			EndPaint(hWnd, &ps);
			break;
		}
		case WM_HOTKEY:
			switch (wParam) {
				case HotKeys::HotKeyClickStartId:
					if (!PP.ClickMouse) {
						PP.ClickMouse = true;
						SetTimer(hWnd, ProgrammParameters::TimerIds::TimerClickId, PP.GlobalMiliSecondsOfClicks, TimerProc);
					}
					else {
						PP.ClickMouse = false;
						KillTimer(hWnd, ProgrammParameters::TimerIds::TimerClickId);
					}
					break;
				case HotKeys::HotKeySettingsId:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGS), NULL, SettingsDlgProc);
					InvalidateRect(hWnd, NULL, TRUE);
					break;
				case HotKeys::HotKeyCloseId:
					DestroyWindow(hWnd);
					break;
				case HotKeys::HotKeyLeftButtonClickId:
					PP.GlobalLeftButtonClick = true;
					PP.GlobalRightButtonClick = false;
					#ifdef APLICATION_HAS_A_TOOLBAR
					if(PP.hRootToolBar != NULL){ 
						SendMessage(PP.hRootToolBar, TB_SETSTATE, IDC_RADIO_RIGHT_BUTTON_CLICK, TBSTATE_ENABLED);
						SendMessage(PP.hRootToolBar, TB_SETSTATE, IDC_RADIO_LEFT_BUTTON_CLICK, TBSTATE_CHECKED | TBSTATE_ENABLED);
					}
					#endif
					break;
				case HotKeys::HotKeyRightButtonClickId:
					PP.GlobalLeftButtonClick = false;
					PP.GlobalRightButtonClick = true;
					#ifdef APLICATION_HAS_A_TOOLBAR
					if (PP.hRootToolBar != NULL) {
						SendMessage(PP.hRootToolBar, TB_SETSTATE, IDC_RADIO_LEFT_BUTTON_CLICK, TBSTATE_ENABLED);
						SendMessage(PP.hRootToolBar, TB_SETSTATE, IDC_RADIO_RIGHT_BUTTON_CLICK, TBSTATE_CHECKED | TBSTATE_ENABLED);
					}
					#endif
					break;
				case HotKeys::HotKeySetIntervalOfClicksId:
					// данный участок кода предоствращает баг с залипанием клавиш Ctrl, Alt, Shift
					LabelBeginForCheckKeyStates:
					if (CheckBit(GetAsyncKeyState(VK_CONTROL), 16) == true || CheckBit(GetAsyncKeyState(VK_SHIFT), 16) == true || CheckBit(GetAsyncKeyState(VK_MENU), 16)) {// данное условие проверяет нажата какая-либо из клавиш Ctrl, Alt, Shift
						Sleep(500);
						goto LabelBeginForCheckKeyStates;
					}
					else {
						goto LabelEndForCheckKeyStates;
					}
					LabelEndForCheckKeyStates:
					if (PP.HookReadMilisecondsOfClicksIsRegistred == true) {
						UnhookWindowsHookEx(PP.hHookReadMiliSecondsOfClicks);
						return (LRESULT)FALSE;
					}
					//конец данного участка
					PP.hHookReadMiliSecondsOfClicks = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInst, NULL);
					if (PP.hHookReadMiliSecondsOfClicks == NULL) {
						DWORD RHKError = GetLastError();
						LPTSTR BufferForFormatMessage = nullptr;
						DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
						if (FMResult == 0) {
							MessageBox(hWnd, _TEXT("Не удалось выполнить функцию SetWindowsHookEx!"), _TEXT("Ошибка при выполнении функции SetWindowsHookEx!"), MB_OK | MB_ICONERROR);
							MessageBox(hWnd, _TEXT("Ошибка! Не удалось узнать причину возникновения ошибки при выполнении функции SetWindowsHookEx!"), _TEXT("Мне не удалось причину возникновения ошибки!"), MB_OK | MB_ICONERROR);
									
						}
						else {
							MessageBox(hWnd, BufferForFormatMessage, _TEXT("Не удалось выполнить функцию SetWindowsHookEx!"), MB_OK | MB_ICONERROR);
						}
						if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
							MessageBox(hWnd, _TEXT("Не удалось освободить буфуер при обработке ошибки выполнения функции SetWindowsHookEx!"), _TEXT("Ошибка!"), MB_OK | MB_ICONERROR);
						}
					}
					else {
						
					}
					return (LRESULT)TRUE;
			}
			break;
		case WM_DESTROY:{
			if(HK.HotKeyClickStartIsRegistred)UnregisterHotKey(hWnd, HotKeys::HotKeyClickStartId);
			if(HK.HotKeySettingsIsRegistred)UnregisterHotKey(hWnd, HotKeys::HotKeySettingsId);
			if(HK.HotKeyCloseIsRegistred)UnregisterHotKey(hWnd, HotKeys::HotKeyCloseId);
			if(HK.HotKeyLeftButtonClickIsRegistred)UnregisterHotKey(hWnd, HotKeys::HotKeyLeftButtonClickId);
			if(HK.HotKeyRightButtonClickIsRegistred)UnregisterHotKey(hWnd, HotKeys::HotKeyRightButtonClickId);
			if(HK.HotKeySetIntervalOfClicksIsRegistred)UnregisterHotKey(hWnd, HotKeys::HotKeySetIntervalOfClicksId);
			if(PP.HookReadMilisecondsOfClicksIsRegistred)UnhookWindowsHookEx(PP.hHookReadMiliSecondsOfClicks);
			if(PP.TheApplicationHasATrayIcon)Shell_NotifyIcon(NIM_DELETE, &nf);
			#ifdef APLICATION_HAS_A_TOOLBAR
			if (PP.hRootToolBar != NULL)ImageList_Destroy(PP.hImageListForRootToolbar);
			#endif
			DestroyIcon(nf.hIcon);
			DestroyIcon(nf.hBalloonIcon);
			PostQuitMessage(0);
			break;
		}
		default: 
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
    return 0;
}

LRESULT CALLBACK PopupWindowShowMiliSecondsWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	RECT rect;
	static HFONT hFont;
	TCHAR FontName[7] = _TEXT("Tahoma");
	LOGFONT lf;
	switch (message) {
		case WM_CREATE:{
			memset(&lf, 0, sizeof(lf));
			lf.lfHeight = 20;
			lf.lfCharSet = RUSSIAN_CHARSET;
			lstrcpy(lf.lfFaceName, FontName);
			hFont = CreateFontIndirect(&lf);
			return TRUE;
		}
		case WM_PAINT:
			break;
		case WM_SETTEXT:{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			//InvalidateRect(hWnd, &rect, TRUE);
			GetClientRect(hWnd, &rect);
			SelectObject(hdc, hFont);
			if (DrawText(hdc, (LPTSTR)lParam, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE) == NULL) {
				DWORD RHKError = GetLastError();
				LPTSTR BufferForFormatMessage = nullptr;
				DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
				if (FMResult == 0) {
					OutputDebugString(_TEXT("Не удалось выполнить функцию DrawText в оконной функции PopupWindowShowMiliSecondsWndProc!\n"));
					OutputDebugString(_TEXT("Мне не удалось причину возникновения ошибки!\n"));
				}
				else {
					OutputDebugString(_TEXT("Не удалось выполнить функцию DrawText в оконной функции PopupWindowShowMiliSecondsWndProc!"));
					OutputDebugString(BufferForFormatMessage);
				}
				if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
					OutputDebugString(_TEXT("Не удалось освободить буфуер при обработке ошибки выполнения функции DrawText в оконной функции PopupWindowShowMiliSecondsWndProc!"));
				}
				return FALSE;
			}
			//ValidateRect(hWnd, &rect);
			EndPaint(hWnd, &ps);
			return TRUE;
		}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
    UNREFERENCED_PARAMETER(lParam);
    switch (message){
		case WM_INITDIALOG:
			return (INT_PTR)TRUE;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL){
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
    }
    return (INT_PTR)FALSE;
}
INT_PTR CALLBACK SettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	HWND hFrequencyOfClicks = GetDlgItem(hDlg, IDC_SLIDER_FREQUENCY_OF_CLICKS);
	HWND hLeftClcikRadioButton = GetDlgItem(hDlg, IDC_RADIO_LEFT_BUTTON_CLICK);
	HWND hRightClcikRadioButton = GetDlgItem(hDlg, IDC_RADIO_RIGHT_BUTTON_CLICK);
	TCHAR szText[500];//текст который отображаеться в IDC_MILLISECONDS
	static LRESULT InFuncMiliSecondsOfClicks = PP.GlobalMiliSecondsOfClicks; // локальная переменная аналог глобальной GlobalMiliSecondsOfClicks,
	// в эту переменную пишеться значение которое пользователь выбрал с помщью элемента IDC_SLIDER_FREQUENCY_OF_CLICKS
	// значение этой переменной записываються в GlobalMiliSecondsOfClicks после нажатия кнопки OK в IDD_SETTINGS
	static bool InFuncRightButtonClick = PP.GlobalRightButtonClick;
	static bool InFuncLeftButtonClick = PP.GlobalLeftButtonClick;
	//UINT iMin = MIN_MILISECONDS;// мнимальное и максимальное значение слайдера IDC_SLIDER_FREQUENCY_OF_CLICKS, который предназначен для выбора
	// интервала кликов, который потом пишеться в переменную InFuncMiliSecondsOfClicks
	UINT_PTR TimerId = 1; // идентификатор таймера который отвечает за динамическое отображение
	//текста в диалоге IDD_SETTINGS в элементе IDC_MILLISECONDS
	UINT uElapse = 10; // время срабатывания таймера оно относится к таймеру с ID TimerId
	switch(message){
		case WM_INITDIALOG:
			SendMessage(hFrequencyOfClicks, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(MIN_MILISECONDS, PP.MaxMiliSeconds));
			SendMessage(hFrequencyOfClicks, TBM_SETPAGESIZE, 0, (LPARAM)10);
			SendMessage(hFrequencyOfClicks, TBM_SETSEL, (WPARAM)FALSE, (WPARAM)MAKELONG(MIN_MILISECONDS, PP.MaxMiliSeconds));
			SendMessage(hFrequencyOfClicks, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)PP.GlobalMiliSecondsOfClicks);
			//InFuncMiliSecondsOfClicks = SendMessage(hFrequencyOfClicks, TBM_GETPOS, NULL, NULL);
			wsprintf(szText, _TEXT("Милисекунды %u"), InFuncMiliSecondsOfClicks);
			SetDlgItemText(hDlg, IDC_MILLISECONDS, szText);
			if (PP.GlobalRightButtonClick) {
				SendMessage(hRightClcikRadioButton, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			}
			else{
				SendMessage(hLeftClcikRadioButton, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			}
			//SetTimer(hDlg, TimerId, uElapse, NULL);
			return (INT_PTR)TRUE;
		case WM_HSCROLL:
			if (lParam == (LPARAM)hFrequencyOfClicks) {
				InFuncMiliSecondsOfClicks = SendMessage(hFrequencyOfClicks, TBM_GETPOS, NULL, NULL);
				wsprintf(szText, _TEXT("Милисекунды %u"), InFuncMiliSecondsOfClicks);
				SetDlgItemText(hDlg, IDC_MILLISECONDS, szText);
			} 
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)){
				case IDOK:
					PP.GlobalMiliSecondsOfClicks = InFuncMiliSecondsOfClicks;
					PP.GlobalLeftButtonClick = InFuncLeftButtonClick;
					PP.GlobalRightButtonClick = InFuncRightButtonClick;
					KillTimer(hDlg, TimerId);
					EndDialog(hDlg, 0);
					break;
				case IDCANCEL:
					KillTimer(hDlg, TimerId);
					EndDialog(hDlg, 0);
					break;
				case IDC_SLIDER_FREQUENCY_OF_CLICKS:
					switch (HIWORD(wParam)) {
						case TB_THUMBTRACK:
							InFuncMiliSecondsOfClicks = SendMessage(hFrequencyOfClicks, TBM_GETPOS, NULL, NULL);
							wsprintf(szText, _TEXT("Милисекунды %u"), InFuncMiliSecondsOfClicks);
							SetDlgItemText(hDlg, IDC_MILLISECONDS, szText);
							break;
					}
					break;
				case IDC_RADIO_LEFT_BUTTON_CLICK:
					if (IsDlgButtonChecked(hDlg, IDC_RADIO_LEFT_BUTTON_CLICK) == BST_CHECKED) {
						InFuncLeftButtonClick = true;
						InFuncRightButtonClick = false;
					}
					break;
				case IDC_RADIO_RIGHT_BUTTON_CLICK:
					if (IsDlgButtonChecked(hDlg, IDC_RADIO_RIGHT_BUTTON_CLICK) == BST_CHECKED) {
						InFuncRightButtonClick = true;
						InFuncLeftButtonClick = false;
					}
					break;
			}
			break;
	}
	return (INT_PTR)FALSE;
}
BOOL ShowPopupMenu(HWND hWnd, HMENU *hm) {// функция показывает всплывающее меню
	if (!*hm) return FALSE;
	SetForegroundWindow(hWnd);
	POINT pt;
	GetCursorPos(&pt);
	BOOL bOK = TrackPopupMenu(*hm, 0, pt.x, pt.y, 0, hWnd, NULL);
	return bOK;
}
WSTRINGARRAY BreakAStringIntoAnArrayOfStringsByCharacter(WSTRING StringToBreak, WCHAR CharacterToBreak){// данная функция разбивает входную строку StringToBreak на массив строк по заданному символу CharacterToBreak
	WSTRING StringForProcessing = DeleteTwoCharactersInARow(StringToBreak, CharacterToBreak);
	WSTRINGARRAY TSA;
	if (StringToBreak == L"") {
		return TSA;
	}
	if (StringForProcessing[0] == CharacterToBreak) {
		StringForProcessing.erase(StringForProcessing.begin());
	}
	if (StringForProcessing[StringForProcessing.size() - 1] == CharacterToBreak) {
		StringForProcessing.erase(StringForProcessing.end());
	}
	WSTRING ts;
	size_t StringForProcessingSize = StringForProcessing.size(), StringForProcessingSizeSmallerByOne = StringForProcessingSize - 1;
	for (size_t i = 0; i < StringForProcessingSize; i++) {
		if ((StringForProcessing[i] == CharacterToBreak)) {
			TSA.push_back(ts);
			ts.clear();
		}
		else if (i == StringForProcessingSizeSmallerByOne) {
			ts += StringForProcessing[i];
			TSA.push_back(ts);
			ts.clear();
		}
		else {
			ts += StringForProcessing[i];
		}
	}
	return TSA;
}
WSTRING DeleteTwoCharactersInARow(WSTRING StringForDeleteTwoCharactersInARow, WCHAR SymbolForDelete) {
	WSTRING ResultString;
	size_t StringSize = StringForDeleteTwoCharactersInARow.size();
	ResultString += StringForDeleteTwoCharactersInARow[0];
	size_t Counter = 0;
	for (size_t i = 1; i < StringForDeleteTwoCharactersInARow.size(); i++) {
		if (ResultString[Counter] == StringForDeleteTwoCharactersInARow[i] == SymbolForDelete)continue;
		else {
			ResultString += StringForDeleteTwoCharactersInARow[i];
			Counter++;
		}
	}
	return ResultString;
}

LRESULT CALLBACK KeyboardProc(_In_ int code, _In_ WPARAM wParam, _In_ LPARAM lParam) {// функция для перехвата клавиатуры, перехват будет происходить при зажатии Ctrl+Alt+Shift+Z, будут перехватываться клавиши с цифрами 0-9, это нужно для того чтобы ввести новый интервал кликов не открывая настройки автокликера
	static TSTRING StringValueMilisecondsOfClick;
	static bool PopupWindowShowMiliSecondsCreate = false;
	static HWND hPopupWindowShowMiliSeconds = NULL;
	if (PopupWindowShowMiliSecondsCreate == false) {
		TCHAR PopupWindowShowMiliSecondsClass[] = _TEXT("PopupWindowShowMiliSecondsClass");
		WNDCLASSEX wcex;
		wcex = { 0 };
		wcex.cbSize = sizeof(WNDCLASSEX);
		//wcex.style =
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = PopupWindowShowMiliSecondsWndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInst;
		wcex.hIcon = NULL;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = PopupWindowShowMiliSecondsClass;
		wcex.hIconSm = NULL;
		RegisterClassEx(&wcex);
		hPopupWindowShowMiliSeconds = CreateWindowEx(WS_EX_TOPMOST | WS_EX_NOACTIVATE, PopupWindowShowMiliSecondsClass, _TEXT(""), WS_POPUP | WS_BORDER | WS_VISIBLE, 50, 50, 100, 100, NULL, NULL, hInst, NULL);
		ShowWindow(hPopupWindowShowMiliSeconds, SW_SHOWNOACTIVATE);
		UpdateWindow(hPopupWindowShowMiliSeconds);
		PopupWindowShowMiliSecondsCreate = true;
	}
	KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;
	if (code == HC_ACTION) {
		switch (wParam) {
			case WM_KEYUP:
				switch (p->vkCode) {
					case 0x60:
					case 0x30:
						StringValueMilisecondsOfClick += _TEXT('0');
						if (SetWindowText(hPopupWindowShowMiliSeconds, StringValueMilisecondsOfClick.c_str()) == NULL) {
							DWORD RHKError = GetLastError();
							LPTSTR BufferForFormatMessage = nullptr;
							DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
							if (FMResult == 0) {
								OutputDebugString(_TEXT("Не удалось выполнить функцию SetWindowText!\n"));
								OutputDebugString(_TEXT("Мне не удалось причину возникновения ошибки!\n"));

							}
							else {
								OutputDebugString(_TEXT("Не удалось выполнить функцию SetWindowText!\n"));
								OutputDebugString(BufferForFormatMessage);
							}
							if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
								OutputDebugString(_TEXT("Не удалось освободить буфуер при обработке ошибки выполнения функции SetWindowText!"));
							}
						}
						return TRUE;
					case 0x61:
					case 0x31:
						StringValueMilisecondsOfClick += _TEXT('1');
						if (SetWindowText(hPopupWindowShowMiliSeconds, StringValueMilisecondsOfClick.c_str()) == NULL) {
							DWORD RHKError = GetLastError();
							LPTSTR BufferForFormatMessage = nullptr;
							DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
							if (FMResult == 0) {
								OutputDebugString(_TEXT("Не удалось выполнить функцию SetWindowText!\n"));
								OutputDebugString(_TEXT("Мне не удалось причину возникновения ошибки!\n"));

							}
							else {
								OutputDebugString(_TEXT("Не удалось выполнить функцию SetWindowText!\n"));
								OutputDebugString(BufferForFormatMessage);
							}
							if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
								OutputDebugString(_TEXT("Не удалось освободить буфуер при обработке ошибки выполнения функции SetWindowText!"));
							}
						}
						return TRUE;
					case 0x62:
					case 0x32:
						StringValueMilisecondsOfClick += _TEXT('2');
						if (SetWindowText(hPopupWindowShowMiliSeconds, StringValueMilisecondsOfClick.c_str()) == NULL) {
							DWORD RHKError = GetLastError();
							LPTSTR BufferForFormatMessage = nullptr;
							DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
							if (FMResult == 0) {
								OutputDebugString(_TEXT("Не удалось выполнить функцию SetWindowText!\n"));
								OutputDebugString(_TEXT("Мне не удалось причину возникновения ошибки!\n"));

							}
							else {
								OutputDebugString(_TEXT("Не удалось выполнить функцию SetWindowText!\n"));
								OutputDebugString(BufferForFormatMessage);
							}
							if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
								OutputDebugString(_TEXT("Не удалось освободить буфуер при обработке ошибки выполнения функции SetWindowText!"));
							}
						}
						return TRUE;
					case 0x63:
					case 0x33: 
						StringValueMilisecondsOfClick += _TEXT('3');
						if (SetWindowText(hPopupWindowShowMiliSeconds, StringValueMilisecondsOfClick.c_str()) == NULL) {
							DWORD RHKError = GetLastError();
							LPTSTR BufferForFormatMessage = nullptr;
							DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
							if (FMResult == 0) {
								OutputDebugString(_TEXT("Не удалось выполнить функцию SetWindowText!\n"));
								OutputDebugString(_TEXT("Мне не удалось причину возникновения ошибки!\n"));

							}
							else {
								OutputDebugString(_TEXT("Не удалось выполнить функцию SetWindowText!\n"));
								OutputDebugString(BufferForFormatMessage);
							}
							if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
								OutputDebugString(_TEXT("Не удалось освободить буфуер при обработке ошибки выполнения функции SetWindowText!"));
							}
						}
						return TRUE;
					case 0x64:
					case 0x34:
						StringValueMilisecondsOfClick += _TEXT('4');
						if (SetWindowText(hPopupWindowShowMiliSeconds, StringValueMilisecondsOfClick.c_str()) == NULL) {
							DWORD RHKError = GetLastError();
							LPTSTR BufferForFormatMessage = nullptr;
							DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
							if (FMResult == 0) {
								OutputDebugString(_TEXT("Не удалось выполнить функцию SetWindowText!\n"));
								OutputDebugString(_TEXT("Мне не удалось причину возникновения ошибки!\n"));

							}
							else {
								OutputDebugString(_TEXT("Не удалось выполнить функцию SetWindowText!\n"));
								OutputDebugString(BufferForFormatMessage);
							}
							if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
								OutputDebugString(_TEXT("Не удалось освободить буфуер при обработке ошибки выполнения функции SetWindowText!"));
							}
						}
						return TRUE;
					case 0x65:
					case 0x35: 
						StringValueMilisecondsOfClick += _TEXT('5');
						if (SetWindowText(hPopupWindowShowMiliSeconds, StringValueMilisecondsOfClick.c_str()) == NULL) {
							DWORD RHKError = GetLastError();
							LPTSTR BufferForFormatMessage = nullptr;
							DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
							if (FMResult == 0) {
								OutputDebugString(_TEXT("Не удалось выполнить функцию SetWindowText!\n"));
								OutputDebugString(_TEXT("Мне не удалось причину возникновения ошибки!\n"));

							}
							else {
								OutputDebugString(_TEXT("Не удалось выполнить функцию SetWindowText!\n"));
								OutputDebugString(BufferForFormatMessage);
							}
							if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
								OutputDebugString(_TEXT("Не удалось освободить буфуер при обработке ошибки выполнения функции SetWindowText!"));
							}
						}
						return TRUE;
					case 0x66:
					case 0x36:
						StringValueMilisecondsOfClick += _TEXT('6');
						if (SetWindowText(hPopupWindowShowMiliSeconds, StringValueMilisecondsOfClick.c_str()) == NULL) {
							DWORD RHKError = GetLastError();
							LPTSTR BufferForFormatMessage = nullptr;
							DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
							if (FMResult == 0) {
								OutputDebugString(_TEXT("Не удалось выполнить функцию SetWindowText!\n"));
								OutputDebugString(_TEXT("Мне не удалось причину возникновения ошибки!\n"));

							}
							else {
								OutputDebugString(_TEXT("Не удалось выполнить функцию SetWindowText!\n"));
								OutputDebugString(BufferForFormatMessage);
							}
							if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
								OutputDebugString(_TEXT("Не удалось освободить буфуер при обработке ошибки выполнения функции SetWindowText!"));
							}
						}
						return TRUE;
					case 0x67:
					case 0x37:
						StringValueMilisecondsOfClick += _TEXT('7');
						if (SetWindowText(hPopupWindowShowMiliSeconds, StringValueMilisecondsOfClick.c_str()) == NULL) {
							DWORD RHKError = GetLastError();
							LPTSTR BufferForFormatMessage = nullptr;
							DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
							if (FMResult == 0) {
								OutputDebugString(_TEXT("Не удалось выполнить функцию SetWindowText!\n"));
								OutputDebugString(_TEXT("Мне не удалось причину возникновения ошибки!\n"));

							}
							else {
								OutputDebugString(_TEXT("Не удалось выполнить функцию SetWindowText!\n"));
								OutputDebugString(BufferForFormatMessage);
							}
							if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
								OutputDebugString(_TEXT("Не удалось освободить буфуер при обработке ошибки выполнения функции SetWindowText!"));
							}
						}
						return TRUE;
					case 0x68:
					case 0x38: 
						StringValueMilisecondsOfClick += _TEXT('8');
						if (SetWindowText(hPopupWindowShowMiliSeconds, StringValueMilisecondsOfClick.c_str()) == NULL) {
							DWORD RHKError = GetLastError();
							LPTSTR BufferForFormatMessage = nullptr;
							DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
							if (FMResult == 0) {
								OutputDebugString(_TEXT("Не удалось выполнить функцию SetWindowText!\n"));
								OutputDebugString(_TEXT("Мне не удалось причину возникновения ошибки!\n"));

							}
							else {
								OutputDebugString(_TEXT("Не удалось выполнить функцию SetWindowText!\n"));
								OutputDebugString(BufferForFormatMessage);
							}
							if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
								OutputDebugString(_TEXT("Не удалось освободить буфуер при обработке ошибки выполнения функции SetWindowText!"));
							}
						}
						return TRUE;
					case 0x69:
					case 0x39:
						StringValueMilisecondsOfClick += _TEXT('9');
						if (SetWindowText(hPopupWindowShowMiliSeconds, StringValueMilisecondsOfClick.c_str()) == NULL) {
							DWORD RHKError = GetLastError();
							LPTSTR BufferForFormatMessage = nullptr;
							DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
							if (FMResult == 0) {
								OutputDebugString(_TEXT("Не удалось выполнить функцию SetWindowText!\n"));
								OutputDebugString(_TEXT("Мне не удалось причину возникновения ошибки!\n"));

							}
							else {
								OutputDebugString(_TEXT("Не удалось выполнить функцию SetWindowText!\n"));
								OutputDebugString(BufferForFormatMessage);
							}
							if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
								OutputDebugString(_TEXT("Не удалось освободить буфуер при обработке ошибки выполнения функции SetWindowText!"));
							}
						}
						return TRUE;
					case VK_BACK:
						if (StringValueMilisecondsOfClick.size() > 0) {
							StringValueMilisecondsOfClick.pop_back();
							if (SetWindowText(hPopupWindowShowMiliSeconds, StringValueMilisecondsOfClick.c_str()) == NULL) {
								DWORD RHKError = GetLastError();
								LPTSTR BufferForFormatMessage = nullptr;
								DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
								if (FMResult == 0) {
									OutputDebugString(_TEXT("Не удалось выполнить функцию SetWindowText!\n"));
									OutputDebugString(_TEXT("Мне не удалось причину возникновения ошибки!\n"));

								}
								else {
									OutputDebugString(_TEXT("Не удалось выполнить функцию SetWindowText!\n"));
									OutputDebugString(BufferForFormatMessage);
									MessageBox(NULL, BufferForFormatMessage, _TEXT("Не удалось выполнить функцию SetWindowText!"), MB_OK | MB_ICONERROR);
								}
								if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
									OutputDebugString(_TEXT("Не удалось освободить буфуер при обработке ошибки выполнения функции SetWindowText!"));
								}
							}
						}
						return TRUE;
					case VK_RETURN:{
						int MilisecondsOfClicks = _ttoi(StringValueMilisecondsOfClick.c_str());
						if (MilisecondsOfClicks < MIN_MILISECONDS) {
							//TCHAR ErrorText[100];
							//wsprintf(ErrorText, _TEXT("Введённое вами число должно быть больше либо равно чем %u и меньше либо равно чем %u"), MIN_MILISECONDS, PP.MaxMiliSeconds);
							//MessageBox(PP.GlobalHWND, ErrorText, _TEXT("Ошибка! Неверное число"), MB_OK | MB_ICONERROR);
							Beep(1000, 500);
							Beep(100, 500);
							StringValueMilisecondsOfClick.clear();
						}
						else if (MilisecondsOfClicks > PP.MaxMiliSeconds) {
							Beep(100, 500);
							Beep(1000, 500);
							StringValueMilisecondsOfClick.clear();
						}
						else {
							DestroyWindow(hPopupWindowShowMiliSeconds);
							UnhookWindowsHookEx(PP.hHookReadMiliSecondsOfClicks);
							PP.GlobalMiliSecondsOfClicks = MilisecondsOfClicks;
							PP.HookReadMilisecondsOfClicksIsRegistred = PopupWindowShowMiliSecondsCreate = false;
							StringValueMilisecondsOfClick.clear();
							InvalidateRect(PP.GlobalHWND, NULL, TRUE);
						}
						return TRUE;
					}
					default:
						return CallNextHookEx(NULL, code, wParam, lParam);
				}
				break;
			case WM_KEYDOWN:
				switch (p->vkCode) {
					case 0x60:
					case 0x30:
					case 0x61:
					case 0x31:
					case 0x62:
					case 0x32:
					case 0x63:
					case 0x33:
					case 0x64:
					case 0x34:
					case 0x65:
					case 0x35:
					case 0x66:
					case 0x36:
					case 0x67:
					case 0x37:
					case 0x68:
					case 0x38:
					case 0x69:
					case 0x39:
					case VK_BACK:
					case VK_RETURN:
						return TRUE;
				}
				break;
			default:
				return CallNextHookEx(NULL, code, wParam, lParam);
		}
	}
	else if (code > 0) {
		return CallNextHookEx(NULL, code, wParam, lParam);
	}
}
VOID CALLBACK TimerProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ UINT_PTR idEvent, _In_ DWORD dwTime) {
	switch (uMsg){
		case WM_TIMER:
			switch (idEvent) {
				case ProgrammParameters::TimerIds::TimerClickId:
					if (PP.ClickMouse) {
						if (PP.GlobalLeftButtonClick) {
							mouse_event(MOUSEEVENTF_LEFTDOWN, NULL, NULL, NULL, GetMessageExtraInfo());
							mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, NULL, GetMessageExtraInfo());
						}
						if (PP.GlobalRightButtonClick) {
							mouse_event(MOUSEEVENTF_RIGHTDOWN, NULL, NULL, NULL, GetMessageExtraInfo());
							mouse_event(MOUSEEVENTF_RIGHTUP, NULL, NULL, NULL, GetMessageExtraInfo());
						}
					}
					break;
			}
			break;
	}
}
bool CheckBit(unsigned word, unsigned bit) {// функция проверяет установлен ли бит под определённым номером
	return (word & (1u << bit)) != 0;
}