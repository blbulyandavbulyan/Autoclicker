// Autoclicker.cpp: Определяет точку входа для приложения.
//Чтобы включить тестовый фунционал нужно определить макрос NOT_FUNCIONAL

#include "stdafx.h"
#include "Autoclicker.h"
#include "hotkeys.h"
// глобальный экземпляр структуры с переменными состояния программы, это внутренние данные программы, не подлежат сохранению в реестр
ProgrammState PS;
//глобальный экземпляр структуры с настройками программы, подлежат сохранению в реестр
ProgrammParameters PP;
BOOL FlipToTray(HWND hWnd, NOTIFYICONDATA *nf);// эта функция сворачивает окно в трей
BOOL UnflipFromTray(HWND hWnd, NOTIFYICONDATA *nf);// эта функция разворачивает окно из трея
// Отправить объявления функций, включенных в этот модуль кода:
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK PopupWindowShowMiliSecondsWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);//функция обработки сообщений всплывающего окна, в котором вводится интервал кликов
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);//функция обработки сообщений диалогового окна "О программе"
INT_PTR CALLBACK MainDlgProc(HWND, UINT, WPARAM, LPARAM);//функция обработки сообщений основного окна программы(оно же окно с первичными настройками)
BOOL ShowPopupMenu(HWND hWnd, HMENU *hm);// эта функция показывает всплывающее меню, она вызываеться если щёлкнуть два раза левой кнопкой мыши по иконке этого приложения в трее
LRESULT CALLBACK KeyboardProc(_In_ int code, _In_ WPARAM wParam, _In_ LPARAM lParam);// процедура перехвата сообщений от клавиатуры для хука PP.hHookReadMiliSecondsOfClicks
#ifdef TESTED_FUNCTIONAL_CLICK_MOUSE_WHILE_THE_BUTTON_IS_PRESSED
LRESULT CALLBACK KeyboardHookClickingMouseWhileButtonIsPressedProc(_In_ int code, _In_ WPARAM wParam, LPARAM lParam);// процедура перехвата сообщений от клавиатуры для реализации второго режима работы автокликера
#endif

VOID CALLBACK TimerProc(_In_ HWND, _In_ UINT, _In_ UINT_PTR, _In_ DWORD);
bool CheckBit(unsigned word, unsigned bit);// функция проверяет установлен ли бит под определённым номером
UINT CalcBItemWidth(HWND hWnd, LPCTSTR Text);//функция вычисляет максимальную величину прокрутки по заданному тексту
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    //UNREFERENCED_PARAMETER(lpCmdLine);
    // TODO: разместите код здесь.
	//загрузка параметров из реестра, мы загружаем их до обработки аргументов командной строки, таким образом, больший приоритет будут иметь аргументы командной строки
	HANDLE MainMutex = OpenMutex(MUTEX_ALL_ACCESS, 0, _T("BLBULYANDAVBULYANAUTOCLICKERMUTEX"));//данный мютекс используется для отслеживания одновременного запуска нескольких копий данного приложения
	if (!MainMutex) {
		DWORD LastError = GetLastError();
		if (LastError == ERROR_FILE_NOT_FOUND) {
			MainMutex = CreateMutex(NULL, FALSE, _TEXT("BLBULYANDAVBULYANAUTOCLICKERMUTEX"));
			if (!MainMutex) {
				MessageError(_T("Не удалось создать мютекс с именем BLBULYANDAVBULYANAUTOCLICKERMUTEX"), _T("Ошибка создания мютекса в приложении Autoclicker!"), NULL);
			}
		}
		else {
			//то что нам не удалось открыть наш мютекс не будет мешать программе работать, но будут проблемы с контролем запуска нескольких копий
			MessageError(_T("Не удалось создать мютекс приложения с именем BLBULYANDAVBULYANAUTOCLICKERMUTEX"), _T("Ошибка открытия мютекса в приложении Autoclicker!"), NULL, LastError);
		}
	}
	else {
		//мютекс удалось открыть, значит он уже был кем-то создан.
		HWND hMainWindow = FindWindow(NULL, _TEXT("Autoclicker"));//ищем окно програмы по имени(вообще, тут предполагается что пользователь пользуется только одной программой у которой имя окна Autoclicker, тут необходимо добавить отслеживания факта, что это реально окно моей программы, а не чужое)
		//поскольку мне лень понимать свёрнуто ли окно в трей или нет, поэтому я просто буду симулировать нажатие горячей клавиши "Открыть главное окно", путём отправки соотвествующего сообщения
		
		PostMessage(hMainWindow, WM_HOTKEY, ProgrammParameters::HotKey::HotKeyOpenMainWindow, NULL);//имитируем нажатие горячей клавиши, которая открывает окно.

		return NULL;
	}	
	if(ProgrammParameters::RootSettingsRetistryKeyIsExist())PP.LoadSettingsFromRegistry(ProgrammParameters::REG_GENERAL_SETTINGS_QUERY | ProgrammParameters::REG_HOTKEY_SETTINGS_QUERY, NULL);
	TSTRINGARRAY CommandLineStrings = BreakAStringIntoAnArrayOfStringsByCharacter(lpCmdLine, _T(' '));
	for (size_t i = 0; i < CommandLineStrings.size(); i++){
		if(CommandLineStrings[i] == _T("--flip-to-tray")){
			PS.TheApplicationHasATrayIcon = true;
			continue;
		}
		else if (CommandLineStrings[i] == _T("--click-the-right-mouse-button")) {
			PP.GlobalLeftButtonClick = false;
			PP.GlobalRightButtonClick = true;
			continue;
		}
		else if (CommandLineStrings[i] == _T("--set-miliseconds-of-clicks")) {
			if(i != CommandLineStrings.size() - 1) {
				DWORD MiliSecondsOfClicks = 1;
				try {
					MiliSecondsOfClicks = _ttoi(CommandLineStrings[i + 1].c_str());
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
					TSTRING ErrorText = _T("Слишком большое значение аргумента --set-miliseconds-of-clicks, оно не может быть больше чем ") + to_tstring(PP.MaxMiliSeconds);
					MessageBox(NULL, ErrorText.c_str(), _T("Слишком большое значение аргумента командной строки --set-miliseconds-of-clicks."), MB_OK | MB_ICONERROR);
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
		else if (CommandLineStrings[i] == _T("--set-max-miliseconds")) {
			if (i != CommandLineStrings.size() - 1) {
				UINT MaxMiliSeconds = 0;
				try {
					MaxMiliSeconds = _ttoi(CommandLineStrings[i + 1].c_str());
				}
				catch (std::exception &e) {
					UNREFERENCED_PARAMETER(e);
					MessageBox(NULL, _TEXT("Неверное значение аргумента командной строки --set-max-miliseconds, оно должно быть числом!"), _TEXT("Ошибка! Неверное значение аргумента командной строки --set-max-miliseconds."), MB_ICONERROR | MB_OK);
					return -1;
				}
				if (MaxMiliSeconds < PP.MaxMiliSeconds) {
					TSTRING ErrorText = _TEXT("Значение аргумента командной строки --set-max-miliseconds не может быть меньше ") + to_tstring(PP.GlobalMiliSecondsOfClicks);
					MessageBox(NULL, ErrorText.c_str(), _TEXT("Ошибка! Неверное значение аргумента командной строки --set-max-miliseconds."), MB_OK | MB_ICONERROR);
					return -1;
				}
				else if (MaxMiliSeconds > USHRT_MAX) {
					TSTRING ErrorText = _T("Слишком большое значение аргумента --set-max-miliseconds, оно не может быть больше чем ") + to_tstring(USHRT_MAX);
					MessageBoxW(NULL, ErrorText.c_str(), _T("Слишком большое значение аргумента командной строки --set-max-miliseconds."), MB_OK | MB_ICONERROR);
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
			TSTRING ErrorTextString = _T("Неверный аргумент командной строки: \"");
			ErrorTextString += CommandLineStrings[i] + _T("\"!");
			MessageBox(NULL, ErrorTextString.c_str(), _T("Ошибка! Неврный аргумент командной строки!"), MB_OK | MB_ICONERROR);
			return -1;
		}
	}
    // Инициализация глобальных строк
    LoadString(hInstance, IDS_APP_TITLE, PS.szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_AUTOCLICKER, PS.szWindowClass, MAX_LOADSTRING);
   // MyRegisterClass(hInstance);
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
	PP.SaveSettingsInRegistry(ProgrammParameters::REG_HOTKEY_SETTINGS_QUERY | ProgrammParameters::REG_GENERAL_SETTINGS_QUERY, NULL);
	if (MainMutex != NULL) {
		ReleaseMutex(MainMutex);
		CloseHandle(MainMutex);
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
   PS.hInst = hInstance; // Сохранить дескриптор экземпляра в глобальной переменной
  /* HWND hWnd = CreateWindow(PP.szWindowClass, PP.szTitle, WS_OVERLAPPEDWINDOW | WS_MINIMIZE, CW_USEDEFAULT, 0,
	   500, 500, nullptr, nullptr, hInstance, nullptr);*/
	   HWND hWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc);
   if (!hWnd){
		MessageError(_TEXT("Не удалось создать главное окно программы"), L"Ошибка создания главного окна!", NULL);
		return FALSE;
   }
   PS.RootTrayIcon.cbSize = sizeof(NOTIFYICONDATA);
   PS.RootTrayIcon.hWnd = hWnd;
   PS.RootTrayIcon.uID = NULL;
   lstrcpy(PS.RootTrayIcon.szTip, PS.szTitle);
   lstrcpy(PS.RootTrayIcon.szInfoTitle, PS.szTitle);
   PS.RootTrayIcon.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
   PS.RootTrayIcon.uCallbackMessage = WM_MESSAGE_FROM_TRAY;
   PS.RootTrayIcon.hIcon = PS.RootTrayIcon.hBalloonIcon = LoadIcon(PS.hInst, MAKEINTRESOURCE(IDI_AUTOCLICKER));
   if (PS.TheApplicationHasATrayIcon)FlipToTray(hWnd, &PS.RootTrayIcon);
   else{
	   ShowWindow(hWnd, nCmdShow);
	   UpdateWindow(hWnd);
   }
   PS.GlobalHWND = hWnd;
   return TRUE;
}

LRESULT CALLBACK PopupWindowShowMiliSecondsWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static HFONT hFont;
	TCHAR FontName[7] = _TEXT("Tahoma");
	LOGFONT lf;
	static LPCTSTR TextMiliseconds = nullptr;
	static BOOL TextReplaced = TRUE, TextScroll = FALSE;
	static HWND hScrollBar = nullptr;
	static WORD Width = 0;
	static SCROLLINFO si;
	switch (message) {
		case WM_SIZE: 
			Width = LOWORD(lParam);
			break;
		case WM_CREATE:{
			memset(&lf, 0, sizeof(lf));
			lf.lfHeight = 20;
			lf.lfCharSet = RUSSIAN_CHARSET;
			lstrcpy(lf.lfFaceName, FontName);
			hFont = CreateFontIndirect(&lf);
			TextMiliseconds = (LPCTSTR)(((LPCREATESTRUCT)lParam)->lpCreateParams);
			return TRUE;
		}
		case WM_HSCROLL:{
			switch (LOWORD(wParam)) {
				case SB_PAGERIGHT:
					//ScrollPos += (Width/CharWidth);
					break;
				case SB_PAGELEFT:
					//ScrollPos -= (Width / CharWidth);
					break;
				case SB_LINERIGHT:
					//ScrollPos+=CharWidth;
					break;
				case SB_LINELEFT:
					//ScrollPos-= CharWidth;
					break;
				case SB_LEFT:
					//ScrollPos = 0;
					break;
				case SB_RIGHT:
					//ScrollPos = MaxScrollPos;
					break;
			}
			/*if (ScrollPos > MaxScrollPos)ScrollPos = MaxScrollPos;
			else if (ScrollPos < 0)ScrollPos = 0;
			if (ScrollPos == MaxScrollPos) {
				EnableScrollBar(hWnd, SB_HORZ, ESB_DISABLE_RIGHT);
			}
			else if (ScrollPos == 0) {
				EnableScrollBar(hWnd, SB_HORZ, ESB_DISABLE_LEFT);
			}
			if ((ScrollPos > 0) && (ScrollPos < MaxScrollPos)) {
				EnableScrollBar(hWnd, SB_HORZ, ESB_DISABLE_RIGHT);
				EnableScrollBar(hWnd, SB_HORZ, ESB_ENABLE_BOTH);
			}
			INT CurrentScrollPos = GetScrollPos(hWnd, SB_HORZ);
			ScrollWindow(hWnd, ScrollPos - CurrentScrollPos, 0, NULL, NULL);
			//обновляем положение полосы прокрутки
			SetScrollPos(hWnd, SB_HORZ, ScrollPos, TRUE);
			UpdateWindow(hWnd);
			TextScroll = TRUE;*/
			break;
		}
		case WM_PAINT:{
			if ((TextMiliseconds != nullptr)) {
				RECT rect;
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				GetClientRect(hWnd, &rect);
				SelectObject(hdc, hFont);
				DrawText(hdc, TextMiliseconds, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				EndPaint(hWnd, &ps);
			}
			else if (TextScroll) {
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				RECT rect;

				EndPaint(hWnd, &ps);
			}
			//блок кода для прокручивания окна
		}
		break;
		case WM_SETTEXT:{
			TextMiliseconds = (LPCTSTR)lParam;
			TextReplaced = TRUE;
			InvalidateRect(hWnd, NULL, TRUE);
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
//функция обработки сообщений основного окна
INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	HWND hFrequencyOfClicks = GetDlgItem(hDlg, IDC_SLIDER_FREQUENCY_OF_CLICKS);
	HWND hLeftClcikRadioButton = GetDlgItem(hDlg, IDC_RADIO_LEFT_BUTTON_CLICK);
	HWND hRightClcikRadioButton = GetDlgItem(hDlg, IDC_RADIO_RIGHT_BUTTON_CLICK);
	HWND hWhenMinimizingTheWindowMinimizeToTray = GetDlgItem(hDlg, IDC_WHEN_MINIMIZING_THE_WINDOW_MINIMIZE_TO_TRAY);
	TSTRING szText = _TEXT("Милисекунды ") + to_tstring(PP.GlobalMiliSecondsOfClicks);
	//UINT iMin = MIN_MILISECONDS;// мнимальное и максимальное значение слайдера IDC_SLIDER_FREQUENCY_OF_CLICKS, который предназначен для выбора
	// интервала кликов, который потом пишеться в переменную PP.GlobalMiliSecondsOfClicks
	switch(message){
		case WM_INITDIALOG:{
			SendMessage(hFrequencyOfClicks, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(MIN_MILISECONDS, PP.MaxMiliSeconds));
			SendMessage(hFrequencyOfClicks, TBM_SETPAGESIZE, 0, (LPARAM)10);
			SendMessage(hFrequencyOfClicks, TBM_SETSEL, (WPARAM)FALSE, (WPARAM)MAKELONG(MIN_MILISECONDS, PP.MaxMiliSeconds));
			SendMessage(hFrequencyOfClicks, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)PP.GlobalMiliSecondsOfClicks);
			HICON hDialogIcon = LoadIcon(PS.hInst, MAKEINTRESOURCE(IDI_AUTOCLICKER));
			if(hDialogIcon != NULL){
				SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hDialogIcon);
				SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hDialogIcon);
			}
			//PP.GlobalMiliSecondsOfClicks = SendMessage(hFrequencyOfClicks, TBM_GETPOS, NULL, NULL);
			SetDlgItemText(hDlg, IDC_MILLISECONDS, szText.c_str());
			//установка радиокнопок, отвечающих за выбор кнопки мыши для клика
			if (PP.GlobalRightButtonClick)SendMessage(hRightClcikRadioButton, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendMessage(hLeftClcikRadioButton, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			//установка чекбокса отвечающего за то, будет ли сворачиваться программа в трей
			if (PP.WhenMinimizingTheWindowMinimizeToTray)SendMessage(hWhenMinimizingTheWindowMinimizeToTray, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendMessage(hWhenMinimizingTheWindowMinimizeToTray, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			RegisterHotKeys(hDlg, PP.HotKeys, sizeof(PP.HotKeys) / sizeof(ProgrammParameters::HotKey));
			return (INT_PTR)TRUE;
		}
		case WM_HSCROLL:
			if (lParam == (LPARAM)hFrequencyOfClicks) {
				PP.GlobalMiliSecondsOfClicks = SendMessage(hFrequencyOfClicks, TBM_GETPOS, NULL, NULL);
				szText = _TEXT("Милисекунды ") + to_tstring(PP.GlobalMiliSecondsOfClicks);
				SetDlgItemText(hDlg, IDC_MILLISECONDS, szText.c_str());
			} 
			break;
		// обработка сообщений от иконки в трее
		case WM_MESSAGE_FROM_TRAY: {
			switch (lParam) {
				case WM_LBUTTONDBLCLK:// если по иконке в трее кликнули два раза
					PS.TheApplicationHasATrayIcon = !UnflipFromTray(hDlg, &PS.RootTrayIcon);// показать окно и убрать иконку из трея 
					break;
				case WM_RBUTTONDOWN: {// если была нажата правая кнокпа мыши
					HMENU hRootMenu = LoadMenu(PS.hInst, MAKEINTRESOURCE(IDC_AUTOCLICKER));
					HMENU hCMenu = GetSubMenu(hRootMenu, 0);
					ShowPopupMenu(hDlg, &hCMenu);// показать всплывающее меню
					DestroyMenu(hCMenu);
					break;
				}
			}
			break;
		}
		//в этом блоке расположена обработка кнопки свернуть, если на ней назначено сворачивание в трей
		case WM_SYSCOMMAND: {
			switch (wParam) {
			case SC_MINIMIZE:// обработка нажатии кнопки "Свернуть окно"
				if (PP.WhenMinimizingTheWindowMinimizeToTray == true)PS.TheApplicationHasATrayIcon = FlipToTray(hDlg, &PS.RootTrayIcon);
				break;
			}
			break;
		}
		//обработка горячих клавиш
		case WM_HOTKEY:
			switch (wParam) {
				case ProgrammParameters::HotKey::HotKeyClickStartId:
					if (!PS.ClickMouse) {
						PS.ClickMouse = true;
						SetTimer(hDlg, ProgrammState::TimerIds::TimerClickId, PP.GlobalMiliSecondsOfClicks, TimerProc);
					}
					else {
						PS.ClickMouse = false;
						KillTimer(hDlg, ProgrammState::TimerIds::TimerClickId);
					}
					break;
				case ProgrammParameters::HotKey::HotKeyOpenMainWindow: {
					if(PS.TheApplicationHasATrayIcon)PS.TheApplicationHasATrayIcon = !UnflipFromTray(hDlg, &PS.RootTrayIcon);
					else {
						if(IsIconic(hDlg))ShowWindow(hDlg, SW_RESTORE);
						SetForegroundWindow(hDlg);
					}
					break;
				}
				case ProgrammParameters::HotKey::HotKeyOpenHotKeySettings: {
					HotKeySettingsDlgParameters HDP;
					HDP.hDialogIcon = LoadIcon(PS.hInst, MAKEINTRESOURCE(IDI_SETTINGS_DLG_PROC));
					HDP.HotKeysCount = sizeof(ProgrammParameters::HotKeys) / sizeof(ProgrammParameters::HotKey);
					HDP.PHotKeys = PP.HotKeys;
					UnregisterHotKeys(hDlg, PP.HotKeys, HDP.HotKeysCount);
					DialogBoxParam(PS.hInst, MAKEINTRESOURCE(IDD_HOTKEY_SETTINGS), NULL, HotKeySettingsDlgProc, (LPARAM)&HDP);
					RegisterHotKeys(hDlg, PP.HotKeys, HDP.HotKeysCount);
					break;
				}
				case ProgrammParameters::HotKey::HotKeyCloseId:
					DestroyWindow(hDlg);
					break;
				case ProgrammParameters::HotKey::HotKeyLeftButtonClickId:
					PP.GlobalLeftButtonClick = true;
					PP.GlobalRightButtonClick = false;
					InvalidateRect(hDlg, NULL, FALSE);
					break;
				case ProgrammParameters::HotKey::HotKeyRightButtonClickId:
					PP.GlobalLeftButtonClick = false;
					PP.GlobalRightButtonClick = true;
					InvalidateRect(hDlg, NULL, FALSE);
					break;
				case ProgrammParameters::HotKey::HotKeySetIntervalOfClicksId:
					// данный участок кода предоствращает баг с залипанием клавиш Ctrl, Alt, Shift
					if (PS.HookReadMilisecondsOfClicksIsRegistred == true) {
						return (LRESULT)TRUE;
					}
					LabelBeginForCheckKeyStates:
					if (CheckBit(GetAsyncKeyState(VK_CONTROL), 16) == true || CheckBit(GetAsyncKeyState(VK_SHIFT), 16) == true || CheckBit(GetAsyncKeyState(VK_MENU), 16)) {// данное условие проверяет нажата какая-либо из клавиш Ctrl, Alt, Shift
						Sleep(500);
						goto LabelBeginForCheckKeyStates;
					}
					else {
						goto LabelEndForCheckKeyStates;
					}
					LabelEndForCheckKeyStates:
					//конец данного участка
					PS.hHookReadMiliSecondsOfClicks = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, PS.hInst, NULL);
					KeyboardProc(-1, 0, 0);
					if (PS.hHookReadMiliSecondsOfClicks == NULL)MessageError(_TEXT("Ошибка в функции ") __FUNCTION__ _TEXT(" не удалось зарегистрировать хук перехвата клавиатуры для чтения интервала кликов"), _TEXT("Ошибка регистрации хука"), hDlg);
					else PS.HookReadMilisecondsOfClicksIsRegistred = true;
					return (LRESULT)TRUE;
			}
			break;
		case WM_CLOSE:
			DestroyWindow(hDlg);
			break;
		case WM_PAINT: {
			//переключение переключателей, если они были переключены горячими клавишами
			if (!IsDlgButtonChecked(hDlg, IDC_RADIO_RIGHT_BUTTON_CLICK) && PP.GlobalRightButtonClick) {
				SendMessage(hRightClcikRadioButton, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				SendMessage(hLeftClcikRadioButton, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			}
			else if(!IsDlgButtonChecked(hDlg, IDC_RADIO_LEFT_BUTTON_CLICK) && PP.GlobalLeftButtonClick) {
				SendMessage(hRightClcikRadioButton, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
				SendMessage(hLeftClcikRadioButton, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			}
			if (!IsDlgButtonChecked(hDlg, IDC_WHEN_MINIMIZING_THE_WINDOW_MINIMIZE_TO_TRAY) && PP.WhenMinimizingTheWindowMinimizeToTray)
				SendMessage(hWhenMinimizingTheWindowMinimizeToTray, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else if(IsDlgButtonChecked(hDlg, IDC_WHEN_MINIMIZING_THE_WINDOW_MINIMIZE_TO_TRAY) && !PP.WhenMinimizingTheWindowMinimizeToTray)
				SendMessage(hWhenMinimizingTheWindowMinimizeToTray, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			//обновление информации о количестве милисекунд, если она была изменена
			TSTRING fClicks = _TEXT(""); 
			int TextLenght = GetWindowTextLength(GetDlgItem(hDlg, IDC_MILLISECONDS));
			if (TextLenght != 0) {
				UINT InTextMiliseconds = 0;
				TSTRING TextMilisecondsOfClicks;
				fClicks.resize(TextLenght+1);
				GetWindowText(GetDlgItem(hDlg, IDC_MILLISECONDS), (LPTSTR)fClicks.data(), TextLenght+1);
				for (UINT i = 0; i < fClicks.size(); i++) {
					switch (fClicks[i]) {
						case _T('0'):
						case _T('1'):
						case _T('2'):
						case _T('3'):
						case _T('4'):
						case _T('5'):
						case _T('6'):
						case _T('7'):
						case _T('8'):
						case _T('9'):
							TextMilisecondsOfClicks += fClicks[i];
						default:
							continue;
					}
				}
				if (TextMilisecondsOfClicks != _TEXT("")) {
					try {
						InTextMiliseconds = _ttoi(TextMilisecondsOfClicks.c_str());
						if (InTextMiliseconds != PP.GlobalMiliSecondsOfClicks) {
							SetDlgItemText(hDlg, IDC_MILLISECONDS, szText.c_str());
							SendMessage(hFrequencyOfClicks, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)PP.GlobalMiliSecondsOfClicks);
						}
					}
					catch(std::exception &e){}
				}	
			}
			break;
		}
		case WM_DESTROY: {
			//для разрушения иконок трея
			if (PS.TheApplicationHasATrayIcon)Shell_NotifyIcon(NIM_DELETE, &PS.RootTrayIcon);
			//мы не вызываем DestroyIcon для иконок загруженных с ресурсов
			UnregisterHotKeys(hDlg, PP.HotKeys, sizeof(ProgrammParameters::HotKeys) / sizeof(ProgrammParameters::HotKey));//деактивация горячих клавиш
			if (PS.HookReadMilisecondsOfClicksIsRegistred) {
				if (UnhookWindowsHookEx(PS.hHookReadMiliSecondsOfClicks) == NULL)MessageError(_TEXT("Ошибка в функии ") __FUNCTION__ _TEXT(". Ошибка уничтожения хука перехвата клавиатуры для чтения интервала кликов при выходе из программы!"), _TEXT("Ошибка уничтожения хука"), NULL);
			}
			PostQuitMessage(0);
			break;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)){
				case IDM_ABOUT:
					DialogBox(PS.hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, About);
					break;
				case IDM_EXIT:
					DestroyWindow(hDlg);
					break;
				case ID_HOTKEY_SETTINGS: {
					HotKeySettingsDlgParameters HDP;
					HDP.hDialogIcon = LoadIcon(PS.hInst, MAKEINTRESOURCE(IDI_SETTINGS_DLG_PROC));
					HDP.HotKeysCount = sizeof(ProgrammParameters::HotKeys) / sizeof(ProgrammParameters::HotKey);
					HDP.PHotKeys = PP.HotKeys;
					UnregisterHotKeys(hDlg, PP.HotKeys, HDP.HotKeysCount);
					DialogBoxParam(PS.hInst, MAKEINTRESOURCE(IDD_HOTKEY_SETTINGS), NULL, HotKeySettingsDlgProc, (LPARAM)& HDP);
					RegisterHotKeys(hDlg, PP.HotKeys, HDP.HotKeysCount);
					break;
				}
				case IDC_RADIO_LEFT_BUTTON_CLICK:
					if (IsDlgButtonChecked(hDlg, IDC_RADIO_LEFT_BUTTON_CLICK) == BST_CHECKED) {
						PP.GlobalLeftButtonClick = true;
						PP.GlobalRightButtonClick = false;
					}
					break;
				case IDC_RADIO_RIGHT_BUTTON_CLICK:
					if (IsDlgButtonChecked(hDlg, IDC_RADIO_RIGHT_BUTTON_CLICK) == BST_CHECKED) {
						PP.GlobalRightButtonClick = true;
						PP.GlobalLeftButtonClick = false;
					}
					break;
				case IDC_WHEN_MINIMIZING_THE_WINDOW_MINIMIZE_TO_TRAY:
					if (IsDlgButtonChecked(hDlg, IDC_WHEN_MINIMIZING_THE_WINDOW_MINIMIZE_TO_TRAY) == BST_CHECKED) {
						PP.WhenMinimizingTheWindowMinimizeToTray = true;
					}
					else {
						PP.WhenMinimizingTheWindowMinimizeToTray = false;
					}
					break;
				case IDC_BUTTON_RESET_GENERAL_SETTINGS: {
					if (MessageBox(hDlg, _T("Вы точно уверены?"), _T("Сбосить настройки?!?!"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
						ProgrammParameters PPL;
						PP.GlobalMiliSecondsOfClicks = PPL.GlobalMiliSecondsOfClicks;
						PP.GlobalRightButtonClick = PPL.GlobalRightButtonClick;
						PP.GlobalLeftButtonClick = PPL.GlobalLeftButtonClick;
						PP.MaxMiliSeconds = PPL.MaxMiliSeconds;
						PP.WhenMinimizingTheWindowMinimizeToTray = PPL.WhenMinimizingTheWindowMinimizeToTray;
						InvalidateRect(hDlg, NULL, TRUE);
						//UpdateWindow(hDlg);
					}
					break;
				}
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

LRESULT CALLBACK KeyboardProc(_In_ int code, _In_ WPARAM wParam, _In_ LPARAM lParam) {// функция для перехвата клавиатуры, перехват будет происходить при зажатии Ctrl+Alt+Shift+Z, будут перехватываться клавиши с цифрами 0-9, это нужно для того чтобы ввести новый интервал кликов не открывая настройки автокликера, предназначена для хука PP.hHookReadMiliSecondsOfClicks
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
		wcex.hInstance = PS.hInst;
		wcex.hIcon = NULL;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = PopupWindowShowMiliSecondsClass;
		wcex.hIconSm = NULL;
		RegisterClassEx(&wcex);
		StringValueMilisecondsOfClick = to_tstring(PP.GlobalMiliSecondsOfClicks);
		hPopupWindowShowMiliSeconds = CreateWindowEx(WS_EX_TOPMOST | WS_EX_NOACTIVATE, PopupWindowShowMiliSecondsClass, _TEXT("AC setmiliseconds:"), WS_POPUP | WS_BORDER | WS_VISIBLE | WS_HSCROLL, 50, 50, 100, 100, NULL, NULL, PS.hInst, (LPVOID)StringValueMilisecondsOfClick.c_str());
		//ДОБАВЬ СЮДА ОБРАБОТКУ ОШИБКИ СОЗДАНИЯ ОКНА!!!
		if (hPopupWindowShowMiliSeconds == NULL) {
			MessageError(_TEXT("Ошибка в функции ") __FUNCTION__ _TEXT(". Не удалось создать окно для отображения считанного интервала кликов! Ввод интервала кликов будет прекращён прекращён!"), _TEXT("Ошибка создания окна"), PS.GlobalHWND);
			if (UnhookWindowsHookEx(PS.hHookReadMiliSecondsOfClicks) == NULL)MessageError(_TEXT("Ошибка в функции ") __FUNCTION__ _TEXT(". Не удалось удалить хук перехвата клавиатуры для чтения интервала кликов!"), _TEXT("Ошибка удаления хука!"), PS.GlobalHWND);
			return FALSE;
		}
		ShowWindow(hPopupWindowShowMiliSeconds, SW_SHOWNOACTIVATE);
		UpdateWindow(hPopupWindowShowMiliSeconds);
		PopupWindowShowMiliSecondsCreate = true;
		if (SetWindowText(hPopupWindowShowMiliSeconds, StringValueMilisecondsOfClick.c_str()) == NULL) {
			TSTRING EMsg = _TEXT("Ошибка в функции ") __FUNCTION__ _TEXT(" на строке ") + to_tstring(__LINE__);
			EMsg += _TEXT(" не удалось вывести текущий интервал кликов в окно вывода интервала кликов!");
			MessageDebug(EMsg.c_str(), _TEXT("Ошибка вывода текста в окно"));
		}
	}
	KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;
	if (code == HC_ACTION) {
		switch (wParam) {
			case WM_KEYUP:
				switch (p->vkCode) {
					//здесь идёт общая обработка всех цифр, код преобразуется в символ
					// цифра 0
					case 0x60:
					case 0x30:
					//цифра 1
					case 0x61:
					case 0x31:
					// цифра 2
					case 0x62:
					case 0x32:
					// цифра 3
					case 0x63:
					case 0x33: 
					// цифра 4
					case 0x64:
					case 0x34:
					// цифра 5
					case 0x65:
					case 0x35: 
					// цифра 6
					case 0x66:
					case 0x36:
					//цифра 7
					case 0x67:
					case 0x37:
					// цифра 8
					case 0x68:
					case 0x38:
					// цифра 9
					case 0x69:
					case 0x39:
						StringValueMilisecondsOfClick += MapVirtualKey(p->vkCode, MAPVK_VK_TO_CHAR);
						if (SetWindowText(hPopupWindowShowMiliSeconds, StringValueMilisecondsOfClick.c_str()) == NULL) {
							TSTRING EMsg = _TEXT("Ошибка в функции ") __FUNCTION__ _TEXT(" на строке ") + to_tstring(__LINE__);
							EMsg += _TEXT(" не удалось вывести интервал кликов в окно вывода интервала кликов!");
							MessageDebug(EMsg.c_str(), _TEXT("Ошибка вывода текста в окно"));
						}
						return TRUE;
					case VK_BACK:
						return TRUE;
					case VK_DELETE:
						if (StringValueMilisecondsOfClick.size() > 0)StringValueMilisecondsOfClick.clear();
						if (SetWindowText(hPopupWindowShowMiliSeconds, StringValueMilisecondsOfClick.c_str()) == NULL) {
							TSTRING EMsg = _TEXT("Ошибка в функции ") __FUNCTION__ _TEXT(" на строке ") + to_tstring(__LINE__);
							EMsg += _TEXT(" не удалось очистить значение в окне ввода интервала кликов!");
							MessageDebug(EMsg.c_str(), _TEXT("Ошибка очистки текста в окне!"));
						}
						return TRUE;
					case VK_ESCAPE:
						DestroyWindow(hPopupWindowShowMiliSeconds);
						UnhookWindowsHookEx(PS.hHookReadMiliSecondsOfClicks);
						PS.HookReadMilisecondsOfClicksIsRegistred = PopupWindowShowMiliSecondsCreate = false;
						StringValueMilisecondsOfClick.clear();
						return TRUE;
					case VK_RETURN:{
						DWORD MilisecondsOfClicks = _ttoi(StringValueMilisecondsOfClick.c_str());
						if (MilisecondsOfClicks < MIN_MILISECONDS) {
							Beep(1000, 500);
							Beep(100, 500);
							StringValueMilisecondsOfClick.clear();
							InvalidateRect(hPopupWindowShowMiliSeconds, NULL, TRUE);
						}
						else if (MilisecondsOfClicks > PP.MaxMiliSeconds) {
							Beep(100, 500);
							Beep(1000, 500);
							StringValueMilisecondsOfClick.clear();
							InvalidateRect(hPopupWindowShowMiliSeconds, NULL, TRUE);
						}
						else {
							DestroyWindow(hPopupWindowShowMiliSeconds);
							UnhookWindowsHookEx(PS.hHookReadMiliSecondsOfClicks);
							PP.GlobalMiliSecondsOfClicks = MilisecondsOfClicks;
							PS.HookReadMilisecondsOfClicksIsRegistred = PopupWindowShowMiliSecondsCreate = false;
							StringValueMilisecondsOfClick.clear();
							InvalidateRect(PS.GlobalHWND, NULL, TRUE);
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
					case VK_ESCAPE:
					case VK_RETURN:
					case VK_DELETE:
						return TRUE;
					case VK_BACK:
						if (StringValueMilisecondsOfClick.size() > 0) {
							StringValueMilisecondsOfClick.pop_back();
							if (SetWindowText(hPopupWindowShowMiliSeconds, StringValueMilisecondsOfClick.c_str()) == NULL) {
								TSTRING EMsg = _TEXT("Ошибка в функции ") __FUNCTION__ _TEXT(" на строке ") + to_tstring(__LINE__);
								EMsg += _TEXT(" не удалось отобразить строку с удалённым символом в окне ввода интервала кликов!");
								MessageDebug(EMsg.c_str(), _TEXT("Ошибка удаления символа из окна!"));
							}
						}
						return TRUE;
					default:
						return CallNextHookEx(NULL, code, wParam, lParam);
				}
				break;
			default:
				return CallNextHookEx(NULL, code, wParam, lParam);
		}
	}
	else if (code < 0) {
		return CallNextHookEx(NULL, code, wParam, lParam);
	}
}

VOID CALLBACK TimerProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ UINT_PTR idEvent, _In_ DWORD dwTime) {
	switch (uMsg){
		case WM_TIMER:
			switch (idEvent) {
				case ProgrammState::TimerIds::TimerClickId:
					if (PS.ClickMouse) {
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
//функция предназначена для установки максимального диапазона полосы прокрутки, она считает размер текста в пикселях по заданной строке
UINT CalcBItemWidth(HWND hWnd, LPCTSTR Text) {
	RECT r;
	HDC hWndDC = GetDC(hWnd);
	HDC hDC = CreateCompatibleDC(hWndDC);
	HFONT hFont = (HFONT)SendMessageW(hWnd, WM_GETFONT, 0, 0);
	HGDIOBJ hOrgFont = SelectObject(hDC, hFont);
	ZeroMemory(&r, sizeof(r));
	DrawTextW(hDC, Text, -1, &r, DT_CALCRECT | DT_SINGLELINE | DT_NOCLIP);
	SelectObject(hDC, hOrgFont);
	DeleteDC(hDC);
	ReleaseDC(hWnd, hWndDC);
	return (r.right - r.left) + (2 * GetSystemMetrics(SM_CXEDGE));
}
//функция загружает настройки из реестра
void ProgrammParameters::LoadSettingsFromRegistry(UINT OpCode, HWND hWnd) {
	LSTATUS RegistryOperationResult = ERROR_SUCCESS;
	HKEY hRootSettings = NULL;
	RegistryOperationResult = RegOpenKeyEx(HKEY_CURRENT_USER, PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings"), NULL, KEY_READ, &hRootSettings);
	if (RegistryOperationResult == ERROR_SUCCESS) {
		//загрузка общих настроек программы
		if ((OpCode & ProgrammParameters::REG_GENERAL_SETTINGS_QUERY) == ProgrammParameters::REG_GENERAL_SETTINGS_QUERY) {
			DWORD cbData = sizeof(UINT);//размер получаямых данных из реестра(первоначально присваивается размер UINT т.к. сначала идёт загрузка интервала кликов)
			LPBYTE lpData = NULL;//указатель на полученные данные(буфер выделяем сами)
			//загрузка интервала кликов из реестра
			RegistryOperationResult = RegGetValue(hRootSettings, NULL, _TEXT("IntervalOfClicks"), RRF_RT_REG_DWORD, NULL, &this->GlobalMiliSecondsOfClicks, &cbData);
			if (RegistryOperationResult != ERROR_SUCCESS)MessageError(_TEXT("Не удалось загрузить интервал кликов(параметр IntervalOfClicks в реестре) из реестра по пути HKEY_CURRENT_USER\\") PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings!"), _TEXT("Ошибка получения параметра из реестра"), hWnd, RegistryOperationResult);
			//загрузка кнопки для клика из реестра
			RegistryOperationResult = RegGetValue(hRootSettings, NULL, _TEXT("MouseButtonForClick"), RRF_RT_REG_SZ, NULL, NULL, &cbData);
			if (RegistryOperationResult == ERROR_SUCCESS) {
				lpData = new BYTE[cbData];
				RegistryOperationResult = RegGetValue(hRootSettings, NULL, _TEXT("MouseButtonForClick"), RRF_RT_REG_SZ, NULL, lpData, &cbData);
				if (RegistryOperationResult == ERROR_SUCCESS) {
					TSTRING LoadedButton = (LPTSTR)lpData;
					if (LoadedButton == _TEXT("RBTN")) {
						this->GlobalLeftButtonClick = false;
						this->GlobalRightButtonClick = true;
					}
					else if (LoadedButton == _TEXT("LBTN")) {
						this->GlobalLeftButtonClick = true;
						this->GlobalRightButtonClick = false;
					}
					else MessageBox(hWnd, _TEXT("Значение параметра в реестре для задания кнопки мыши для клика(MouseButtonForClick) не соотвествует допустимым по пути в реестре HKEY_CURRENT_USER\\") PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings!") _TEXT(" Допустимые параметры RBTN или LBTN."), _TEXT("Ошибка! Неверное значение параметра в реестре!"), MB_OK | MB_ICONERROR);
					delete[] lpData;
				}
				else {
					delete[] lpData;
					goto ErrorLoadMouseButtonForClick;
				}
			}
			else {
			ErrorLoadMouseButtonForClick:
				MessageError(_TEXT("Не удалось загрузить параметр отвечающий за задание кнопки мыши для клика(MouseButtonForClick) из реестра по пути HKEY_CURRENT_USER\\") PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings!"), _TEXT("Ошибка загрузки параметра из реестра!"), hWnd, RegistryOperationResult);
			}
			//загрузка информации о сворачивании в трей при сворачивании окна
			cbData = sizeof(this->WhenMinimizingTheWindowMinimizeToTray);
			RegistryOperationResult = RegGetValue(hRootSettings, NULL, _TEXT("WhenStartingTheProgramMinimizeTheWindowToTray"), RRF_RT_REG_BINARY, NULL, &this->WhenMinimizingTheWindowMinimizeToTray, &cbData);
			if (RegistryOperationResult != ERROR_SUCCESS)MessageError(_TEXT("Не удалось получить настройку программы, отвечающую за сворачивания программы в трей при сворачивании окна(параметр WhenStartingTheProgramMinimizeTheWindowToTray) из реестра по пути HKEY_CURRENT_USER\\") PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings!"), _TEXT("Ошибка получения параметра из реестра"), hWnd, RegistryOperationResult);
		}
		//загрузка настроек горячих клавиш
		if ((OpCode & ProgrammParameters::REG_HOTKEY_SETTINGS_QUERY) == ProgrammParameters::REG_HOTKEY_SETTINGS_QUERY) {
			HKEY hRootHotKeySettings = NULL;
			//открываем ключ реестра с настройками горячих клавиш
			RegistryOperationResult = RegOpenKeyEx(hRootSettings, _TEXT("HotKeys"), NULL, KEY_READ, &hRootHotKeySettings);
			if (RegistryOperationResult == ERROR_SUCCESS) {
				//данный цикл идёт по горячим клавишам в массиве HotKeys, и с помощью RegistryHotKeyName получает ветку с настройками соотвествующей горячей клавиши
				for (UINT i = 0; i < sizeof(this->HotKeys) / sizeof(ProgrammParameters::HotKey); i++) {
					HKEY hHotKey = NULL;//дескриптор ключа настроек КОНКРЕТНОЙ горячей клавиши
					//открываем ключ ррестра с настройками КОНКРЕТНОЙ горячей клавиши
					RegistryOperationResult = RegOpenKeyEx(hRootHotKeySettings, this->HotKeys[i].RegistryHotKeyName, NULL, KEY_READ, &hHotKey);
					if (RegistryOperationResult == ERROR_SUCCESS) {//в случае успеха
						ProgrammParameters::HotKey HotKey;//локальный экземпляр структуру, в неё будут писаться считанные с ррестра значения, в случае если они допустимы, они будут присвоены 
						BOOL GetHotKeyFailed = FALSE;//переменная хранящая информацию об успехе получения горячей клавиши
						TSTRING ParameterName[3] = { _TEXT("Modifiers"), _TEXT("vk"), _TEXT("HotKeyIsEnable") };//параметры для получения, подставляются в функцию RegGetValue по очереди
						DWORD dwFlags[3] = { RRF_RT_REG_DWORD, RRF_RT_REG_DWORD, RRF_RT_REG_BINARY };//типы значений, их порядок должен по смыслу соотвествовать порядку параметров
						DWORD Sizes[3] = { sizeof(HotKey.Modifiers), sizeof(HotKey.vk), sizeof(HotKey.HotKeyIsEnable) };//массив размеров переменных для хранения параметров
						LPVOID Params[3] = { &HotKey.Modifiers, &HotKey.vk, &HotKey.HotKeyIsEnable };//массив указателей на переменные для сохранения
						//массив функторов для проверки правильности полученных значений, по смыслу должен соотвествовать порядку во всех предыдущих массивах
						std::function<bool(LPVOID)> IsValidArgument[] = {
							[](LPVOID Arg)->bool {
								UINT a = *((LPUINT)Arg);
								if (((MOD_WIN | MOD_SHIFT | MOD_CONTROL | MOD_ALT) & a) == a)return true;
								else return false;
							},
							[](LPVOID Arg)->bool {
								UINT a = *((LPUINT)Arg);
								return true;
							},
							[](LPVOID Arg)->bool {
								bool a = *((bool*)Arg);
								if ((a == true) || (a == false))return true;
								else return false;
							}
						};
						/*
							Идея такова, мы имеем массив с размерами, массив с указателями для получения данных, массив с именами параметров в реестре,
							массив с функторами для проверки этих данных, мы идём по всем этим массивам и выполняем этот код столько раз, сколько параметров нам нужно получить
						*/
						/*
							ВНИМАНИЕ! ГОРЯЧАЯ КЛАВИША СЧИТАЕТСЯ НЕ ПОЛУЧЕННОЙ (GetHotKeyFailed = TRUE) В СЛУЧАЕ ЕСЛИ ХОТЯ БЫ ОДИН ПАРАМЕТР, ОТВЕЧАЮЩИЙ ЗА НАСТРОЙКУ ГОРЯЧЕЙ КЛАВИШИ
							В РЕЕСТРЕ ИМЕЕТ НЕДОПУСТИМОЕ ЗНАЧЕНИЕ, НЕДОПУСТИМЫЙ ТИП
						*/
						for (UINT p = 0; p < 3; p++) {//цикл получения настроек
							TSTRING EMsg, ECMsg;
							RegistryOperationResult = RegGetValue(hHotKey, NULL, ParameterName[p].c_str(), dwFlags[p], NULL, Params[p], &Sizes[p]);
							if (RegistryOperationResult != ERROR_SUCCESS) {
								EMsg = _TEXT("Не удалось получить значение параметра ") + ParameterName[p] + _TEXT(" для горячей клавиши ");
								ECMsg = _TEXT("Ошибка загрузки настроек ГК с ид ") + to_tstring(i) + _TEXT("!");
								EMsg += _TEXT(" с ИД ") + to_tstring(i);
								EMsg += _TEXT(" по пути HKEY_CURRENT_USER\\") PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings\\HotKeys\\");
								EMsg += this->HotKeys[i].RegistryHotKeyName;
								MessageError(EMsg.c_str(), ECMsg.c_str(), hWnd, RegistryOperationResult);
								GetHotKeyFailed = TRUE;
								break;
							}
							else if (!IsValidArgument[p](Params[p])) {
								EMsg = _TEXT("Неверное  значение параметра ") + ParameterName[p] + _TEXT(" для горячей клавиши ");
								ECMsg = _TEXT("Ошибка загрузки настроек ГК с ид ") + to_tstring(i) + _TEXT("!");
								EMsg += _TEXT(" с ИД ") + to_tstring(i);
								EMsg += _TEXT(" по пути HKEY_CURRENT_USER\\") PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings\\HotKeys\\");
								EMsg += this->HotKeys[i].RegistryHotKeyName;
								MessageBox(hWnd, EMsg.c_str(), ECMsg.c_str(), MB_OK | MB_ICONERROR);
								GetHotKeyFailed = TRUE;
								break;
							}
						}
						RegCloseKey(hHotKey);//закрываем ключ реестра с настройками КОНКРЕТНОЙ горячей клавиши
						//проверка на успешное получение параметров
						if (GetHotKeyFailed)continue;//в случае ошибки переходим к следующей итарции цикла
						else {
							//если всё нормально присваиваем полученную горячую клавишу
							this->HotKeys[i].Modifiers = HotKey.Modifiers;
							this->HotKeys[i].vk = HotKey.vk;
							this->HotKeys[i].HotKeyIsEnable = HotKey.HotKeyIsEnable;
						}
					}
					else {
						TSTRING EMsg = _TEXT("Не удалось открыть ветку с настройками горячей клавиши с ИД ") + to_tstring(i);
						EMsg += _TEXT(" по пути ")PROGRAMM_ROOT_REGISTRY_KEY _TEXT("Settings\\HotKeys\\");
						EMsg += this->HotKeys[i].RegistryHotKeyName;
						MessageError(EMsg.c_str(), _TEXT("Ошибка открытия ветки реестра!"), hWnd, RegistryOperationResult);
					}
				}
			}
			else MessageError(_TEXT("Не удалось открыть ветку с настройками горячих клавиш по пути в реестре HKEY_CURRENT_USER\\") PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings\\HotKeys!"), _TEXT("Ошибка открытия ветки в реестре!"), hWnd, RegistryOperationResult);
		}
		//закрываем ключ настроек программы
		RegCloseKey(hRootSettings);
	}
	else {
		MessageError(_TEXT("Не удалось открыть ветку реестра по пути HKEY_CURRENT_USER\\")PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings")
			_TEXT("! Основные настройки программы не были загружены из реестра, будут использоваться натройки по умолчанию."),
			_TEXT("Ошибка открытия ветки Settings"), hWnd, RegistryOperationResult);
		return;
	}
}
//функция сохраняет настройки в реестр
void ProgrammParameters::SaveSettingsInRegistry(UINT OpCode, HWND hWnd) {
	LSTATUS RegistryOperationResult = ERROR_SUCCESS;
	//сохранение общих настроек программы
	if ((OpCode & ProgrammParameters::REG_GENERAL_SETTINGS_QUERY) == ProgrammParameters::REG_GENERAL_SETTINGS_QUERY) {
		//начало транзакции
		HKEY hRootSettings = 0;
		DWORD lpwdDispositionFromSettingsKey = 0;
		HANDLE hTransaction = CreateTransaction(NULL, 0, 0, 0, 0, INFINITE, (LPWSTR)L"Creation Settings Key");//создаём транзакцию для создания ветки Settings
		if (hTransaction != NULL) {
			RegistryOperationResult = RegCreateKeyTransacted(HKEY_CURRENT_USER, PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings"), NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hRootSettings, &lpwdDispositionFromSettingsKey, hTransaction, NULL);// создание ветки реестра с общими настройками
			if (RegistryOperationResult == ERROR_SUCCESS) {
				//создание параметра IntervalOfClicks в реестре
				RegistryOperationResult = RegSetValueEx(hRootSettings, _TEXT("IntervalOfClicks"), NULL, REG_DWORD, (BYTE*)& this->GlobalMiliSecondsOfClicks, sizeof(this->GlobalMiliSecondsOfClicks));// создания переменной в реестре для хранения интервала кликов
				if (RegistryOperationResult != ERROR_SUCCESS)MessageError(_TEXT("Ошибка при сохранении параметров в реестр. Не удалось создать параметр IntervalOfClicks в ветке реестра ") PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings\n"), _TEXT("Не удалось создать параметр в реестре"), hWnd, RegistryOperationResult);
				//создание параметра MouseButtonForClick в реестре
				RegistryOperationResult = RegSetValueEx(hRootSettings, _TEXT("MouseButtonForClick"), NULL, REG_SZ, (BYTE*)(this->GlobalLeftButtonClick ? _TEXT("LBTN") : _TEXT("RBTN")), sizeof(_TEXT("LBTN")));// создание переменной в реестре для хранения информации о том какой кнопкой мыши кликать
				if (RegistryOperationResult != ERROR_SUCCESS)MessageError(_TEXT("Ошибка при сохранении параметров в реестр. Не удалось создать параметр MouseButtonForClick в ветке реестра ") PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings\n"), _TEXT("Не удалось создать параметр в реестре"), hWnd, RegistryOperationResult);
				//создание параметра WhenStartingTheProgramMinimizeTheWindowToTray в реестре
				RegistryOperationResult = RegSetValueEx(hRootSettings, _TEXT("WhenStartingTheProgramMinimizeTheWindowToTray"), NULL, REG_BINARY, (BYTE*)& this->WhenMinimizingTheWindowMinimizeToTray, sizeof(this->WhenMinimizingTheWindowMinimizeToTray));// создание переменной в реестре которая отвечает за то, нужно ли сворачивать программу в трей при запуске, если значение равно 0 то не нужно, если больше нуля, то нужно
				if (RegistryOperationResult != ERROR_SUCCESS)MessageError(_TEXT("Ошибка при сохранении параметров в реестр. Не удалось создать параметр WhenStartingTheProgramMinimizeTheWindowToTray в ветке реестра ") PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings\n"), _TEXT("Не удалось создать параметр в реестре"), hWnd, RegistryOperationResult);
				//закрываеем ключ реестра
				RegCloseKey(hRootSettings);
			}
			else MessageError(_TEXT("Не удалось создать ветку в реестре по пути HKEY_CURRENTUSER\\") PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings"), _TEXT("Ошибка создания ветки в реестре!"), hWnd, RegistryOperationResult);
			CommitTransaction(hTransaction);
			CloseHandle(hTransaction);
		}
		else MessageError(_TEXT("Не удалось создать транзакцию для создания ветки в реестре по пути HKEY_CURRENTUSER\\") PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings"), _TEXT("Ошибка создания транзакции!"), hWnd);
	}
	//сохранение настроек горячих клавиш
	if ((OpCode & ProgrammParameters::REG_HOTKEY_SETTINGS_QUERY) == ProgrammParameters::REG_HOTKEY_SETTINGS_QUERY) {
		DWORD lpwdDispositionFromHotKetSettingsKey = 0;
		HKEY hRootHotKeySettings = 0;
		HANDLE hTransaction = CreateTransaction(NULL, 0, 0, 0, 0, INFINITE, (LPWSTR)L"Creation HotKey Settings");//создаём транзакцию для сохранения настроек горячих клавиш в реестр
		if (hTransaction != NULL) {
			// создание ветки реестра с настройками горячих клавиш
			RegistryOperationResult = RegCreateKeyTransacted(HKEY_CURRENT_USER, PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings\\HotKeys"), NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hRootHotKeySettings, &lpwdDispositionFromHotKetSettingsKey, hTransaction, NULL);
			if (RegistryOperationResult == ERROR_SUCCESS) {
				//На данный момент настройки горячей клавиши сохраняются в реестр в бинарном виде
				for (UINT i = 0; i < sizeof(ProgrammParameters::HotKeys) / sizeof(ProgrammParameters::HotKey); i++) {
					HKEY hRootHotKey = NULL;
					//открываем корневой ключ конфигурации горячей клавиши
					RegistryOperationResult = RegCreateKeyTransacted(hRootHotKeySettings, this->HotKeys[i].RegistryHotKeyName, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hRootHotKey, &lpwdDispositionFromHotKetSettingsKey, hTransaction, NULL);
					if (RegistryOperationResult == ERROR_SUCCESS) {
						RegSetValueEx(hRootHotKey, _TEXT("Modifiers"), NULL, REG_DWORD, (BYTE*)& this->HotKeys[i].Modifiers, sizeof(this->HotKeys[i].Modifiers));
						RegSetValueEx(hRootHotKey, _TEXT("vk"), NULL, REG_DWORD, (BYTE*)& this->HotKeys[i].vk, sizeof(this->HotKeys[i].vk));
						RegSetValueEx(hRootHotKey, _TEXT("HotKeyIsEnable"), NULL, REG_BINARY, (BYTE*)& this->HotKeys[i].HotKeyIsEnable, sizeof(this->HotKeys[i].HotKeyIsEnable));
						//закрываем корневой ключ конфигурации горячей клавиши
						RegCloseKey(hRootHotKey);
					}
					else {
						TSTRING EMsg = _TEXT("Не удалось сохраненить горячую клавишу ");
						EMsg += ProgrammParameters::HotKeys[i].RegistryHotKeyName;
						EMsg += _TEXT(" в реестр ");
						EMsg += _TEXT(" по пути HKEY_CURRENT_USER\\") PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings\\HotKeys");
						MessageError(EMsg.c_str(), L"Ошибка сохранения параметра в реестр", hWnd, RegistryOperationResult);
					}
				}
				//закрываем корневой ключ конфигурации горячих клавиш
				RegCloseKey(hRootHotKeySettings);
			}
			//вывод сообщения об ошибке в случае не удачи создания ключа HKEY_CURRENT_USER\SOFTWARE\\Blbulyan Software\Autoclicker\Settings\HotKeys
			else MessageError(_TEXT("Не удалось создать ветку HKEY_CURRENT_USER\\")PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings\\HotKeys"), _TEXT("Ошибка создания ветки реестра"), hWnd, RegistryOperationResult);
			CommitTransaction(hTransaction);//применение изменений, коммитим транзакцию
			CloseHandle(hTransaction);//закрываем дескриптор транзакции
		}
		else MessageError(_TEXT("Не удалось создать транзакцию для создания ключа HKEY_CURRENTUSER\\") PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings\\HotKeys"), _TEXT("Ошибка создания транзакции!"), hWnd);
	}
}
//функция проверяет существует ли корневой ключ реестра моей программы на компьютере
bool ProgrammParameters::RootSettingsRetistryKeyIsExist() {
	HKEY hKey = NULL;
	LSTATUS RegOpen = RegOpenKeyEx(HKEY_CURRENT_USER, PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings"), NULL, KEY_READ, &hKey);
	if (RegOpen == ERROR_SUCCESS) {
		RegCloseKey(hKey);
		return true;
	}
	else return false;
}