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
#define PROGRAMM_ROOT_REGISTRY_KEY _TEXT("HKEY_CURRENT_USER\\SOFTWARE\\Blbulyan Software\\Autoclicker")
// Глобальные переменные:
#define TESTED_FUNCTIONAL_CLICK_MOUSE_WHILE_THE_BUTTON_IS_PRESSED
//структура с переменными состояния программы, это внутренние данные программы, не подлежат сохранению в реестр
struct ProgrammState {
	HINSTANCE hInst;                                // текущий экземпляр
	TCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
	TCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
	HWND GlobalHWND = NULL;// дескриптор главного окна
	static enum TimerIds {
		TimerClickId
	}; // в этом перечислении сожержаться идентификаторы таймеров, идентификатр таймера TimerClickId связан с таймером который отправляет сообщение WM_TIMER в соотвествии с интервалом GlobalMiliSecondsOfClicks
	bool TheApplicationHasATrayIcon = false;// эта переменная равна true если приложение имеет иконку в трее
	bool ClickMouse = false; // эта переменная становиться true если нажато Ctrl + Alt + Shift + C, и false при повторном нажатии и отвечает за то, будут ли происходить щелчки мышью или нет
	HHOOK hHookReadMiliSecondsOfClicks = NULL;// хук на перехват клавиатуры для чтения нового интервала кликов, активируется при нажатии Ctrl+Alt+Shift+Z(по умолчанию)
	bool HookReadMilisecondsOfClicksIsRegistred = false;// зарегистрирован ли hHookReadMiliSecondsOfClicks
	#ifdef TESTED_FUNCTIONAL_CLICK_MOUSE_WHILE_THE_BUTTON_IS_PRESSED
	HHOOK hKeyboardHookClickingMouseWhileButtonIsPressed = NULL;
	bool hKeyboardHookClickingMouseWhileButtonIsPressedIsCreated = false;
	#endif
	NOTIFYICONDATA RootTrayIcon;
} PS;
//структура с настройками программы, подлежат сохранению в реестр
struct ProgrammParameters {
	bool GlobalLeftButtonClick = true; // щёлкать левой кнопкой мыши если выбран переключатель IDC_RADIO_LEFT_BUTTON_CLICK в IDD_SETTINGS 
	bool GlobalRightButtonClick = false;// щёлкать правой кнопкой мыши если выбран переключатель IDC_RADIO_RIGHT_BUTTON_CLICK в IDD_SETTINGS 
	LRESULT GlobalMiliSecondsOfClicks = 10; // глобальная переменная которая содержит интервал кликов
	WORD MaxMiliSeconds = 1000;// максимальное значение интервала кликов
	bool WhenMinimizingTheWindowMinimizeToTray = false;// при сворачивании окна сворачивать программу в трей
	struct HotKey {
		enum HotKeysList {
			HotKeyClickStartId, HotKeyOpenMainWindow, HotKeyCloseId, HotKeyRightButtonClickId, HotKeyLeftButtonClickId, HotKeySetIntervalOfClicksId, HotKeyOpenHotKeySettings
		};
		UINT Modifiers;
		UINT vk;
		bool HotKeyIsRegistred = false;
		bool HotKeyIsEnable = false;
	};
	static HotKey HotKeys[];
	/*
	void LoadDataFromRegistry();
	void CreateRegistryKey(TSTRING KeyName, HKEY hRootKey);*/
	private:
		struct CreateProgrammRegistryKeyResult {
			LSTATUS CreateKeyResult = 0;
			HKEY hCreatedKey = nullptr;
			bool KeyExist = false;
		};
		CreateProgrammRegistryKeyResult CreateProgrammRegistryKey(HANDLE hTransaction, HKEY hRootKey, LPCTSTR KeyName);
} PP;
/*
void ProgrammParameters::LoadDataFromRegistry() {

}
void ProgrammParameters::CreateRegistryKey(TSTRING KeyName, HKEY hRootKey) {
	if (KeyName == _TEXT("Settings")) {
		//начало транзакции
		HKEY hRootSettings = 0;
		DWORD lpwdDispositionFromSettingsKey = 0;
		HANDLE hTransaction = CreateTransaction(NULL, 0, 0, 0, 0, INFINITE, (LPWSTR)L"Creation Settings Key");//создаём транзакцию для создания ветки Settings
		if (hTransaction != NULL) {
			LSTATUS CreateASettingsKeyStatus = RegCreateKeyTransacted(hRootKey, _TEXT("Settings"), NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hRootSettings, &lpwdDispositionFromSettingsKey, hTransaction, NULL);// создание ветки реестра с общими настройками
			if (CreateASettingsKeyStatus == ERROR_SUCCESS) {
				LSTATUS CreateIntervalOfClicks = RegSetValueEx(hRootSettings, _TEXT("IntervalOfClicks"), NULL, REG_DWORD, (BYTE*)& this->GlobalMiliSecondsOfClicks, sizeof(this->GlobalMiliSecondsOfClicks));// создания переменной в реестре для хранения интервала кликов
				if (CreateIntervalOfClicks != ERROR_SUCCESS) {
					LPTSTR BufferForFormatMessage = nullptr;
					DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, CreateIntervalOfClicks, LANG_SYSTEM_DEFAULT, (LPTSTR)& BufferForFormatMessage, NULL, nullptr);
					OutputDebugString(_TEXT("Не удалось создать параметр IntervalOfClicks в ветке реестра ") PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings\n"));
					if (FMResult == 0) {
						OutputDebugString(_TEXT("Мне не удалось причину возникновения ошибки!\n"));
					}
					else {
						OutputDebugString(BufferForFormatMessage);
					}
					if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
						OutputDebugString(_TEXT("Не удалось освободить буфуер при обработке предыдущей ошибки!"));
					}
				}
				LSTATUS CreateMouseButtonForClick = RegSetValueEx(hRootSettings, _TEXT("MouseButtonForClick"), NULL, REG_EXPAND_SZ, (BYTE*)_TEXT("LBTN"), sizeof(_TEXT("LBTN")) / sizeof(TCHAR));// создание переменной в реестре для хранения информации о том какой кнопкой мыши кликать
				if (CreateMouseButtonForClick != ERROR_SUCCESS) {
					LPTSTR BufferForFormatMessage = nullptr;
					DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, CreateIntervalOfClicks, LANG_SYSTEM_DEFAULT, (LPTSTR)& BufferForFormatMessage, NULL, nullptr);
					OutputDebugString(_TEXT("Не удалось создать параметр MouseButtonForClick в ветке реестра HKEY_CURRENT_USER\\SOFTWARE\\Blbulyan Software\\Autoclicker\\Settings\n"));
					if (FMResult == 0) {
						OutputDebugString(_TEXT("Мне не удалось причину возникновения ошибки!\n"));
					}
					else {
						OutputDebugString(BufferForFormatMessage);
					}
					if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
						OutputDebugString(_TEXT("Не удалось освободить буфуер при обработке предыдущей ошибки!"));
					}
				}
				LSTATUS WhenStartingTheProgramMinimizeTheWindowToTray = RegSetValueEx(hRootSettings, _TEXT("WhenStartingTheProgramMinimizeTheWindowToTray"), NULL, REG_DWORD, (BYTE*)0, sizeof(UINT));// создание переменной в реестре которая отвечает за то, нужно ли сворачивать программу в трей при запуске, если значение равно 0 то не нужно, если больше нуля, то нужно
				if (CreateMouseButtonForClick != ERROR_SUCCESS) {
					LPTSTR BufferForFormatMessage = nullptr;
					DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, CreateIntervalOfClicks, LANG_SYSTEM_DEFAULT, (LPTSTR)& BufferForFormatMessage, NULL, nullptr);
					OutputDebugString(_TEXT("Не удалось создать параметр MouseButtonForClick в ветке реестра HKEY_CURRENT_USER\\SOFTWARE\\Blbulyan Software\\Autoclicker\\Settings\n"));
					if (FMResult == 0) {
						OutputDebugString(_TEXT("Мне не удалось причину возникновения ошибки!\n"));
					}
					else {
						OutputDebugString(BufferForFormatMessage);
					}
					if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
						OutputDebugString(_TEXT("Не удалось освободить буфуер при обработке предыдущей ошибки!"));
					}
				}
			}
		}
		else {
			//обработка ошибки создания транзакции
		}

	}
	else if (KeyName == _TEXT("HotKey Settings")) {
		DWORD lpwdDispositionFromHotKetSettingsKey = 0;
		HKEY hRootHotKeySettings = 0;
		HANDLE hTransaction = CreateTransaction(NULL, 0, 0, 0, 0, INFINITE, (LPWSTR)L"Creation HotKey Settings");
		if (hTransaction != NULL) {
			LSTATUS CreateAHotKeySettingsKeyStatus = RegCreateKeyTransacted(hRootKey, _TEXT("Settings"), NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hRootHotKeySettings, &lpwdDispositionFromHotKetSettingsKey, hTransaction, NULL);;// создание ветки реестра с настройками горячих клавиш
			if (CreateAHotKeySettingsKeyStatus == ERROR_SUCCESS) {
				//LSTATUS sHotKeyClickStartId = 

			}
			else {

			}
		}
		else {

		}

	}
}*/
/*ProgrammParameters::CreateProgrammRegistryKeyResult ProgrammParameters::CreateProgrammRegistryKey(HANDLE hTransaction, HKEY hRootKey, LPCTSTR KeyName)
{
	ProgrammParameters::CreateProgrammRegistryKeyResult result;
	return CreateProgrammRegistryKeyResult();
}*/
ProgrammParameters::HotKey ProgrammParameters::HotKeys[] = {
		{//данная горячая клавиша включает и отключает процесс кликов
			MOD_CONTROL | MOD_ALT | MOD_SHIFT,
			VK_C,
			false,
			true
		},
		{//горячая клавиша открывает главное окно программы
			MOD_CONTROL | MOD_ALT | MOD_SHIFT,
			VK_S,
			false,
			true
		},
		{// данная горячая клавиша завершает работу программы
			MOD_CONTROL | MOD_ALT | MOD_SHIFT,
			VK_Q,
			false,
			true
		},
		{// данная горячая клавиша включает режим кликов правой кнопкой мыши
			MOD_CONTROL | MOD_ALT | MOD_SHIFT,
			VK_R,
			false,
			true
		},
		{// данная горячая клавиша включает режим кликов левой кнопкой мыши
			MOD_CONTROL | MOD_ALT | MOD_SHIFT,
			VK_E,
			false,
			true
		},
		{// данная горячая клавиша активирует ввод интервала кликов
			MOD_CONTROL | MOD_ALT | MOD_SHIFT,
			VK_Z,
			false,
			true
		},
		{//данная горячая клавиша открывает окно настроек горячих клавиш программы
			MOD_CONTROL | MOD_ALT | MOD_SHIFT,
			VK_H,
			false,
			true
		}
};
BOOL FlipToTray(HWND hWnd, NOTIFYICONDATA *nf);// эта функция сворачивает окно в трей
BOOL UnflipFromTray(HWND hWnd, NOTIFYICONDATA *nf);// эта функция разворачивает окно из трея
// Отправить объявления функций, включенных в этот модуль кода:
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK PopupWindowShowMiliSecondsWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SettingsDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK HotKeySettingsDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL ShowPopupMenu(HWND hWnd, HMENU *hm);// эта функция показывает всплывающее меню, она вызываеться если щёлкнуть два раза левой кнопкой мыши по иконке этого приложения в трее
WSTRINGARRAY BreakAStringIntoAnArrayOfStringsByCharacter(WSTRING StringToBreak, WCHAR CharacterToBreak);// функция разбивает строку на массив строк по символу CharacterToBreak
WSTRING DeleteTwoCharactersInARow(WSTRING StringForDeleteTwoCharactersInARow, WCHAR SymbolForDelete);
UINT ConvertHotKeyFromControlHotKeyForRegisterHotKey(BYTE ToConvert);// функция конвертирует модификаторы горячей клавиши полученные от HotKey Control в приеемлемую форму для функции RegisterHotKey
BYTE ConvertHotKeyFromRegisterHotKeyForHotKeyControl(UINT ToConvert);// функция конвертирует модификаторы горячей клавиши предназначенные для RegisterHotKey в модификаторы предназначенные для HotKey Control
LRESULT CALLBACK KeyboardProc(_In_ int code, _In_ WPARAM wParam, _In_ LPARAM lParam);// процедура перехвата сообщений от клавиатуры для хука PP.hHookReadMiliSecondsOfClicks
#ifdef TESTED_FUNCTIONAL_CLICK_MOUSE_WHILE_THE_BUTTON_IS_PRESSED
LRESULT CALLBACK KeyboardHookClickingMouseWhileButtonIsPressedProc(_In_ int code, _In_ WPARAM wParam, LPARAM lParam);// процедура перехвата сообщений от клавиатуры для реализации второго режима работы автокликера
#endif
BOOL ReregisterHotKey(HWND hWnd, int id, UINT fsModifiers, UINT vk);
void UnregisterHotKeys(HWND hWnd, ProgrammParameters::HotKey *HotKeysForRegister, UINT HotKeyArraySize);
void RegisterHotKeys(HWND hWnd, ProgrammParameters::HotKey *HotKeysForRegister, UINT HotKeyArraySize);
template<typename A>void AssignOneArrayToAnother(A *a, A *b, UINT CountElements);// функция присваивает массиву A массив B
bool IsThereSuchAHotKeyInTheArray(ProgrammParameters::HotKey *HotKeys, UINT ArraySize, UINT UncheckedIndex, UINT Modifiers, UINT vk);// есть ли горячая клавиша с таким Modifiers и vk в массиве с горячими клавишами, не считая горячей клавиши под индексом UncheckedIndex
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
	TSTRINGARRAY CommandLineStrings = BreakAStringIntoAnArrayOfStringsByCharacter(lpCmdLine, L' ');
	for (size_t i = 0; i < CommandLineStrings.size(); i++){
		if(CommandLineStrings[i] == L"--flip-to-tray"){
			PS.TheApplicationHasATrayIcon = true;
			continue;
		}
		else if (CommandLineStrings[i] == L"--click-the-right-mouse-button") {
			PP.GlobalLeftButtonClick = false;
			PP.GlobalRightButtonClick = true;
			continue;
		}
		else if (CommandLineStrings[i] == L"--set-miliseconds-of-clicks") {
			if(i != CommandLineStrings.size() - 1) {
				DWORD MiliSecondsOfClicks = 1;
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
					wsprintf(ErrorText, L"Слишком большое значение аргумента --set-miliseconds-of-clicks, оно не может быть больше чем %u", PP.MaxMiliSeconds);
					MessageBox(NULL, ErrorText, L"Слишком большое значение аргумента командной строки --set-miliseconds-of-clicks.", MB_OK | MB_ICONERROR);
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
	   HWND hWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_SETTINGS), NULL, SettingsDlgProc);
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
   PS.GlobalHWND = hWnd;
   return TRUE;
}
void UnregisterHotKeys(HWND hWnd, ProgrammParameters::HotKey *HotKeysForRegister, UINT HotKeyArraySize){
	for (UINT i = 0; i < HotKeyArraySize; i++) {
		if (HotKeysForRegister[i].HotKeyIsRegistred && HotKeysForRegister[i].HotKeyIsEnable) {
			UnregisterHotKey(hWnd, i);
		}
	}
}
void RegisterHotKeys(HWND hWnd, ProgrammParameters::HotKey *HotKeysForRegister, UINT HotKeyArraySize) {
	for (UINT i = 0; i < HotKeyArraySize; i++) {
		if (HotKeysForRegister[i].HotKeyIsEnable) {
			if (RegisterHotKey(hWnd, i, HotKeysForRegister[i].Modifiers, HotKeysForRegister[i].vk) == 0) {
				DWORD RHKError = GetLastError();
				LPTSTR BufferForFormatMessage = nullptr;
				DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
				if (FMResult == 0) {
					TCHAR ErrorText[200];
					wsprintf(ErrorText, _TEXT("Не удалось зарегистрировать горячую клавишу с ИД %u!"), i);
					MessageBox(hWnd, ErrorText, _TEXT("Не удалось зарегистрировать горячую клавишу!"), MB_OK | MB_ICONERROR);
					wsprintf(ErrorText, _TEXT("Не удалось узнать причину ошибки регистрации горячей клавиши с ИД %u!"), i);
					MessageBox(hWnd, ErrorText, _TEXT("Мне не удалось причину возникновения ошибки!"), MB_OK | MB_ICONERROR);
				}
				else {
					TCHAR ErrorText[200];
					wsprintf(ErrorText, _TEXT("Не удалось зарегистрировать горячую клавишу с ИД %u!"), i);
					MessageBox(hWnd, BufferForFormatMessage, ErrorText, MB_OK | MB_ICONERROR);
				}
				if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
					TCHAR ErrorText[200];
					wsprintf(ErrorText, _TEXT("Не удалось освободить буфуер при обработке ошибки регистрации горячей клавиши с идентификатором %u!"), i);
					MessageBox(hWnd, ErrorText, _TEXT("Ошибка!"), MB_OK | MB_ICONERROR);
				}
			}
			else {
				HotKeysForRegister[i].HotKeyIsRegistred = true;
			}
		}
	}
}
template<typename A>void AssignOneArrayToAnother(A *a, A *b, UINT CountElements) {
	for (UINT i = 0; i < CountElements; i++) {
		a[i] = b[i];
	}
}
bool IsThereSuchAHotKeyInTheArray(ProgrammParameters::HotKey *HotKeys, UINT ArraySize, UINT UncheckedIndex, UINT Modifiers, UINT vk) {
	for (UINT i = 0; i < ArraySize; i++) {
		if ((i != UncheckedIndex) && (HotKeys[i].Modifiers == Modifiers) && (HotKeys[i].vk == vk)) {
			return true;
		}
	}
	return false;
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
				if (DrawText(hdc, TextMiliseconds, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE) == NULL) {
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
				else {/**
					UINT Max = CalcBItemWidth(hWnd, TextMiliseconds);
					if (Max > Width) {
						ScrollPos = MaxScrollPos = Max - Width;
						SetScrollRange(hWnd, SB_HORZ, 0, MaxScrollPos, TRUE);
						SetScrollPos(hWnd, SB_HORZ, MaxScrollPos, TRUE);
						EnableScrollBar(hWnd, SB_HORZ, ESB_DISABLE_RIGHT);
					}
					else {
						SetScrollRange(hWnd, SB_HORZ, 0, 0, TRUE);
						SetScrollPos(hWnd, SB_HORZ, 0, TRUE);
						ScrollPos = 0;
						MaxScrollPos = 0;
					}
					TextReplaced = FALSE;*/
				}
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
INT_PTR CALLBACK SettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	HWND hFrequencyOfClicks = GetDlgItem(hDlg, IDC_SLIDER_FREQUENCY_OF_CLICKS);
	HWND hLeftClcikRadioButton = GetDlgItem(hDlg, IDC_RADIO_LEFT_BUTTON_CLICK);
	HWND hRightClcikRadioButton = GetDlgItem(hDlg, IDC_RADIO_RIGHT_BUTTON_CLICK);
	HWND hWhenMinimizingTheWindowMinimizeToTray = GetDlgItem(hDlg, IDC_WHEN_MINIMIZING_THE_WINDOW_MINIMIZE_TO_TRAY);
	TCHAR szText[500];//текст который отображаеться в IDC_MILLISECONDS
	//UINT iMin = MIN_MILISECONDS;// мнимальное и максимальное значение слайдера IDC_SLIDER_FREQUENCY_OF_CLICKS, который предназначен для выбора
	// интервала кликов, который потом пишеться в переменную PP.GlobalMiliSecondsOfClicks
	switch(message){
		case WM_INITDIALOG:{
			SendMessage(hFrequencyOfClicks, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(MIN_MILISECONDS, PP.MaxMiliSeconds));
			SendMessage(hFrequencyOfClicks, TBM_SETPAGESIZE, 0, (LPARAM)10);
			SendMessage(hFrequencyOfClicks, TBM_SETSEL, (WPARAM)FALSE, (WPARAM)MAKELONG(MIN_MILISECONDS, PP.MaxMiliSeconds));
			SendMessage(hFrequencyOfClicks, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)PP.GlobalMiliSecondsOfClicks);
			HICON hDialogIcon = LoadIconW(PS.hInst, MAKEINTRESOURCEW(IDI_AUTOCLICKER));
			if(hDialogIcon != NULL){
				SendMessageW(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hDialogIcon);
				SendMessageW(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hDialogIcon);
			}
			//PP.GlobalMiliSecondsOfClicks = SendMessage(hFrequencyOfClicks, TBM_GETPOS, NULL, NULL);
			wsprintf(szText, _TEXT("Милисекунды %u"), PP.GlobalMiliSecondsOfClicks);
			SetDlgItemText(hDlg, IDC_MILLISECONDS, szText);
			if (PP.GlobalRightButtonClick) {
				SendMessage(hRightClcikRadioButton, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			}
			else{
				SendMessage(hLeftClcikRadioButton, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			}
			if (PP.WhenMinimizingTheWindowMinimizeToTray) {
				SendMessage(hWhenMinimizingTheWindowMinimizeToTray, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			}
			else {
				SendMessage(hWhenMinimizingTheWindowMinimizeToTray, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			}
			RegisterHotKeys(hDlg, ProgrammParameters::HotKeys, sizeof(ProgrammParameters::HotKeys) / sizeof(ProgrammParameters::HotKey));
			//SetTimer(hDlg, TimerId, uElapse, NULL);
			return (INT_PTR)TRUE;
		}
		case WM_HSCROLL:
			if (lParam == (LPARAM)hFrequencyOfClicks) {
				PP.GlobalMiliSecondsOfClicks = SendMessage(hFrequencyOfClicks, TBM_GETPOS, NULL, NULL);
				wsprintf(szText, _TEXT("Милисекунды %u"), PP.GlobalMiliSecondsOfClicks);
				SetDlgItemText(hDlg, IDC_MILLISECONDS, szText);
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
					SetForegroundWindow(hDlg);
					break;
				}
				case ProgrammParameters::HotKey::HotKeyOpenHotKeySettings: {
					break;
				}
				case ProgrammParameters::HotKey::HotKeyCloseId:
					if (PS.TheApplicationHasATrayIcon)Shell_NotifyIcon(NIM_DELETE, &PS.RootTrayIcon);
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
					if (PS.hHookReadMiliSecondsOfClicks == NULL) {
						DWORD RHKError = GetLastError();
						LPTSTR BufferForFormatMessage = nullptr;
						DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)& BufferForFormatMessage, NULL, nullptr);
						if (FMResult == 0) {
							MessageBox(hDlg, _TEXT("Не удалось создать хук для перехвата клавиатуры, с установления нового интервала кликов!"), _TEXT("Ошибка при выполнении функции SetWindowsHookEx!"), MB_OK | MB_ICONERROR);
							MessageBox(hDlg, _TEXT("Не удалось узнать причину предыдущей ошибки!"), _TEXT("Мне не удалось причину возникновения ошибки!"), MB_OK | MB_ICONERROR);

						}
						else {
							MessageBox(hDlg, _TEXT("Не удалось создать хук для перехвата клавиатуры, с установления нового интервала кликов!"), _TEXT("Ошибка при выполнении функции SetWindowsHookEx!"), MB_OK | MB_ICONERROR);
							MessageBox(hDlg, BufferForFormatMessage, _TEXT("Причина предыдущей ошибки"), MB_OK | MB_ICONINFORMATION);
						}
						if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
							MessageBox(hDlg, _TEXT("Не удалось освободить буфуер при обработке предыдущей ошибки!"), _TEXT("Ошибка очистки буфера!"), MB_OK | MB_ICONERROR);
						}
					}
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
			//обновление информации о количестве милисекунд, если она была изменена
			TCHAR fClicks[500];
			UINT InTextMiliseconds = 0;
			GetWindowText(hFrequencyOfClicks, fClicks, 500);
			_tscanf_s(_TEXT("%ui"), fClicks, InTextMiliseconds);
			if (InTextMiliseconds != PP.GlobalMiliSecondsOfClicks) {
				wsprintf(szText, _TEXT("Милисекунды %u"), PP.GlobalMiliSecondsOfClicks);
				SetDlgItemText(hDlg, IDC_MILLISECONDS, szText);
			}
			break;
		}
		case WM_DESTROY: {
			//для разрушения иконок трея
			if (PS.TheApplicationHasATrayIcon)Shell_NotifyIcon(NIM_DELETE, &PS.RootTrayIcon);
			DestroyIcon(PS.RootTrayIcon.hIcon);
			DestroyIcon(PS.RootTrayIcon.hBalloonIcon);
			UnregisterHotKeys(hDlg, ProgrammParameters::HotKeys, sizeof(ProgrammParameters::HotKeys) / sizeof(ProgrammParameters::HotKey));//деактивация горячих клавиш
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
				case ID_HOTKEY_SETTINGS:
					UnregisterHotKeys(hDlg, ProgrammParameters::HotKeys, sizeof(ProgrammParameters::HotKeys) / sizeof(ProgrammParameters::HotKey));
					DialogBox(PS.hInst, MAKEINTRESOURCE(IDD_HOTKEY_SETTINGS), hDlg, HotKeySettingsDlgProc);
					RegisterHotKeys(hDlg, ProgrammParameters::HotKeys, sizeof(ProgrammParameters::HotKeys) / sizeof(ProgrammParameters::HotKey));
					break;
				case IDC_SLIDER_FREQUENCY_OF_CLICKS:
					switch (HIWORD(wParam)) {
						case TB_THUMBTRACK:
							PP.GlobalMiliSecondsOfClicks = SendMessage(hFrequencyOfClicks, TBM_GETPOS, NULL, NULL);
							wsprintf(szText, _TEXT("Милисекунды %u"), PP.GlobalMiliSecondsOfClicks);
							SetDlgItemText(hDlg, IDC_MILLISECONDS, szText);
							break;
					}
					break;
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
			}
			break;
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK HotKeySettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	HWND hSelectHotKeyId = GetDlgItem(hDlg, IDC_SELECT_HOTKEY_ID);
	HWND hEnterHotKey = GetDlgItem(hDlg, IDC_ENTER_HOTKEY);
	HWND hEnableHotKey = GetDlgItem(hDlg, IDC_ENABLE_HOTKEY);
	static ProgrammParameters::HotKey HotKeys[sizeof(ProgrammParameters::HotKeys) / sizeof(ProgrammParameters::HotKey)];
	switch (message) {
		case WM_INITDIALOG:{
			AssignOneArrayToAnother(HotKeys, ProgrammParameters::HotKeys, sizeof(ProgrammParameters::HotKeys) / sizeof(ProgrammParameters::HotKey));
			TSTRINGARRAY StringInCombobox = {
				_TEXT("Кликать(ИД0)"),
				_TEXT("Открыть главное окно(ИД1)"),
				_TEXT("Выход(ИД2)"),
				_TEXT("Кликать правой(ИД3)"),
				_TEXT("Кликать левой(ИД4)"),
				_TEXT("Изменить интервал(ИД5)"),
				_TEXT("Настройки ГК(ИД6)")
			};
			for (UINT i = 0; i < sizeof(ProgrammParameters::HotKeys) / sizeof(ProgrammParameters::HotKey); i++) {
				TSTRING TextToComboBox = StringInCombobox[i];
				LRESULT ResultSendMessage = SendMessage(hSelectHotKeyId, CB_ADDSTRING, NULL, (LPARAM)TextToComboBox.c_str());
				if (ResultSendMessage == CB_ERR) {
					DWORD RHKError = GetLastError();
					LPTSTR BufferForFormatMessage = nullptr;
					DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
					if (FMResult == 0) {
						OutputDebugString(_TEXT("Не удалось добавить строку в ComboBox\n"));
						OutputDebugString(_TEXT("Мне не удалось причину возникновения ошибки!\n"));

					}
					else {
						OutputDebugString(_TEXT("не удалось добавить строку в ComboBox!\n"));
						OutputDebugString(BufferForFormatMessage);
					}
					if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
						OutputDebugString(_TEXT("Не удалось освободить буфуер при обработке предыдущей ошибки!\n"));
					}
				}
				else if (ResultSendMessage == CB_ERRSPACE) {
					OutputDebugString(_TEXT("Недостаточно места для добавления новой строки в ComboBox!\n"));
				}
			}
			SendMessage(hSelectHotKeyId, CB_SETCURSEL, 0, NULL);
			WPARAM wParamForSetHotKey = MAKEWORD(HotKeys[0].vk, ConvertHotKeyFromRegisterHotKeyForHotKeyControl(HotKeys[0].Modifiers));
			if (HotKeys[0].HotKeyIsEnable)SendMessage(hEnableHotKey, BM_SETCHECK, (WPARAM)BST_CHECKED, NULL);
			else SendMessage(hEnableHotKey, BM_SETCHECK, (WPARAM)BST_UNCHECKED, NULL);
			SendMessage(hEnterHotKey, HKM_SETHOTKEY, wParamForSetHotKey, NULL);
			HICON hDialogIcon = LoadIconW(PS.hInst, MAKEINTRESOURCEW(IDI_AUTOCLICKER));
			if (hDialogIcon != NULL) {
				SendMessageW(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hDialogIcon);
				SendMessageW(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hDialogIcon);
			}
			return (INT_PTR)TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:{
					AssignOneArrayToAnother(ProgrammParameters::HotKeys, HotKeys, sizeof(ProgrammParameters::HotKeys) / sizeof(ProgrammParameters::HotKey));
					EndDialog(hDlg, IDOK);
					break;
				}
				case IDC_ENABLE_HOTKEY:{
					LRESULT SelectedHotKeyId = SendMessage(hSelectHotKeyId, CB_GETCURSEL, NULL, NULL);
					if (IsDlgButtonChecked(hDlg, IDC_ENABLE_HOTKEY) == BST_CHECKED)HotKeys[SelectedHotKeyId].HotKeyIsEnable = true;
					else HotKeys[SelectedHotKeyId].HotKeyIsEnable = false;
					break;
				}
				case IDCANCEL:
					EndDialog(hDlg, IDOK);
					break;
			}
			switch (HIWORD(wParam)) {
				case CBN_SELCHANGE:
					if ((HWND)lParam == hSelectHotKeyId) {
						LRESULT SelectedHotKeyId = SendMessage(hSelectHotKeyId, CB_GETCURSEL, NULL, NULL);
						WPARAM wParamForSetHotKey = MAKEWORD(HotKeys[SelectedHotKeyId].vk, ConvertHotKeyFromRegisterHotKeyForHotKeyControl(HotKeys[SelectedHotKeyId].Modifiers));
						if (HotKeys[SelectedHotKeyId].HotKeyIsEnable)SendMessage(hEnableHotKey, BM_SETCHECK, (WPARAM)BST_CHECKED, NULL);
						else SendMessage(hEnableHotKey, BM_SETCHECK, (WPARAM)BST_UNCHECKED, NULL);
						SendMessage(hEnterHotKey, HKM_SETHOTKEY, wParamForSetHotKey, NULL);
					}
					break;
				case EN_CHANGE:{
					LRESULT EnteredHotKey = SendMessage(hEnterHotKey, HKM_GETHOTKEY, NULL, NULL);
					if (EnteredHotKey != 0) {
						LRESULT SelectedHotKeyId = SendMessage(hSelectHotKeyId, CB_GETCURSEL, NULL, NULL);
						UINT Modifiers = ConvertHotKeyFromControlHotKeyForRegisterHotKey((UINT)HIBYTE(LOWORD(EnteredHotKey)));
						UINT vk = LOBYTE(LOWORD(EnteredHotKey));
						if (IsThereSuchAHotKeyInTheArray(HotKeys, sizeof(ProgrammParameters::HotKeys) / sizeof(ProgrammParameters::HotKey), SelectedHotKeyId, Modifiers, vk)) {
							MessageBox(hDlg, _TEXT("Такая комбинация клавиш уже назначена для другой горячей клавиши, введите другую!"), _TEXT("Невозможно назначить такую комбинацию клавиш!"), MB_OK | MB_ICONERROR);
							HotKeys[SelectedHotKeyId].Modifiers = ProgrammParameters::HotKeys[SelectedHotKeyId].Modifiers;
							HotKeys[SelectedHotKeyId].vk = ProgrammParameters::HotKeys[SelectedHotKeyId].vk;
							WPARAM wParamForSetHotKey = MAKEWORD(HotKeys[SelectedHotKeyId].vk, ConvertHotKeyFromRegisterHotKeyForHotKeyControl(HotKeys[SelectedHotKeyId].Modifiers));
							SendMessage(hEnterHotKey, HKM_SETHOTKEY, wParamForSetHotKey, NULL);
						}
						else {
							HotKeys[SelectedHotKeyId].Modifiers = Modifiers;
							HotKeys[SelectedHotKeyId].vk = vk;
						}
					}
					break;
				}
			}
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
WSTRING DeleteTwoCharactersInARow(WSTRING StringForDeleteTwoCharactersInARow, WCHAR SymbolForDelete) {// данная функция удаляет заданный дублирующийся символ
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

UINT ConvertHotKeyFromControlHotKeyForRegisterHotKey(BYTE ToConvert) {// функция конвертирует модификаторы горячей клавиши полученные от HotKey Control в приеемлемую форму для функции RegisterHotKey
	UINT Result = 0;
	if ((ToConvert&0x04)==4) {
		Result |= MOD_ALT;
	}
	if ((ToConvert & 0x02) == 0x02) {
		Result |= MOD_CONTROL;
	}
	if ((ToConvert & 0x01) == 0x01) {
		Result |= MOD_SHIFT;
	}
	return Result;
}
BYTE ConvertHotKeyFromRegisterHotKeyForHotKeyControl(UINT ToConvert) {// функция конвертирует модификаторы горячей клавиши предназначенные для RegisterHotKey в модификаторы предназначенные для HotKey Control
	BYTE Result = 0;
	if ((ToConvert & MOD_ALT) == MOD_ALT) {
		Result |= 0x04;
	}
	if ((ToConvert & MOD_CONTROL) == MOD_CONTROL) {
		Result |= 0x02;
	}
	if ((ToConvert & MOD_SHIFT) == MOD_SHIFT) {
		Result |= 0x01;
	}
	return Result;
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
		ShowWindow(hPopupWindowShowMiliSeconds, SW_SHOWNOACTIVATE);
		UpdateWindow(hPopupWindowShowMiliSeconds);
		PopupWindowShowMiliSecondsCreate = true;
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
						return TRUE;
					case VK_DELETE:
						if (StringValueMilisecondsOfClick.size() > 0)StringValueMilisecondsOfClick.clear();
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
#ifdef TESTED_FUNCTIONAL_CLICK_MOUSE_WHILE_THE_BUTTON_IS_PRESSED
LRESULT CALLBACK KeyboardHookClickingMouseWhileButtonIsPressedProc(int code, WPARAM wParam, LPARAM lParam) {
	KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;
	if (code == HC_ACTION) {
		switch (wParam) {
			case WM_KEYDOWN:
				//if(p->vkCode == PP.)
				break;
			case WM_KEYUP:
				break;
		}
	}
	else if (code < 0) {
		return CallNextHookEx(NULL, code, wParam, lParam);
	}
}
#endif
BOOL ReregisterHotKey(HWND hWnd, int id, UINT fsModifiers, UINT vk) {
	if (UnregisterHotKey(hWnd, id) != 0) {
		if (RegisterHotKey(hWnd, id, fsModifiers, vk) != 0) return TRUE;
		else return FALSE;
	}
	else return FALSE;
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


