#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>
#include "product.h"



class Database {
private:
    sqlite3* db = nullptr;

public:
    bool Open();
    void Close();

    std::vector<std::wstring> GetCategories();
    std::vector<std::wstring> GetSubcategories(const std::wstring& category);
    std::vector<Product> GetProducts(const std::wstring& category, const std::wstring& subcategory);

private:
    bool ExecuteSQL(const char* sql);
    void CreateTables();
    void InsertSampleData();
};

Database& GetDatabase();