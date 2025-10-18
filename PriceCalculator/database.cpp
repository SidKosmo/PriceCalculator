#include "database.h"
#include <iostream>
#include <sstream>
#include <windows.h>

bool Database::Open() {
    int rc = sqlite3_open("products.db", &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // Устанавливаем кодировку UTF-8
    sqlite3_exec(db, "PRAGMA encoding = 'UTF-8'", nullptr, nullptr, nullptr);

    CreateTables();
    InsertSampleData();
    return true;
}

void Database::Close() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool Database::ExecuteSQL(const char* sql) {
    char* errorMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errorMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errorMsg << std::endl;
        sqlite3_free(errorMsg);
        return false;
    }
    return true;
}

// Функция для преобразования UTF-8 в std::wstring
std::wstring UTF8ToWString(const char* utf8) {
    if (!utf8 || utf8[0] == '\0') return L"";

    int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
    if (wideLen == 0) return L"";

    std::wstring wideStr(wideLen - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, &wideStr[0], wideLen);

    return wideStr;
}

void Database::CreateTables() {
    // Таблица категорий
    ExecuteSQL(
        "CREATE TABLE IF NOT EXISTS categories ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL UNIQUE)"
    );

    // Таблица подкатегорий
    ExecuteSQL(
        "CREATE TABLE IF NOT EXISTS subcategories ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "category_id INTEGER, "
        "name TEXT NOT NULL, "
        "FOREIGN KEY(category_id) REFERENCES categories(id))"
    );

    // Таблица товаров
    ExecuteSQL(
        "CREATE TABLE IF NOT EXISTS products ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "subcategory_id INTEGER, "
        "name TEXT NOT NULL, "
        "price REAL NOT NULL, "
        "FOREIGN KEY(subcategory_id) REFERENCES subcategories(id))"
    );
}

void Database::InsertSampleData() {
    // Проверяем есть ли данные
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM products", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_int(stmt, 0) > 0) {
            sqlite3_finalize(stmt);
            return;
        }
        sqlite3_finalize(stmt);
    }

    // Вставляем тестовые данные с явным указанием UTF-8
    ExecuteSQL("BEGIN TRANSACTION");

    // Категории
    ExecuteSQL("INSERT OR IGNORE INTO categories (name) VALUES ('Продукты питания')");
    ExecuteSQL("INSERT OR IGNORE INTO categories (name) VALUES ('Бытовая химия')");
    ExecuteSQL("INSERT OR IGNORE INTO categories (name) VALUES ('Электроника')");
    ExecuteSQL("INSERT OR IGNORE INTO categories (name) VALUES ('Одежда')");

    // Подкатегории
    ExecuteSQL("INSERT OR IGNORE INTO subcategories (category_id, name) VALUES (1, 'Хлебобулочные')");
    ExecuteSQL("INSERT OR IGNORE INTO subcategories (category_id, name) VALUES (1, 'Молочные продукты')");
    ExecuteSQL("INSERT OR IGNORE INTO subcategories (category_id, name) VALUES (1, 'Фрукты')");
    ExecuteSQL("INSERT OR IGNORE INTO subcategories (category_id, name) VALUES (1, 'Овощи')");

    ExecuteSQL("INSERT OR IGNORE INTO subcategories (category_id, name) VALUES (2, 'Стирка')");
    ExecuteSQL("INSERT OR IGNORE INTO subcategories (category_id, name) VALUES (2, 'Гигиена')");
    ExecuteSQL("INSERT OR IGNORE INTO subcategories (category_id, name) VALUES (2, 'Уборка')");

    ExecuteSQL("INSERT OR IGNORE INTO subcategories (category_id, name) VALUES (3, 'Аудио')");
    ExecuteSQL("INSERT OR IGNORE INTO subcategories (category_id, name) VALUES (3, 'Компьютерная техника')");
    ExecuteSQL("INSERT OR IGNORE INTO subcategories (category_id, name) VALUES (3, 'Мобильные устройства')");

    ExecuteSQL("INSERT OR IGNORE INTO subcategories (category_id, name) VALUES (4, 'Мужская')");
    ExecuteSQL("INSERT OR IGNORE INTO subcategories (category_id, name) VALUES (4, 'Женская')");

    // Товары
    ExecuteSQL("INSERT INTO products (subcategory_id, name, price) VALUES (1, 'Хлеб белый', 45.00)");
    ExecuteSQL("INSERT INTO products (subcategory_id, name, price) VALUES (1, 'Хлеб черный', 55.00)");
    ExecuteSQL("INSERT INTO products (subcategory_id, name, price) VALUES (1, 'Булочка сдобная', 35.00)");

    ExecuteSQL("INSERT INTO products (subcategory_id, name, price) VALUES (2, 'Молоко 1л', 85.00)");
    ExecuteSQL("INSERT INTO products (subcategory_id, name, price) VALUES (2, 'Сыр российский', 450.00)");
    ExecuteSQL("INSERT INTO products (subcategory_id, name, price) VALUES (2, 'Йогурт', 65.00)");
    ExecuteSQL("INSERT INTO products (subcategory_id, name, price) VALUES (2, 'Сметана', 120.00)");

    ExecuteSQL("INSERT INTO products (subcategory_id, name, price) VALUES (3, 'Яблоки', 150.00)");
    ExecuteSQL("INSERT INTO products (subcategory_id, name, price) VALUES (3, 'Бананы', 110.00)");
    ExecuteSQL("INSERT INTO products (subcategory_id, name, price) VALUES (3, 'Апельсины', 180.00)");

    ExecuteSQL("INSERT INTO products (subcategory_id, name, price) VALUES (4, 'Картофель', 60.00)");
    ExecuteSQL("INSERT INTO products (subcategory_id, name, price) VALUES (4, 'Помидоры', 250.00)");
    ExecuteSQL("INSERT INTO products (subcategory_id, name, price) VALUES (4, 'Огурцы', 180.00)");

    ExecuteSQL("COMMIT");
}

std::vector<std::wstring> Database::GetCategories() {
    std::vector<std::wstring> categories;
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, "SELECT name FROM categories ORDER BY name", -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* name = sqlite3_column_text(stmt, 0);
            if (name) {
                categories.push_back(UTF8ToWString(reinterpret_cast<const char*>(name)));
            }
        }
        sqlite3_finalize(stmt);
    }

    return categories;
}

std::vector<std::wstring> Database::GetSubcategories(const std::wstring& category) {
    std::vector<std::wstring> subcategories;
    sqlite3_stmt* stmt;

    const char* sql = "SELECT s.name FROM subcategories s "
        "JOIN categories c ON s.category_id = c.id "
        "WHERE c.name = ? ORDER BY s.name";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        // Преобразуем wstring в UTF-8 для запроса
        std::string categoryUtf8;
        int categoryLen = WideCharToMultiByte(CP_UTF8, 0, category.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (categoryLen > 0) {
            categoryUtf8.resize(categoryLen - 1);
            WideCharToMultiByte(CP_UTF8, 0, category.c_str(), -1, &categoryUtf8[0], categoryLen, nullptr, nullptr);
        }

        sqlite3_bind_text(stmt, 1, categoryUtf8.c_str(), -1, SQLITE_STATIC);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* name = sqlite3_column_text(stmt, 0);
            if (name) {
                subcategories.push_back(UTF8ToWString(reinterpret_cast<const char*>(name)));
            }
        }
        sqlite3_finalize(stmt);
    }

    return subcategories;
}

std::vector<Product> Database::GetProducts(const std::wstring& category, const std::wstring& subcategory) {
    std::vector<Product> products;
    sqlite3_stmt* stmt;

    const char* sql = "SELECT p.id, p.name, p.price FROM products p "
        "JOIN subcategories s ON p.subcategory_id = s.id "
        "JOIN categories c ON s.category_id = c.id "
        "WHERE c.name = ? AND s.name = ? ORDER BY p.name";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        // Преобразуем wstring в UTF-8 для запроса
        std::string categoryUtf8, subcategoryUtf8;

        int categoryLen = WideCharToMultiByte(CP_UTF8, 0, category.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (categoryLen > 0) {
            categoryUtf8.resize(categoryLen - 1);
            WideCharToMultiByte(CP_UTF8, 0, category.c_str(), -1, &categoryUtf8[0], categoryLen, nullptr, nullptr);
        }

        int subcategoryLen = WideCharToMultiByte(CP_UTF8, 0, subcategory.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (subcategoryLen > 0) {
            subcategoryUtf8.resize(subcategoryLen - 1);
            WideCharToMultiByte(CP_UTF8, 0, subcategory.c_str(), -1, &subcategoryUtf8[0], subcategoryLen, nullptr, nullptr);
        }

        sqlite3_bind_text(stmt, 1, categoryUtf8.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, subcategoryUtf8.c_str(), -1, SQLITE_STATIC);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Product product;
            product.id = sqlite3_column_int(stmt, 0);

            const unsigned char* name = sqlite3_column_text(stmt, 1);
            if (name) {
                product.name = UTF8ToWString(reinterpret_cast<const char*>(name));
            }

            product.price = sqlite3_column_double(stmt, 2);
            product.category = category;
            product.subcategory = subcategory;

            products.push_back(product);
        }
        sqlite3_finalize(stmt);
    }

    return products;
}