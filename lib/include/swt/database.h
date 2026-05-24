#pragma once

#include <string>
#include <sqlite3.h>

namespace sf {

class Database {
public:
    explicit Database(const std::string& path);
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    sqlite3* handle() { return db_; }
    void migrate();

private:
    sqlite3* db_ = nullptr;
};

} // namespace sf
