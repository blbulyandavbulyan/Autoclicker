#pragma once
#include "stdafx.h"
#include "Autoclicker.h"
//в данном файле расположены объ€влени€ функций  работы с гор€чими клавишами
UINT ConvertHotKeyFromControlHotKeyForRegisterHotKey(BYTE ToConvert);// функци€ конвертирует модификаторы гор€чей клавиши полученные от HotKey Control в приеемлемую форму дл€ функции RegisterHotKey
BYTE ConvertHotKeyFromRegisterHotKeyForHotKeyControl(UINT ToConvert);// функци€ конвертирует модификаторы гор€чей клавиши предназначенные дл€ RegisterHotKey в модификаторы предназначенные дл€ HotKey Control
BOOL ReregisterHotKey(HWND hWnd, int id, UINT fsModifiers, UINT vk);//функци€ перерегистрирует гор€чую клавишу
void UnregisterHotKeys(HWND hWnd, ProgrammParameters::HotKey* HotKeysForRegister, UINT HotKeyArraySize);//функци€ удал€ет регистрацию (дерегистрирует) гор€чие клавиши
void RegisterHotKeys(HWND hWnd, ProgrammParameters::HotKey* HotKeysForRegister, UINT HotKeyArraySize);//функци€ регистрирует гор€чие клавиши
bool IsThereSuchAHotKeyInTheArray(ProgrammParameters::HotKey* HotKeys, UINT ArraySize, UINT UncheckedIndex, UINT Modifiers, UINT vk);// есть ли гор€ча€ клавиша с таким Modifiers и vk в массиве с гор€чими клавишами, не счита€ гор€чей клавиши под индексом UncheckedIndex
INT_PTR CALLBACK HotKeySettingsDlgProc(HWND, UINT, WPARAM, LPARAM);//диалогова€ процедура окна настройки гор€чих клавиш
//функци€ присваивает один массив другому массиву
template<typename A>void AssignOneArrayToAnother(A* a, A* b, UINT CountElements) {
	for (UINT i = 0; i < CountElements; i++)a[i] = b[i];
}
//структура с параметрами дл€ диалоговой процедуры настройки гор€чих клавиш
struct HotKeySettingsDlgParameters {
	ProgrammParameters::HotKey* PHotKeys = nullptr;//указатель на массив с гор€чими клавишами, из него будут читатьс€ √  дл€ инициализации диалога, в него же будут писатьс€ гор€чии клавиши после того как они будут модифицированны
	UINT HotKeysCount = 0;//количество гор€чих клавиш в массиве на который указывает PHotKeys
	HICON hDialogIcon = NULL;//иконка диалога настроек гор€чих клавиш
};
typedef HotKeySettingsDlgParameters* PHotKeySettingsDlgParameters;
typedef const HotKeySettingsDlgParameters* PCHotKeySettingsDlgParameters;