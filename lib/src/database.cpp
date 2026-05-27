#include "swt/database.h"
#include <stdexcept>

namespace sf {

Database::Database(const std::string& path) {
    int rc = sqlite3_open(path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::string err = sqlite3_errmsg(db_);
        sqlite3_close(db_);
        db_ = nullptr;
        throw std::runtime_error("Failed to open database: " + err);
    }
    sqlite3_exec(db_, "PRAGMA journal_mode=WAL", nullptr, nullptr, nullptr);
    sqlite3_exec(db_, "PRAGMA foreign_keys=ON", nullptr, nullptr, nullptr);
}

Database::~Database() {
    if (db_) sqlite3_close(db_);
}

void Database::migrate() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS exercise (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            name        TEXT NOT NULL UNIQUE,
            category    TEXT,
            muscle_group TEXT,
            notes       TEXT,
            created_at  TEXT DEFAULT (datetime('now'))
        );

        CREATE TABLE IF NOT EXISTS workout (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            name        TEXT,
            started_at  TEXT NOT NULL DEFAULT (datetime('now')),
            finished_at TEXT,
            notes       TEXT
        );

        CREATE TABLE IF NOT EXISTS workout_set (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            workout_id  INTEGER NOT NULL REFERENCES workout(id) ON DELETE CASCADE,
            exercise_id INTEGER NOT NULL REFERENCES exercise(id),
            set_order   INTEGER NOT NULL,
            reps        INTEGER,
            weight      REAL,
            rpe         REAL,
            rest_secs       INTEGER,
            duration_secs   INTEGER,
            tempo       TEXT,
            notes       TEXT,
            created_at  TEXT DEFAULT (datetime('now'))
        );

        CREATE INDEX IF NOT EXISTS idx_set_workout ON workout_set(workout_id);
        CREATE INDEX IF NOT EXISTS idx_set_exercise ON workout_set(exercise_id);

        CREATE TABLE IF NOT EXISTS workout_template (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            name        TEXT NOT NULL,
            notes       TEXT,
            created_at  TEXT DEFAULT (datetime('now'))
        );

        CREATE TABLE IF NOT EXISTS template_set (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            template_id INTEGER NOT NULL REFERENCES workout_template(id) ON DELETE CASCADE,
            exercise_id INTEGER NOT NULL REFERENCES exercise(id),
            set_order   INTEGER NOT NULL,
            reps        INTEGER,
            rpe         REAL,
            rest_secs       INTEGER,
            duration_secs   INTEGER,
            tempo       TEXT,
            notes       TEXT
        );

        CREATE INDEX IF NOT EXISTS idx_tset_template ON template_set(template_id);
    )";

    char* err = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        std::string msg = err ? err : "unknown error";
        sqlite3_free(err);
        throw std::runtime_error("Migration failed: " + msg);
    }

    sqlite3_exec(db_, "ALTER TABLE workout ADD COLUMN template_id INTEGER REFERENCES workout_template(id)",
                 nullptr, nullptr, nullptr);
    sqlite3_exec(db_, "ALTER TABLE template_set ADD COLUMN weight REAL",
                 nullptr, nullptr, nullptr);
    sqlite3_exec(db_, "ALTER TABLE workout_set ADD COLUMN duration_secs INTEGER",
                 nullptr, nullptr, nullptr);
    sqlite3_exec(db_, "ALTER TABLE template_set ADD COLUMN duration_secs INTEGER",
                 nullptr, nullptr, nullptr);

    sqlite3_exec(db_, R"(
        CREATE TABLE IF NOT EXISTS settings (
            key   TEXT PRIMARY KEY,
            value TEXT
        )
    )", nullptr, nullptr, nullptr);
}

} // namespace sf
