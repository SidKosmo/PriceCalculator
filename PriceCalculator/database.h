#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>

struct Product {
    int id;
    std::string name;
    double price;
    std::string category;
    std::string subcategory;
};

class Database {
private:
    sqlite3* db = nullptr;

public:
    bool Open();
    void Close();

    std::vector<std::string> GetCategories();
    std::vector<std::string> GetSubcategories(const std::string& category);
    std::vector<Product> GetProducts(const std::string& category, const std::string& subcategory);

private:
    bool ExecuteSQL(const char* sql);
    void CreateTables();
    void InsertSampleData();
};

Database& GetDatabase();