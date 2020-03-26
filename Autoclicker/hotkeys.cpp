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