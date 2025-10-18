#pragma once
#include <windows.h>
#include "product.h"  // ДОБАВИТЬ ЭТУ СТРОКУ

// Объявления функций
void RegisterWindowClass();
HWND CreateMainWindow();
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Объявление глобальной функции для доступа к базе данных
class Database;
Database& GetDatabase();