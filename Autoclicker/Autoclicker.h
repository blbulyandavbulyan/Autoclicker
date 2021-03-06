#pragma once
#include "stdafx.h"
#include "Resource.h"
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
		REG_GENERAL_SETTINGS_QUERY = 0x00000001, //сохрнаить общие настройки
		REG_HOTKEY_SETTINGS_QUERY =  0x00000010//сохрнанить настройки горячих клавиш 
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
		LPCTSTR RegistryHotKeyName = NULL;//имя ветки настроек горячей клавиши в реестре(имя горячей клавиши в реестре)
		LPCTSTR HotKeyNameInHotKeySettingsDlg = NULL;//имя горячей клавиши в диалоговом окне настроек горячих клавиш 
	};
	//массив с горячими клавишами программы
	HotKey HotKeys[7] =
	{
		{//данная горячая клавиша включает и отключает процесс кликов
			MOD_CONTROL | MOD_ALT | MOD_SHIFT,
			VK_C,
			false,
			true,
			_TEXT("HotKeyClickStart"),
			_TEXT("Кликать(ИД0)")
		},
		{//горячая клавиша открывает главное окно программы
			MOD_CONTROL | MOD_ALT | MOD_SHIFT,
			VK_S,
			false,
			true,
			_TEXT("HotKeyOpenMainWindow"),
			_TEXT("Открыть главное окно(ИД1)")
		},
		{// данная горячая клавиша завершает работу программы
			MOD_CONTROL | MOD_ALT | MOD_SHIFT,
			VK_Q,
			false,
			true,
			_TEXT("HotKeyClose"),
			_TEXT("Выход(ИД2)")
		},
		{// данная горячая клавиша включает режим кликов правой кнопкой мыши
			MOD_CONTROL | MOD_ALT | MOD_SHIFT,
			VK_R,
			false,
			true,
			_TEXT("HotKeyRightButtonClick"),
			_TEXT("Кликать правой(ИД3)")
		},
		{// данная горячая клавиша включает режим кликов левой кнопкой мыши
			MOD_CONTROL | MOD_ALT | MOD_SHIFT,
			VK_E,
			false,
			true,
			_TEXT("HotKeyLeftButtonClick"),
			_TEXT("Кликать левой(ИД4)")
		},
		{// данная горячая клавиша активирует ввод интервала кликов
			MOD_CONTROL | MOD_ALT | MOD_SHIFT,
			VK_Z,
			false,
			true,
			_TEXT("HotKeySetIntervalOfClicks"),
			_TEXT("Изменить интервал(ИД5)")
		},
		{//данная горячая клавиша открывает окно настроек горячих клавиш программы
			MOD_CONTROL | MOD_ALT | MOD_SHIFT,
			VK_H,
			false,
			true,
			_TEXT("HotKeyOpenHotKeySettings"),
			_TEXT("Настройки ГК(ИД6)")
		}
	};
	void LoadSettingsFromRegistry(UINT OpCode, HWND hWnd);//функцция загружает настройки из реестра
	void SaveSettingsInRegistry(UINT OpCode, HWND hWnd);//функции сохраняет настройки в реестр
	static bool RootSettingsRetistryKeyIsExist();//функция провреряет наличие корневого ключа реестра моей программы на компьютере
};
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
	NOTIFYICONDATA RootTrayIcon;//структура с информацией о трее программы
};