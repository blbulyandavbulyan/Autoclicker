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
//функци€ присваивает один массив другому массиву
template<typename A>void AssignOneArrayToAnother(A* a, A* b, UINT CountElements) {
	for (UINT i = 0; i < CountElements; i++)a[i] = b[i];
}