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
//корневой ключ реестра программы(он будет создат в HKEY_CURRENT_USER
#define PROGRAMM_ROOT_REGISTRY_KEY _TEXT("SOFTWARE\\Blbulyan Software\\Autoclicker")
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
	UINT GlobalMiliSecondsOfClicks = 10; // глобальная переменная которая содержит интервал кликов
	WORD MaxMiliSeconds = 1000;// максимальное значение интервала кликов
	bool WhenMinimizingTheWindowMinimizeToTray = false;// при сворачивании окна сворачивать программу в трей
	//перечисление с флагами, которые регулируют определяют смысл операции запроса к реестру
	/*
		Если вызывается функция сохранения настроек, то соответсвующие флаги добавляют настойки к сохранению
		Если вызывается функция загрузки настроек, то соответсвующие флаги добавляют настойки к загрузке
	*/
	enum QuerySettingsInRegistryFlags {
		//мы присваеваем первому элементу перечисления единицу, т.к. если он будет равен нулю, то флаги будут работать некорректно
		REG_GENERAL_SETTINGS_QUERY = 1, //сохрнаить общие настройки
		REG_HOTKEY_SETTINGS_QUERY //сохрнанить настройки горячих клавиш 
	};
	struct HotKey {
		//ВНИМАНИЕ! Порядок идентификаторов должен совпадать по смыслу с порядком горячих клавиш!
		enum HotKeysList {// перечисление с идентификаторами горячих клавиш
			HotKeyClickStartId, HotKeyOpenMainWindow, HotKeyCloseId, HotKeyRightButtonClickId, HotKeyLeftButtonClickId, HotKeySetIntervalOfClicksId, HotKeyOpenHotKeySettings
		};
		UINT Modifiers = NULL;//модификаторы
		UINT vk = NULL;// код виртуальной клавишы
		bool HotKeyIsRegistred = false;//горячая клавиша зарегистрирована
		bool HotKeyIsEnable = false;//включена ли горячая клавиша
		LPCTSTR RegistryHotKeyName;
	};
	//массив с горячими клавишами программы
	HotKey HotKeys[7] = 
	{
		{//данная горячая клавиша включает и отключает процесс кликов
			MOD_CONTROL | MOD_ALT | MOD_SHIFT,
			VK_C,
			false,
			true,
			_TEXT("HotKeyClickStart")
		},
		{//горячая клавиша открывает главное окно программы
			MOD_CONTROL | MOD_ALT | MOD_SHIFT,
			VK_S,
			false,
			true,
			_TEXT("HotKeyOpenMainWindow")
		},
		{// данная горячая клавиша завершает работу программы
			MOD_CONTROL | MOD_ALT | MOD_SHIFT,
			VK_Q,
			false,
			true,
			_TEXT("HotKeyClose")
		},
		{// данная горячая клавиша включает режим кликов правой кнопкой мыши
			MOD_CONTROL | MOD_ALT | MOD_SHIFT,
			VK_R,
			false,
			true,
			_TEXT("HotKeyRightButtonClick")
		},
		{// данная горячая клавиша включает режим кликов левой кнопкой мыши
			MOD_CONTROL | MOD_ALT | MOD_SHIFT,
			VK_E,
			false,
			true,
			_TEXT("HotKeyLeftButtonClick")
		},
		{// данная горячая клавиша активирует ввод интервала кликов
			MOD_CONTROL | MOD_ALT | MOD_SHIFT,
			VK_Z,
			false,
			true,
			_TEXT("HotKeySetIntervalOfClicks")
		},
		{//данная горячая клавиша открывает окно настроек горячих клавиш программы
			MOD_CONTROL | MOD_ALT | MOD_SHIFT,
			VK_H,
			false,
			true,
			_TEXT("HotKeyOpenHotKeySettings")
		}
	};
	void LoadSettingsFromRegistry(UINT OpCode);//функцция загружает настройки из реестра
	void SaveSettingsInRegistry(UINT OpCode);//функции сохраняет настройки в реестр
} PP;
//масссив с именами горячих клавиш для записи их настроек в реестр  
//функция загружает настройки из реестра
void ProgrammParameters::LoadSettingsFromRegistry(UINT OpCode) {
	//загрузка общих настроек программы
	if ((OpCode & ProgrammParameters::REG_GENERAL_SETTINGS_QUERY) == ProgrammParameters::REG_GENERAL_SETTINGS_QUERY) {

	}
	//загрузка настроек горячих клавиш
	if ((OpCode & ProgrammParameters::REG_HOTKEY_SETTINGS_QUERY) == ProgrammParameters::REG_HOTKEY_SETTINGS_QUERY) {

	}
}
//функция сохраняет настройки в реестр
void ProgrammParameters::SaveSettingsInRegistry(UINT OpCode) {
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
				if (RegistryOperationResult != ERROR_SUCCESS)MessageError(_TEXT("Ошибка при сохранении параметров в реестр. Не удалось создать параметр IntervalOfClicks в ветке реестра ") PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings\n"), _TEXT("Не удалось создать параметр в реестре"), NULL, RegistryOperationResult);
				//создание параметра MouseButtonForClick в реестре
				RegistryOperationResult = RegSetValueEx(hRootSettings, _TEXT("MouseButtonForClick"), NULL, REG_EXPAND_SZ, (BYTE*)(this->GlobalLeftButtonClick ? _TEXT("LBTN") : _TEXT("RBTN")), sizeof(_TEXT("LBTN")));// создание переменной в реестре для хранения информации о том какой кнопкой мыши кликать
				if (RegistryOperationResult != ERROR_SUCCESS)MessageError(_TEXT("Ошибка при сохранении параметров в реестр. Не удалось создать параметр MouseButtonForClick в ветке реестра ") PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings\n"), _TEXT("Не удалось создать параметр в реестре"), NULL, RegistryOperationResult);
				//создание параметра WhenStartingTheProgramMinimizeTheWindowToTray в реестре
				RegistryOperationResult = RegSetValueEx(hRootSettings, _TEXT("WhenStartingTheProgramMinimizeTheWindowToTray"), NULL, REG_BINARY, (BYTE*)&this->WhenMinimizingTheWindowMinimizeToTray, sizeof(this->WhenMinimizingTheWindowMinimizeToTray));// создание переменной в реестре которая отвечает за то, нужно ли сворачивать программу в трей при запуске, если значение равно 0 то не нужно, если больше нуля, то нужно
				if (RegistryOperationResult != ERROR_SUCCESS)MessageError(_TEXT("Ошибка при сохранении параметров в реестр. Не удалось создать параметр WhenStartingTheProgramMinimizeTheWindowToTray в ветке реестра ") PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings\n"), _TEXT("Не удалось создать параметр в реестре"), NULL, RegistryOperationResult);
				//процедура сохранения конфигурации горячих клавиш
			}
			else MessageError(_TEXT("Не удалось создать ветку в реестре по пути HKEY_CURRENTUSER\\") PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings"), _TEXT("Ошибка создания ветки в реестре!"), NULL, RegistryOperationResult);
			CommitTransaction(hTransaction);
			CloseHandle(hTransaction);
		}
		else MessageError(_TEXT("Не удалось создать транзакцию для создания ветки в реестре по пути HKEY_CURRENTUSER\\") PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings"), _TEXT("Ошибка создания транзакции!"), NULL);
	}
	//сохранение настроек горячих клавиш
	if ((OpCode & ProgrammParameters::REG_HOTKEY_SETTINGS_QUERY) == ProgrammParameters::REG_HOTKEY_SETTINGS_QUERY) {
		DWORD lpwdDispositionFromHotKetSettingsKey = 0;
		HKEY hRootHotKeySettings = 0;
		HANDLE hTransaction = CreateTransaction(NULL, 0, 0, 0, 0, INFINITE, (LPWSTR)L"Creation HotKey Settings");
		if (hTransaction != NULL) {
			RegistryOperationResult = RegCreateKeyTransacted(HKEY_CURRENT_USER, PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings\\HotKeys"), NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hRootHotKeySettings, &lpwdDispositionFromHotKetSettingsKey, hTransaction, NULL);// создание ветки реестра с настройками горячих клавиш
			if (RegistryOperationResult == ERROR_SUCCESS) {
				//На данный момент настройки горячей клавиши сохраняются в реестр в бинарном виде
				for (UINT i = 0; i < sizeof(ProgrammParameters::HotKeys) / sizeof(ProgrammParameters::HotKey); i++) {
					HKEY hRootHotKey = NULL;
					RegistryOperationResult = RegCreateKeyTransacted(hRootHotKeySettings, this->HotKeys[i].RegistryHotKeyName, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hRootHotKey, &lpwdDispositionFromHotKetSettingsKey, hTransaction, NULL);
					if (RegistryOperationResult == ERROR_SUCCESS) {
						RegSetValueEx(hRootHotKey, _TEXT("Modifiers"), NULL, REG_DWORD, (BYTE*)& this->HotKeys[i].Modifiers, sizeof(this->HotKeys[i].Modifiers));
						RegSetValueEx(hRootHotKey, _TEXT("vk"), NULL, REG_DWORD, (BYTE*)& this->HotKeys[i].vk, sizeof(this->HotKeys[i].vk));
						RegSetValueEx(hRootHotKey, _TEXT("HotKeyIsEnable"), NULL, REG_BINARY, (BYTE*)& this->HotKeys[i].HotKeyIsEnable, sizeof(this->HotKeys[i].HotKeyIsEnable));
					}
					else {
						TSTRING EMsg = _TEXT("Не удалось сохраненить горячую клавишу ");
						EMsg += ProgrammParameters::HotKeys[i].RegistryHotKeyName;
						EMsg += _TEXT(" в реестр ");
						EMsg += _TEXT(" по пути HKEY_CURRENT_USER\\") PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings\\HotKeys");
						MessageError(EMsg.c_str(), L"Ошибка сохранения параметра в реестр", NULL);
					}
				}
			}
			//вывод сообщения об ошибке в случае не удачи создания ключа HKEY_CURRENT_USER\SOFTWARE\\Blbulyan Software\Autoclicker\Settings\HotKeys
			else MessageError(_TEXT("Не удалось создать ветку HKEY_CURRENT_USER\\")PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings\\HotKeys"), _TEXT("Ошибка создания ветки реестра"), NULL, RegistryOperationResult);
			CommitTransaction(hTransaction);//применение изменений, коммитим транзакцию
			CloseHandle(hTransaction);//закрываем дескриптор транзакции
		}
		else MessageError(_TEXT("Не удалось создать транзакцию для создания ключа HKEY_CURRENTUSER\\") PROGRAMM_ROOT_REGISTRY_KEY _TEXT("\\Settings\\HotKeys"), _TEXT("Ошибка создания транзакции!"), NULL);
	}
}
BOOL FlipToTray(HWND hWnd, NOTIFYICONDATA *nf);// эта функция сворачивает окно в трей
BOOL UnflipFromTray(HWND hWnd, NOTIFYICONDATA *nf);// эта функция разворачивает окно из трея
// Отправить объявления функций, включенных в этот модуль кода:
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK PopupWindowShowMiliSecondsWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SettingsDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK HotKeySettingsDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL ShowPopupMenu(HWND hWnd, HMENU *hm);// эта функция показывает всплывающее меню, она вызываеться если щёлкнуть два раза левой кнопкой мыши по иконке этого приложения в трее
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
	PP.SaveSettingsInRegistry(ProgrammParameters::REG_HOTKEY_SETTINGS_QUERY | ProgrammParameters::REG_GENERAL_SETTINGS_QUERY);
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
//данная функция дерегистрирует(отключает) горячие клавиши из массива горячих клавиш
void UnregisterHotKeys(HWND hWnd, ProgrammParameters::HotKey * HotKeysForUnRegister, UINT HotKeyArraySize){
	for (UINT i = 0; i < HotKeyArraySize; i++) {
		if (HotKeysForUnRegister[i].HotKeyIsRegistred && HotKeysForUnRegister[i].HotKeyIsEnable) {
			UnregisterHotKey(hWnd, i); 
			HotKeysForUnRegister[i].HotKeyIsRegistred = false;
		}
	}
}
//данная функция регистрирует горячие клавиши из массива горячих клавищ
void RegisterHotKeys(HWND hWnd, ProgrammParameters::HotKey *HotKeysForRegister, UINT HotKeyArraySize) {
	for (UINT i = 0; i < HotKeyArraySize; i++) {
		if (HotKeysForRegister[i].HotKeyIsEnable) {
			if (RegisterHotKey(hWnd, i, HotKeysForRegister[i].Modifiers, HotKeysForRegister[i].vk) == NULL) {
				TSTRING ErrorText = _TEXT("Не удалось зарегистрировать горячую клавишу с ИД ") + to_tstring(i);
				MessageError(ErrorText.c_str(), _TEXT("Ошибка регистрации горячей клавиши!"), hWnd);
			}
			else HotKeysForRegister[i].HotKeyIsRegistred = true;
		}
	}
}
//функция присваивает один массив другому массиву
template<typename A>void AssignOneArrayToAnother(A *a, A *b, UINT CountElements) {
	for (UINT i = 0; i < CountElements; i++)a[i] = b[i];
}
bool IsThereSuchAHotKeyInTheArray(ProgrammParameters::HotKey *HotKeys, UINT ArraySize, UINT UncheckedIndex, UINT Modifiers, UINT vk) {
	for (UINT i = 0; i < ArraySize; i++) {
		if ((i != UncheckedIndex) && (HotKeys[i].Modifiers == Modifiers) && (HotKeys[i].vk == vk))return true;
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
			RegisterHotKeys(hDlg, PP.HotKeys, sizeof(PP.HotKeys) / sizeof(ProgrammParameters::HotKey));
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
					UnregisterHotKeys(hDlg, PP.HotKeys, sizeof(ProgrammParameters::HotKeys) / sizeof(ProgrammParameters::HotKey));
					DialogBox(PS.hInst, MAKEINTRESOURCE(IDD_HOTKEY_SETTINGS), NULL, HotKeySettingsDlgProc);
					RegisterHotKeys(hDlg, PP.HotKeys, sizeof(ProgrammParameters::HotKeys) / sizeof(ProgrammParameters::HotKey));
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
				case ID_HOTKEY_SETTINGS:
					UnregisterHotKeys(hDlg, PP.HotKeys, sizeof(ProgrammParameters::HotKeys) / sizeof(ProgrammParameters::HotKey));
					DialogBox(PS.hInst, MAKEINTRESOURCE(IDD_HOTKEY_SETTINGS), hDlg, HotKeySettingsDlgProc);
					RegisterHotKeys(hDlg, PP.HotKeys, sizeof(ProgrammParameters::HotKeys) / sizeof(ProgrammParameters::HotKey));
					break;
				/*
				case IDC_SLIDER_FREQUENCY_OF_CLICKS:
					switch (HIWORD(wParam)) {
						case TB_THUMBTRACK:
							PP.GlobalMiliSecondsOfClicks = SendMessage(hFrequencyOfClicks, TBM_GETPOS, NULL, NULL);
							wsprintf(szText, _TEXT("Милисекунды %u"), PP.GlobalMiliSecondsOfClicks);
							SetDlgItemText(hDlg, IDC_MILLISECONDS, szText);
							break;
					}
					break;
				*/
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
			AssignOneArrayToAnother(HotKeys, PP.HotKeys, sizeof(ProgrammParameters::HotKeys) / sizeof(ProgrammParameters::HotKey));
			//данный массив следует вынести в (структуру состояния программы)/(в структуру параметров программы) или вообще сделать глобальным
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
				LRESULT ResultSendMessage = SendMessage(hSelectHotKeyId, CB_ADDSTRING, NULL, (LPARAM)StringInCombobox[i].c_str());
				if (ResultSendMessage == CB_ERR) {
					TSTRING DMsg = _TEXT("Не удалось добавить строку \"");
					DMsg += StringInCombobox[i];
					DMsg += _TEXT("\" в ComboBox");
					MessageDebug(DMsg.c_str(), _TEXT("Ошибка в функции ") __FUNCTION__);
				}
				else if (ResultSendMessage == CB_ERRSPACE) {
					TSTRING DMsg = _TEXT("Ошибка в функцции ") __FUNCTION__ _TEXT(": Не удалось добавить строку \"");
					DMsg += StringInCombobox[i];
					DMsg += _TEXT("\" в ComboBox!\n");
					DMsg += _TEXT("Недостаточно места для добавления новой строки в ComboBox!\n");
					OutputDebugString(DMsg.c_str());
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
			//данный вызов предназначен для того, чтобы вывести окно на передний план, он здесь нужен для корректной работы горячей клавиши вызова настроек горячих клавиш
			SetForegroundWindow(hDlg);
			return (INT_PTR)TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:{
					AssignOneArrayToAnother(PP.HotKeys, HotKeys, sizeof(ProgrammParameters::HotKeys) / sizeof(ProgrammParameters::HotKey));
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
						UINT SelectedHotKeyId = SendMessage(hSelectHotKeyId, CB_GETCURSEL, NULL, NULL);
						UINT Modifiers = ConvertHotKeyFromControlHotKeyForRegisterHotKey((UINT)HIBYTE(LOWORD(EnteredHotKey)));
						UINT vk = LOBYTE(LOWORD(EnteredHotKey));
						if (IsThereSuchAHotKeyInTheArray(HotKeys, sizeof(ProgrammParameters::HotKeys) / sizeof(ProgrammParameters::HotKey), SelectedHotKeyId, Modifiers, vk)) {
							MessageBox(hDlg, _TEXT("Такая комбинация клавиш уже назначена для другой горячей клавиши, введите другую!"), _TEXT("Невозможно назначить такую комбинацию клавиш!"), MB_OK | MB_ICONERROR);
							HotKeys[SelectedHotKeyId].Modifiers = PP.HotKeys[SelectedHotKeyId].Modifiers;
							HotKeys[SelectedHotKeyId].vk = PP.HotKeys[SelectedHotKeyId].vk;
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