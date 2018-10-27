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
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
struct ProgrammParameters {
	bool GlobalLeftButtonClick = true; // щёлкать левой кнопкой мыши если выбран переключатель IDC_RADIO_LEFT_BUTTON_CLICK в IDD_SETTINGS 
	bool GlobalRightButtonClick = false;// щёлкать правой кнопкой мыши если выбран переключатель IDC_RADIO_RIGHT_BUTTON_CLICK в IDD_SETTINGS 
	bool ClickMouse = false; // эта переменная становиться true если нажато Ctrl + Alt + Shift + C, и false при повторном нажатии и отвечает за то, будут ли происходить щелчки мышью или нет
	LRESULT GlobalMiliSecondsOfClicks = 10; // глобальная переменная которая содержит интервал кликов
	static enum TimerIds {
		TimerClickId
	}; // в этом перечислении сожержаться идентификаторы таймеров, идентификатр таймера TimerClickId связан с таймером который отправляет сообщение WM_TIMER в соотвествии с интервалом GlobalMiliSecondsOfClicks
	bool TheApplicationHasATrayIcon = false;// эта переменная равна true если приложение имеет иконку в трее
} PP;
BOOL FlipToTray(HWND hWnd, NOTIFYICONDATA *nf);// эта функция сворачивает окно в трей
BOOL UnflipFromTray(HWND hWnd, NOTIFYICONDATA *nf);// эта функция разворачивает окно из трея
// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SettingsDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL ShowPopupMenu(HWND hWnd, HINSTANCE hInstance, WORD nResourceID);// эта функция показывает всплывающее меню, она вызываеться если щёлкнуть два раза левой кнопкой мыши по иконке этого приложения в трее
VOID CALLBACK TimerProc(_In_ HWND, _In_ UINT, _In_ UINT_PTR, _In_ DWORD);
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: разместите код здесь.

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_AUTOCLICKER, szWindowClass, MAX_LOADSTRING);
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
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_AUTOCLICKER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_AUTOCLICKER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassExW(&wcex);
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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить дескриптор экземпляра в глобальной переменной
   HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_MINIMIZE, CW_USEDEFAULT, 0,
	   500, 500, nullptr, nullptr, hInstance, nullptr);
   if (!hWnd){
      return FALSE;
   }
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
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
	enum HotKeys{HotKeyClickStartId, HotKeySettingsId, HotKeyCloseId, HotKeyRightButtonClick, HotKeyLeftButtonClick}; // перечисление горячих клавиш, HotKeyClickStartId -
	//идентификатор горячей клавиши которая отвечает за присваивания переменной ClickMouse значения true
	// и установки таймера с идентификатором TimerIds::TimerClickId,
	//HotKeySettingsId - идентификатор горячей клавиши которая отвечает за показ диалога настроек;
	//HotKeyRightButtonClick - идентификатор горячей клавиши, которая отвечает за ЕСЛИ ЗАЖАТА ПРАВАЯ КНОПКА МЫШИ
	//HotKeyLeftButtonClick - идентификатор горячей клавиши, которая отвечает за ЕСЛИ ЗАЖАТА ЛЕВАЯ КНОПКА МЫШИ
	RECT rect;
	static HFONT hFont;
	static NOTIFYICONDATA nf;
	TCHAR FontName[7] = _TEXT("Tahoma");
	LOGFONT lf;
    switch (message){
		case WM_MESSAGE_FROM_TRAY:{// обработка сообщений от иконки в трее
			switch (lParam) {
				case WM_LBUTTONDBLCLK:// если по иконке в трее кликнули два раза
					PP.TheApplicationHasATrayIcon = !UnflipFromTray(hWnd, &nf);// показать окно и убрать иконку из трея 
					break;
				case WM_RBUTTONDOWN:{// если была нажата правая кнокпа мыши
					ShowPopupMenu(hWnd, hInst, IDC_AUTOCLICKER);// показать всплывающее меню
					}
					break;
			}
			}
			break;
		case WM_SYSCOMMAND:{
			switch (wParam) {
				case SC_MINIMIZE:{
					PP.TheApplicationHasATrayIcon = FlipToTray(hWnd, &nf);
					}
					break;
				default:
					DefWindowProc(hWnd, message, wParam, lParam);
					break;
			}
			}
			break;
		case WM_COMMAND:{
				int wmId = LOWORD(wParam);
				// Разобрать выбор в меню:
				switch (wmId){
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
			}
			break;
		case WM_CREATE:{
			RegisterHotKey(hWnd, HotKeys::HotKeyClickStartId, MOD_CONTROL | MOD_ALT | MOD_SHIFT, VK_C);
			RegisterHotKey(hWnd, HotKeys::HotKeySettingsId, MOD_CONTROL | MOD_ALT | MOD_SHIFT, VK_S);
			RegisterHotKey(hWnd, HotKeys::HotKeyCloseId, MOD_CONTROL | MOD_ALT | MOD_SHIFT, VK_Q);
			memset(&lf, 0, sizeof(lf));
			lf.lfHeight = 20;
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
			}	
			break;
		case WM_PAINT:{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				TCHAR szText[200];
				// TODO: Добавьте сюда любой код прорисовки, использующий HDC...
				wsprintf(szText, L"Интервал кликов: %u\n Чтобы запустить автокликер нажмите Ctrl + Alt + Shift + C\n Чтбы открыть настройки нажмете Ctrl + Alt + Shift + S", PP.GlobalMiliSecondsOfClicks);
				GetClientRect(hWnd, &rect);
				SelectObject(hdc, hFont);
				DrawText(hdc, szText, -1, &rect, DT_CENTER | DT_VCENTER);
				EndPaint(hWnd, &ps);
			}
			break;
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
			}
			break;
		break;
		case WM_DESTROY:{
			UnregisterHotKey(hWnd, HotKeys::HotKeyClickStartId);
			UnregisterHotKey(hWnd, HotKeys::HotKeySettingsId);
			UnregisterHotKey(hWnd, HotKeys::HotKeyCloseId);
			if(PP.TheApplicationHasATrayIcon)Shell_NotifyIcon(NIM_DELETE, &nf);
			DestroyIcon(nf.hIcon);
			PostQuitMessage(0);
		}
			break;
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
	static LRESULT InFuncMiliSecondsOfClicks = MIN_MILISECONDS; // локальная переменная аналог глобальной GlobalMiliSecondsOfClicks,
	// в эту переменную пишеться значение которое пользователь выбрал с помщью элемента IDC_SLIDER_FREQUENCY_OF_CLICKS
	// значение этой переменной записываються в GlobalMiliSecondsOfClicks после нажатия кнопки OK в IDD_SETTINGS
	static bool InFuncRightButtonClick = false;
	static bool InFuncLeftButtonClick = true;
	UINT iMin = MIN_MILISECONDS, iMax = MAX_MILISECONDS;// мнимальное и максимальное значение слайдера IDC_SLIDER_FREQUENCY_OF_CLICKS, который предназначен для выбора
	// интервала кликов, который потом пишеться в переменную InFuncMiliSecondsOfClicks
	UINT_PTR TimerId = 1; // идентификатор таймера который отвечает за динамическое отображение
	//текста в диалоге IDD_SETTINGS в элементе IDC_MILLISECONDS
	UINT uElapse = 10; // время срабатывания таймера оно относится к таймеру с ID TimerId
	switch(message){
		case WM_INITDIALOG:
			SendMessage(hFrequencyOfClicks, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(iMin, iMax));
			SendMessage(hFrequencyOfClicks, TBM_SETPAGESIZE, 0, (LPARAM)10);
			SendMessage(hFrequencyOfClicks, TBM_SETSEL, (WPARAM)FALSE, (WPARAM)MAKELONG(iMin, iMax));
			SendMessage(hFrequencyOfClicks, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)InFuncMiliSecondsOfClicks);
			//InFuncMiliSecondsOfClicks = SendMessage(hFrequencyOfClicks, TBM_GETPOS, NULL, NULL);
			wsprintf(szText, L"Милисекунды %u", InFuncMiliSecondsOfClicks);
			SetDlgItemText(hDlg, IDC_MILLISECONDS, szText);
			if (InFuncRightButtonClick) {
				SendMessage(hRightClcikRadioButton, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			}
			if (InFuncLeftButtonClick) {
				SendMessage(hLeftClcikRadioButton, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			}
			SetTimer(hDlg, TimerId, uElapse, NULL);
			return (INT_PTR)TRUE;
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
		case WM_TIMER:
			if (SendMessage(hFrequencyOfClicks, TBM_GETPOS, NULL, NULL) != InFuncMiliSecondsOfClicks) {
				InFuncMiliSecondsOfClicks = SendMessage(hFrequencyOfClicks, TBM_GETPOS, NULL, NULL);
				wsprintf(szText, L"Милисекунды %u", InFuncMiliSecondsOfClicks);
				SetDlgItemText(hDlg, IDC_MILLISECONDS, szText);
			}
			break;
	}
	return (INT_PTR)FALSE;
}
BOOL ShowPopupMenu(HWND hWnd, HINSTANCE hInstance, WORD nResourceID) {
	HMENU hMenu = ::LoadMenu(hInstance,
		MAKEINTRESOURCE(nResourceID));
	if (!hMenu)  return FALSE;
	HMENU hPopup = GetSubMenu(hMenu, 0);
	AppendMenu(hPopup, MF_ENABLED | MF_STRING, IDM_ABOUT, _TEXT("О программе"));
	if (!hPopup) return FALSE;
	SetForegroundWindow(hWnd);
	POINT pt;
	GetCursorPos(&pt);
	BOOL bOK = TrackPopupMenu(hPopup, 0, pt.x, pt.y, 0, hWnd, NULL);
	DestroyMenu(hMenu);
	return bOK;
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