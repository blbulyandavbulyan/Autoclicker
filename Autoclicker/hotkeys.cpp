#include "stdafx.h"
#include "hotkeys.h"
#define VK__none_ 0xFF
//данный файл содержит реализацию функция для работы с горячими клавишами
struct ElementarHotKey {
	UINT Modifiers = NULL;
	UINT vk = NULL;
};
ElementarHotKey GetVkAndModifiersFromEnteredHotKey(WPARAM EneteredhotKey) {
	ElementarHotKey result;
	result.Modifiers = ((UINT)HIBYTE(LOWORD(EneteredhotKey)));
	result.vk = LOBYTE(LOWORD(EneteredhotKey));
	return result;
}
bool CheckVirtualKeyCodeOnTheCursorKey(UINT vk) {
	switch (vk) {
		case VK_UP://стрелка вверх
		case VK_DOWN://стрелка вниз
		case VK_RIGHT://стрелка вправо
		case VK_LEFT:// стрелка влево
		case VK_INSERT:// клавиша Insert
		case VK_PRIOR:// клавиша PgUp
		case VK_NEXT:// клавиша PgDown
		case VK_HOME:// клавиша Home
		case VK_END:// клавиша End
			return true;
	}
	return false;
}

//LRESULT SetKeyForRegisterHotKeyInHotKeyControl(HWND hHotKeyCtrl, UINT vk, UINT Modifiers) {
//	WPARAM wParamForSetHotKey = MAKEWORD(HotKeys[0].vk, (CheckVirtualKeyCodeOnTheCursorKey(HotKeys[0].vk) ? ConvertHotKeyFromRegisterHotKeyForHotKeyControl(HotKeys[0].Modifiers));
//	return SendMessage(hEnterHotKey, HKM_SETHOTKEY, wParamForSetHotKey, NULL);
//}
// функция конвертирует модификаторы горячей клавиши полученные от HotKey Control в приеемлемую форму для функции RegisterHotKey
UINT ConvertHotKeyFromControlHotKeyForRegisterHotKey(BYTE ToConvert) {
	UINT Result = 0;
	//клавиша ALT
	if ((ToConvert & HOTKEYF_ALT) == HOTKEYF_ALT)Result |= MOD_ALT;
	//клавиша Ctrl
	if ((ToConvert & HOTKEYF_CONTROL) == HOTKEYF_CONTROL)Result |= MOD_CONTROL;
	//клавиша Shift
	if ((ToConvert & HOTKEYF_SHIFT) == HOTKEYF_SHIFT)Result |= MOD_SHIFT;
	return Result;

}
// функция конвертирует модификаторы горячей клавиши предназначенные для RegisterHotKey в модификаторы предназначенные для HotKey Control
BYTE ConvertHotKeyFromRegisterHotKeyForHotKeyControl(UINT ToConvert) {
	BYTE Result = 0;
	//клавиша ALT
	if ((ToConvert & MOD_ALT) == MOD_ALT)Result |= HOTKEYF_ALT;
	//клавиша Ctrl
	if ((ToConvert & MOD_CONTROL) == MOD_CONTROL)Result |= HOTKEYF_CONTROL;
	//клавиша Shift
	if ((ToConvert & MOD_SHIFT) == MOD_SHIFT)Result |= HOTKEYF_SHIFT;
	return Result;
}
//функция перегистрирует горячую клавишу
BOOL ReregisterHotKey(HWND hWnd, int id, UINT fsModifiers, UINT vk) {
	if (UnregisterHotKey(hWnd, id) != 0) {
		if (RegisterHotKey(hWnd, id, fsModifiers, vk) != 0) return TRUE;
		else return FALSE;
	}
	else return FALSE;
}
//данная функция дерегистрирует(отключает) горячие клавиши из массива горячих клавиш
void UnregisterHotKeys(HWND hWnd, ProgrammParameters::HotKey* HotKeysForUnRegister, UINT HotKeyArraySize) {
	for (UINT i = 0; i < HotKeyArraySize; i++) {
		if (HotKeysForUnRegister[i].HotKeyIsRegistred && HotKeysForUnRegister[i].HotKeyIsEnable) {
			UnregisterHotKey(hWnd, i);
			HotKeysForUnRegister[i].HotKeyIsRegistred = false;
		}
	}
}
//данная функция регистрирует горячие клавиши из массива горячих клавищ
void RegisterHotKeys(HWND hWnd, ProgrammParameters::HotKey* HotKeysForRegister, UINT HotKeyArraySize) {
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
//функция проверяет, существует ли горячая клавиша в массиве, кроме той индекс которой равен UncheckedIndex
bool IsThereSuchAHotKeyInTheArray(ProgrammParameters::HotKey* HotKeys, UINT ArraySize, UINT UncheckedIndex, UINT Modifiers, UINT vk) {
	for (UINT i = 0; i < ArraySize; i++) {
		if ((i != UncheckedIndex) && (HotKeys[i].Modifiers == Modifiers) && (HotKeys[i].vk == vk))return true;
	}
	return false;
}
LRESULT CALLBACK HotKeySubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam,LPARAM lParam,
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
	if (uIdSubclass != 0)return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	switch (uMsg){
		case WM_PAINT:{
			ElementarHotKey HK = GetVkAndModifiersFromEnteredHotKey(SendMessageW(hWnd, HKM_GETHOTKEY, 0, 0));
			struct GetKeyNameData {
				unsigned : 16;//unused
				unsigned ScanCode : 8;//Скан код клаивиши
				unsigned Extended : 1;//является ли клавиша расширенной
				unsigned DNC : 1;//Игнорировать левый правый шифт и т.д.
				unsigned : 6;//unused
			};
			if (HK.vk == VK__none_) {
				WSTRING StringToOutput;
				auto MyGetKeyName = [hWnd, uMsg, wParam, lParam](UINT vk, LPWSTR szKeyName, int cchSize)->POINT {
					GetKeyNameData GKND = { 0 };
					GKND.ScanCode = MapVirtualKeyW(vk, MAPVK_VK_TO_VSC);
					GKND.Extended = 0;
					GKND.DNC = 0;
					POINT Result;
					Result.x = GetKeyNameTextW(*((LONG*)&GKND), szKeyName, cchSize) != NULL;
					if (Result.x == 0) {
						TSTRING ErrorText = _TEXT("Не удалось получить имя данной клавиши клавиши, её vk:") + to_tstring(vk), ErrorCaption = _TEXT("Ошибка в функции ") __FUNCTION__  _TEXT(" на строке ") + to_tstring(__LINE__);
						MessageDebug(ErrorText.c_str(), ErrorCaption.c_str());
						Result.y = DefSubclassProc(hWnd, uMsg, wParam, lParam);
					}
					return Result;
				};
				POINT Result = { 0 };
				WCHAR szKeyName[50] = { 0 };
				UINT FlagsCheckHotKey[3] = { HOTKEYF_ALT , HOTKEYF_SHIFT, HOTKEYF_CONTROL };
				UINT VkCodes[3] = { VK_MENU, VK_SHIFT, VK_CONTROL };
				for (BYTE i = 0; i < 3; i++) {
					if ((HK.Modifiers & FlagsCheckHotKey[i]) == FlagsCheckHotKey[i]) {
						Result = MyGetKeyName(VkCodes[i], szKeyName, 50);
						if (Result.x == 0) return Result.y;
						else {
							if (StringToOutput.size() > 0) {
								StringToOutput += L" + ";
								StringToOutput += szKeyName;
							}
							else StringToOutput += szKeyName;
						}
					}
				}
				if (StringToOutput.size() != 0)StringToOutput += L" + Fn";
				else StringToOutput += L"Fn";
				if (SetWindowTextW(hWnd, StringToOutput.c_str()) == 0)MessageDebug(_TEXT("Не удалось установить текст в HotKeyControl"), _TEXT("Ошибка в функции ") __FUNCTION__);
				return TRUE;
			}
			else return DefSubclassProc(hWnd, uMsg, wParam, lParam);
		}
		default:
			return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}
}
//диалоговая процедура окна настройки горячих клавиш
INT_PTR CALLBACK HotKeySettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	HWND hSelectHotKeyId = GetDlgItem(hDlg, IDC_SELECT_HOTKEY_ID);
	HWND hEnterHotKey = GetDlgItem(hDlg, IDC_ENTER_HOTKEY);
	HWND hEnableHotKey = GetDlgItem(hDlg, IDC_ENABLE_HOTKEY);
	static ProgrammParameters::HotKey* HotKeys = nullptr;
	static PHotKeySettingsDlgParameters Params = nullptr;
	switch (message) {
	case WM_INITDIALOG: {
		Params = (PHotKeySettingsDlgParameters)lParam;
		if ((Params->HotKeysCount != 0) && (Params != nullptr) && (Params->PHotKeys != nullptr))HotKeys = new ProgrammParameters::HotKey[Params->HotKeysCount];
		else {
			EndDialog(hDlg, FAST_FAIL_INVALID_ARG);
			return 0;
		}
		AssignOneArrayToAnother(HotKeys, Params->PHotKeys, Params->HotKeysCount);
		//данный массив следует вынести в (структуру состояния программы)/(в структуру параметров программы) или вообще сделать глобальным
		for (UINT i = 0; i < Params->HotKeysCount; i++) {
			LRESULT ResultSendMessage = SendMessage(hSelectHotKeyId, CB_ADDSTRING, NULL, (LPARAM)HotKeys[i].HotKeyNameInHotKeySettingsDlg);
			if (ResultSendMessage == CB_ERR) {
				TSTRING DMsg = _TEXT("Не удалось добавить строку \"");
				DMsg += HotKeys[i].HotKeyNameInHotKeySettingsDlg;
				DMsg += _TEXT("\" в ComboBox");
				MessageDebug(DMsg.c_str(), _TEXT("Ошибка в функции ") __FUNCTION__);
			}
			else if (ResultSendMessage == CB_ERRSPACE) {
				TSTRING DMsg = _TEXT("Ошибка в функцции ") __FUNCTION__ _TEXT(": Не удалось добавить строку \"");
				DMsg += HotKeys[i].HotKeyNameInHotKeySettingsDlg;
				DMsg += _TEXT("\" в ComboBox!\n");
				DMsg += _TEXT("Недостаточно места для добавления новой строки в ComboBox!\n");
				OutputDebugString(DMsg.c_str());
			}
		}
		SendMessage(hSelectHotKeyId, CB_SETCURSEL, 0, NULL);
		WPARAM wParamForSetHotKey = MAKEWORD(HotKeys[0].vk,ConvertHotKeyFromRegisterHotKeyForHotKeyControl(HotKeys[0].Modifiers));
		SendMessage(hEnterHotKey, HKM_SETHOTKEY, wParamForSetHotKey, NULL);
		if (HotKeys[0].HotKeyIsEnable)SendMessage(hEnableHotKey, BM_SETCHECK, (WPARAM)BST_CHECKED, NULL);
		else SendMessage(hEnableHotKey, BM_SETCHECK, (WPARAM)BST_UNCHECKED, NULL);
		//HICON hDialogIcon = LoadIcon(PS.hInst, MAKEINTRESOURCE(IDI_AUTOCLICKER));
		if (Params->hDialogIcon != NULL) {
			SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)Params->hDialogIcon);
			SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)Params->hDialogIcon);
		}
		SetWindowSubclass(hEnterHotKey, HotKeySubclassProc, 0, 0);
		//данный вызов предназначен для того, чтобы вывести окно на передний план, он здесь нужен для корректной работы горячей клавиши вызова настроек горячих клавиш
		SetForegroundWindow(hDlg);
		return (INT_PTR)TRUE;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ENABLE_HOTKEY: {
			LRESULT SelectedHotKeyId = SendMessage(hSelectHotKeyId, CB_GETCURSEL, NULL, NULL);
			if (IsDlgButtonChecked(hDlg, IDC_ENABLE_HOTKEY) == BST_CHECKED)HotKeys[SelectedHotKeyId].HotKeyIsEnable = true;
			else HotKeys[SelectedHotKeyId].HotKeyIsEnable = false;
			break;
		}
		case IDC_BUTTON_RESET_ALL_HOTKEYS: {
			if (MessageBox(hDlg, _T("Сбросить настройки горячих клавиш?"), _T("Вы уверены?!?"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
				ProgrammParameters PPL;
				AssignOneArrayToAnother(HotKeys, PPL.HotKeys, Params->HotKeysCount);
				LRESULT SelectedHotKeyId = SendMessage(hSelectHotKeyId, CB_GETCURSEL, NULL, NULL);
				WPARAM wParamForSetHotKey = MAKEWORD(HotKeys[SelectedHotKeyId].vk, ConvertHotKeyFromRegisterHotKeyForHotKeyControl(HotKeys[SelectedHotKeyId].Modifiers));
				if (HotKeys[SelectedHotKeyId].HotKeyIsEnable)SendMessage(hEnableHotKey, BM_SETCHECK, (WPARAM)BST_CHECKED, NULL);
				else SendMessage(hEnableHotKey, BM_SETCHECK, (WPARAM)BST_UNCHECKED, NULL);
				SendMessage(hEnterHotKey, HKM_SETHOTKEY, wParamForSetHotKey, NULL);
			}
			break;
		}
		case IDC_BUTTON_HOTKEY_RESET: {
			ProgrammParameters PPL;
			LRESULT SelectedHotKeyId = SendMessage(hSelectHotKeyId, CB_GETCURSEL, NULL, NULL);
			HotKeys[SelectedHotKeyId].Modifiers = PPL.HotKeys[SelectedHotKeyId].Modifiers;
			HotKeys[SelectedHotKeyId].vk = PPL.HotKeys[SelectedHotKeyId].vk;
			HotKeys[SelectedHotKeyId].HotKeyIsEnable = PPL.HotKeys[SelectedHotKeyId].HotKeyIsEnable;
			WPARAM wParamForSetHotKey = MAKEWORD(HotKeys[SelectedHotKeyId].vk, ConvertHotKeyFromRegisterHotKeyForHotKeyControl(HotKeys[SelectedHotKeyId].Modifiers));
			if (HotKeys[SelectedHotKeyId].HotKeyIsEnable)SendMessage(hEnableHotKey, BM_SETCHECK, (WPARAM)BST_CHECKED, NULL);
			else SendMessage(hEnableHotKey, BM_SETCHECK, (WPARAM)BST_UNCHECKED, NULL);
			SendMessage(hEnterHotKey, HKM_SETHOTKEY, wParamForSetHotKey, NULL);
			break;
		}
		case IDOK:
			AssignOneArrayToAnother(Params->PHotKeys, HotKeys, Params->HotKeysCount);
		case IDCANCEL:
			RemoveWindowSubclass(hEnterHotKey, HotKeySubclassProc, 0);
			delete[] HotKeys;
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
		case EN_CHANGE: {
			LRESULT EnteredHotKey = SendMessage(hEnterHotKey, HKM_GETHOTKEY, NULL, NULL);
			if (EnteredHotKey != 0) {
				UINT SelectedHotKeyId = SendMessage(hSelectHotKeyId, CB_GETCURSEL, NULL, NULL);
				//if ((HIBYTE(LOWORD(EnteredHotKey)) & HOTKEYF_EXT) == HOTKEYF_EXT)__debugbreak();
				UINT Modifiers = ConvertHotKeyFromControlHotKeyForRegisterHotKey((UINT)HIBYTE(LOWORD(EnteredHotKey)));
				UINT vk = LOBYTE(LOWORD(EnteredHotKey));
				if (IsThereSuchAHotKeyInTheArray(HotKeys, Params->HotKeysCount, SelectedHotKeyId, Modifiers, vk)) {
					MessageBox(hDlg, _TEXT("Такая комбинация клавиш уже назначена для другой горячей клавиши, введите другую!"), _TEXT("Невозможно назначить такую комбинацию клавиш!"), MB_OK | MB_ICONERROR);
					HotKeys[SelectedHotKeyId].Modifiers = Params->PHotKeys[SelectedHotKeyId].Modifiers;
					HotKeys[SelectedHotKeyId].vk = Params->PHotKeys[SelectedHotKeyId].vk;
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