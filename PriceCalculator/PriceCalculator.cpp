#include "PriceCalculator.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include "database.h"

#pragma comment(lib, "comctl32.lib")

using namespace std;

// Элементы управления
HWND hCategoryCombo, hSubcategoryCombo, hProductCombo;
HWND hQuantityEdit, hAddButton, hReceiptList, hTotalLabel;
HWND hRemoveButton, hClearButton;
double totalAmount = 0.0;




void UpdateProducts() {
    ComboBox_ResetContent(hProductCombo);

    int categoryIndex = ComboBox_GetCurSel(hCategoryCombo);
    int subcategoryIndex = ComboBox_GetCurSel(hSubcategoryCombo);
    if (categoryIndex == CB_ERR || subcategoryIndex == CB_ERR) return;

    wchar_t category[100], subcategory[100];
    ComboBox_GetLBText(hCategoryCombo, categoryIndex, category);
    ComboBox_GetLBText(hSubcategoryCombo, subcategoryIndex, subcategory);

    std::string scategory(category, category + wcslen(category));
    std::string ssubcategory(subcategory, subcategory + wcslen(subcategory));

    auto products = GetDatabase().GetProducts(scategory, ssubcategory);
    for (const auto& product : products) {
        std::wstring wname(product.name.begin(), product.name.end());
        std::wstringstream productInfo;
        productInfo << wname << L" - " << std::fixed << std::setprecision(2) << product.price << L" руб.";
        ComboBox_AddString(hProductCombo, productInfo.str().c_str());
    }

    if (ComboBox_GetCount(hProductCombo) > 0) {
        ComboBox_SetCurSel(hProductCombo, 0);
    }
}

void UpdateSubcategories() {
    ComboBox_ResetContent(hSubcategoryCombo);

    int categoryIndex = ComboBox_GetCurSel(hCategoryCombo);
    if (categoryIndex == CB_ERR) return;

    wchar_t category[100];
    ComboBox_GetLBText(hCategoryCombo, categoryIndex, category);
    std::string scategory(category, category + wcslen(category));

    auto subcategories = GetDatabase().GetSubcategories(scategory);
    for (const auto& subcategory : subcategories) {
        std::wstring wsubcategory(subcategory.begin(), subcategory.end());
        ComboBox_AddString(hSubcategoryCombo, wsubcategory.c_str());
    }

    if (ComboBox_GetCount(hSubcategoryCombo) > 0) {
        ComboBox_SetCurSel(hSubcategoryCombo, 0);
        UpdateProducts();
    }
}

void FillCategories() {
    ComboBox_ResetContent(hCategoryCombo);

    auto categories = GetDatabase().GetCategories();
    for (const auto& category : categories) {
        // Преобразуем string в wstring для Windows API
        std::wstring wcategory(category.begin(), category.end());
        ComboBox_AddString(hCategoryCombo, wcategory.c_str());
    }

    if (ComboBox_GetCount(hCategoryCombo) > 0) {
        ComboBox_SetCurSel(hCategoryCombo, 0);
        UpdateSubcategories();
    }
}

void AddToReceipt() {
    int productIndex = ComboBox_GetCurSel(hProductCombo);
    if (productIndex == CB_ERR) return;

    wchar_t quantityText[10];
    GetWindowText(hQuantityEdit, quantityText, 10);
    int quantity = _wtoi(quantityText);
    if (quantity <= 0) quantity = 1;

    wchar_t productInfo[100];
    ComboBox_GetLBText(hProductCombo, productIndex, productInfo);

    // Извлекаем цену из строки товара "Название - 100.00 руб."
    wstring productStr(productInfo);
    size_t dashPos = productStr.find(L" - ");
    size_t rubPos = productStr.find(L" руб.");

    if (dashPos != wstring::npos && rubPos != wstring::npos) {
        // Извлекаем название (все до " - ")
        wstring productName = productStr.substr(0, dashPos);

        // Извлекаем цену (между " - " и " руб.")
        wstring priceStr = productStr.substr(dashPos + 3, rubPos - (dashPos + 3));
        double price = stod(priceStr);

        double totalPrice = price * quantity;

        wstringstream receiptItem;
        receiptItem << productName << L" x" << quantity
            << L" = " << fixed << setprecision(2) << totalPrice << L" руб.";

        ListBox_AddString(hReceiptList, receiptItem.str().c_str());

        totalAmount += totalPrice;

        wstringstream totalText;
        totalText << L"Общая сумма: " << fixed << setprecision(2) << totalAmount << L" руб.";
        SetWindowText(hTotalLabel, totalText.str().c_str());
    }
}

void RemoveSelectedItem() {
    int selectedIndex = ListBox_GetCurSel(hReceiptList);
    if (selectedIndex != LB_ERR) {
        wchar_t itemText[200];
        ListBox_GetText(hReceiptList, selectedIndex, itemText);

        // Извлекаем сумму из строки "Товар x2 = 100.00 руб."
        wstring itemStr(itemText);
        size_t equalsPos = itemStr.find(L"= ");
        if (equalsPos != wstring::npos) {
            wstring amountStr = itemStr.substr(equalsPos + 2);
            // Удаляем " руб." в конце
            size_t rubPos = amountStr.find(L" руб.");
            if (rubPos != wstring::npos) {
                amountStr = amountStr.substr(0, rubPos);
            }
            double itemAmount = stod(amountStr);
            totalAmount -= itemAmount;
        }

        ListBox_DeleteString(hReceiptList, selectedIndex);

        wstringstream totalText;
        totalText << L"Общая сумма: " << fixed << setprecision(2) << totalAmount << L" руб.";
        SetWindowText(hTotalLabel, totalText.str().c_str());
    }
}

void ClearReceipt() {
    ListBox_ResetContent(hReceiptList);
    totalAmount = 0.0;
    wstring totalText = L"Общая сумма: 0.00 руб.";
    SetWindowText(hTotalLabel, totalText.c_str());
}

void CreateControls(HWND hwnd) {
    

    // Метка категории
    CreateWindowW(L"STATIC", L"Категория:", WS_CHILD | WS_VISIBLE,
        10, 10, 80, 20, hwnd, NULL, NULL, NULL);

    // Комбобокс категорий
    hCategoryCombo = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        100, 10, 200, 200, hwnd, NULL, NULL, NULL);

    // Метка подкатегории
    CreateWindowW(L"STATIC", L"Подкатегория:", WS_CHILD | WS_VISIBLE,
        10, 40, 80, 20, hwnd, NULL, NULL, NULL);

    // Комбобокс подкатегорий
    hSubcategoryCombo = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        100, 40, 200, 200, hwnd, NULL, NULL, NULL);

    // Метка товара
    CreateWindowW(L"STATIC", L"Товар:", WS_CHILD | WS_VISIBLE,
        10, 70, 80, 20, hwnd, NULL, NULL, NULL);

    // Комбобокс товаров
    hProductCombo = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        100, 70, 200, 200, hwnd, NULL, NULL, NULL);

    // Метка количества
    CreateWindowW(L"STATIC", L"Количество:", WS_CHILD | WS_VISIBLE,
        10, 100, 80, 20, hwnd, NULL, NULL, NULL);

    // Поле ввода количества
    hQuantityEdit = CreateWindowW(L"EDIT", L"1", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
        100, 100, 80, 20, hwnd, NULL, NULL, NULL);

    // Кнопка добавления
    hAddButton = CreateWindowW(L"BUTTON", L"Добавить в чек", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        190, 100, 110, 20, hwnd, (HMENU)1, NULL, NULL);

    // Список чека
    hReceiptList = CreateWindowW(L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | WS_VSCROLL,
        10, 130, 380, 200, hwnd, NULL, NULL, NULL);

    // Кнопка удаления
    hRemoveButton = CreateWindowW(L"BUTTON", L"Удалить выделенное", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 340, 120, 25, hwnd, (HMENU)2, NULL, NULL);

    // Кнопка очистки
    hClearButton = CreateWindowW(L"BUTTON", L"Очистить чек", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        140, 340, 100, 25, hwnd, (HMENU)3, NULL, NULL);

    // Метка общей суммы
    hTotalLabel = CreateWindowW(L"STATIC", L"Общая сумма: 0.00 руб.", WS_CHILD | WS_VISIBLE | SS_RIGHT,
        250, 340, 140, 25, hwnd, NULL, NULL, NULL);

    

    // Заполняем категории после создания всех контролов
    FillCategories();
}

void RegisterWindowClass() {
    WNDCLASS wc = {};
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"PriceCalculatorClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.style = CS_HREDRAW | CS_VREDRAW;

    RegisterClass(&wc);
}



HWND CreateMainWindow() {
    

    return CreateWindow(L"PriceCalculatorClass", L"Калькулятор стоимости товаров",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 420, 420,
        NULL, NULL, GetModuleHandle(NULL), NULL);
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        CreateControls(hwnd);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 1: // Добавить в чек
            AddToReceipt();
            break;
        case 2: // Удалить выделенное
            RemoveSelectedItem();
            break;
        case 3: // Очистить чек
            ClearReceipt();
            break;
        }

        if (HIWORD(wParam) == CBN_SELCHANGE) {
            if ((HWND)lParam == hCategoryCombo) {
                UpdateSubcategories();
            }
            else if ((HWND)lParam == hSubcategoryCombo) {
                UpdateProducts();
            }
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}