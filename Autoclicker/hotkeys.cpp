#include "stdafx.h"
#include "hotkeys.h"
//данный файл содержит реализацию функци€ дл€ работы с гор€чими клавишами
// функци€ конвертирует модификаторы гор€чей клавиши полученные от HotKey Control в приеемлемую форму дл€ функции RegisterHotKey
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
// функци€ конвертирует модификаторы гор€чей клавиши предназначенные дл€ RegisterHotKey в модификаторы предназначенные дл€ HotKey Control
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
//функци€ перегистрирует гор€чую клавишу
BOOL ReregisterHotKey(HWND hWnd, int id, UINT fsModifiers, UINT vk) {
	if (UnregisterHotKey(hWnd, id) != 0) {
		if (RegisterHotKey(hWnd, id, fsModifiers, vk) != 0) return TRUE;
		else return FALSE;
	}
	else return FALSE;
}
//данна€ функци€ дерегистрирует(отключает) гор€чие клавиши из массива гор€чих клавиш
void UnregisterHotKeys(HWND hWnd, ProgrammParameters::HotKey* HotKeysForUnRegister, UINT HotKeyArraySize) {
	for (UINT i = 0; i < HotKeyArraySize; i++) {
		if (HotKeysForUnRegister[i].HotKeyIsRegistred && HotKeysForUnRegister[i].HotKeyIsEnable) {
			UnregisterHotKey(hWnd, i);
			HotKeysForUnRegister[i].HotKeyIsRegistred = false;
		}
	}
}
//данна€ функци€ регистрирует гор€чие клавиши из массива гор€чих клавищ
void RegisterHotKeys(HWND hWnd, ProgrammParameters::HotKey* HotKeysForRegister, UINT HotKeyArraySize) {
	for (UINT i = 0; i < HotKeyArraySize; i++) {
		if (HotKeysForRegister[i].HotKeyIsEnable) {
			if (RegisterHotKey(hWnd, i, HotKeysForRegister[i].Modifiers, HotKeysForRegister[i].vk) == NULL) {
				TSTRING ErrorText = _TEXT("Ќе удалось зарегистрировать гор€чую клавишу с »ƒ ") + to_tstring(i);
				MessageError(ErrorText.c_str(), _TEXT("ќшибка регистрации гор€чей клавиши!"), hWnd);
			}
			else HotKeysForRegister[i].HotKeyIsRegistred = true;
		}
	}
}
//функци€ провер€ет, существует ли гор€ча€ клавиша в массиве, кроме той индекс которой равен UncheckedIndex
bool IsThereSuchAHotKeyInTheArray(ProgrammParameters::HotKey* HotKeys, UINT ArraySize, UINT UncheckedIndex, UINT Modifiers, UINT vk) {
	for (UINT i = 0; i < ArraySize; i++) {
		if ((i != UncheckedIndex) && (HotKeys[i].Modifiers == Modifiers) && (HotKeys[i].vk == vk))return true;
	}
	return false;
}
//диалогова€ процедура окна настройки гор€чих клавиш
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
		//данный массив следует вынести в (структуру состо€ни€ программы)/(в структуру параметров программы) или вообще сделать глобальным
		for (UINT i = 0; i < Params->HotKeysCount; i++) {
			LRESULT ResultSendMessage = SendMessage(hSelectHotKeyId, CB_ADDSTRING, NULL, (LPARAM)HotKeys[i].HotKeyNameInHotKeySettingsDlg);
			if (ResultSendMessage == CB_ERR) {
				TSTRING DMsg = _TEXT("Ќе удалось добавить строку \"");
				DMsg += HotKeys[i].HotKeyNameInHotKeySettingsDlg;
				DMsg += _TEXT("\" в ComboBox");
				MessageDebug(DMsg.c_str(), _TEXT("ќшибка в функции ") __FUNCTION__);
			}
			else if (ResultSendMessage == CB_ERRSPACE) {
				TSTRING DMsg = _TEXT("ќшибка в функцции ") __FUNCTION__ _TEXT(": Ќе удалось добавить строку \"");
				DMsg += HotKeys[i].HotKeyNameInHotKeySettingsDlg;
				DMsg += _TEXT("\" в ComboBox!\n");
				DMsg += _TEXT("Ќедостаточно места дл€ добавлени€ новой строки в ComboBox!\n");
				OutputDebugString(DMsg.c_str());
			}
		}
		SendMessage(hSelectHotKeyId, CB_SETCURSEL, 0, NULL);
		WPARAM wParamForSetHotKey = MAKEWORD(HotKeys[0].vk, ConvertHotKeyFromRegisterHotKeyForHotKeyControl(HotKeys[0].Modifiers));
		SendMessage(hEnterHotKey, HKM_SETHOTKEY, wParamForSetHotKey, NULL);
		if (HotKeys[0].HotKeyIsEnable)SendMessage(hEnableHotKey, BM_SETCHECK, (WPARAM)BST_CHECKED, NULL);
		else SendMessage(hEnableHotKey, BM_SETCHECK, (WPARAM)BST_UNCHECKED, NULL);
		//HICON hDialogIcon = LoadIcon(PS.hInst, MAKEINTRESOURCE(IDI_AUTOCLICKER));
		if (Params->hDialogIcon != NULL) {
			SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)Params->hDialogIcon);
			SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)Params->hDialogIcon);
		}
		//данный вызов предназначен дл€ того, чтобы вывести окно на передний план, он здесь нужен дл€ корректной работы гор€чей клавиши вызова настроек гор€чих клавиш
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
			if (MessageBox(hDlg, _T("—бросить настройки гор€чих клавиш?"), _T("¬ы уверены?!?"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
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
				UINT Modifiers = ConvertHotKeyFromControlHotKeyForRegisterHotKey((UINT)HIBYTE(LOWORD(EnteredHotKey)));
				UINT vk = LOBYTE(LOWORD(EnteredHotKey));
				if (IsThereSuchAHotKeyInTheArray(HotKeys, Params->HotKeysCount, SelectedHotKeyId, Modifiers, vk)) {
					MessageBox(hDlg, _TEXT("“ака€ комбинаци€ клавиш уже назначена дл€ другой гор€чей клавиши, введите другую!"), _TEXT("Ќевозможно назначить такую комбинацию клавиш!"), MB_OK | MB_ICONERROR);
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