#pragma once

#ifdef WIN32
#include <Windows.h>

typedef void (CALLBACK AdditionalInputHandler)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

AdditionalInputHandler* GetAdditionalInputHandler();
void SetAdditionalInputHandler(AdditionalInputHandler* handler);

#endif