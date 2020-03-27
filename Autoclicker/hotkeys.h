#pragma once
#include "stdafx.h"
#include "Autoclicker.h"
//� ������ ����� ����������� ���������� �������  ������ � �������� ���������
UINT ConvertHotKeyFromControlHotKeyForRegisterHotKey(BYTE ToConvert);// ������� ������������ ������������ ������� ������� ���������� �� HotKey Control � ����������� ����� ��� ������� RegisterHotKey
BYTE ConvertHotKeyFromRegisterHotKeyForHotKeyControl(UINT ToConvert);// ������� ������������ ������������ ������� ������� ��������������� ��� RegisterHotKey � ������������ ��������������� ��� HotKey Control
BOOL ReregisterHotKey(HWND hWnd, int id, UINT fsModifiers, UINT vk);//������� ���������������� ������� �������
void UnregisterHotKeys(HWND hWnd, ProgrammParameters::HotKey* HotKeysForRegister, UINT HotKeyArraySize);//������� ������� ����������� (��������������) ������� �������
void RegisterHotKeys(HWND hWnd, ProgrammParameters::HotKey* HotKeysForRegister, UINT HotKeyArraySize);//������� ������������ ������� �������
bool IsThereSuchAHotKeyInTheArray(ProgrammParameters::HotKey* HotKeys, UINT ArraySize, UINT UncheckedIndex, UINT Modifiers, UINT vk);// ���� �� ������� ������� � ����� Modifiers � vk � ������� � �������� ���������, �� ������ ������� ������� ��� �������� UncheckedIndex
INT_PTR CALLBACK HotKeySettingsDlgProc(HWND, UINT, WPARAM, LPARAM);//���������� ��������� ���� ��������� ������� ������
//������� ����������� ���� ������ ������� �������
template<typename A>void AssignOneArrayToAnother(A* a, A* b, UINT CountElements) {
	for (UINT i = 0; i < CountElements; i++)a[i] = b[i];
}
struct HotKeySettingsDlgParameters {
	ProgrammParameters::HotKey* PHotKeys = nullptr;
	UINT HotKeysCount = 0;
	HICON hDialogIcon = NULL;
};
typedef HotKeySettingsDlgParameters* PHotKeySettingsDlgParameters;
typedef const HotKeySettingsDlgParameters* PCHotKeySettingsDlgParameters;