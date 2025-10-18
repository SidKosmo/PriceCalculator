#include <windows.h>
#include <commctrl.h>
#include "PriceCalculator.h"
#include "database.h"

// Глобальная переменная базы данных
Database g_database;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Инициализация Common Controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    // Инициализация базы данных
    if (!g_database.Open()) {
        MessageBoxW(NULL,
            L"Не удалось открыть базу данных!",
            L"Ошибка базы данных",
            MB_ICONERROR | MB_OK);
        return 1;
    }

    // Регистрация класса окна
    RegisterWindowClass();

    // Создание главного окна
    HWND hwnd = CreateMainWindow();
    if (!hwnd) {
        MessageBoxW(NULL, L"Не удалось создать главное окно!", L"Ошибка", MB_ICONERROR);
        g_database.Close();
        return 1;
    }

    // Показ окна
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Цикл сообщений
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Закрытие базы данных при выходе
    g_database.Close();

    return (int)msg.wParam;
}

// Функция для получения глобальной базы данных
Database& GetDatabase() {
    return g_database;
}