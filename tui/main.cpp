#include "app.h"
#include <cstdlib>
#include <filesystem>
#include <iostream>

int main(int argc, char* argv[]) {
    namespace fs = std::filesystem;

    std::string db_path;
    if (argc > 1) {
        db_path = argv[1];
    } else {
#ifdef _WIN32
        const char* home = std::getenv("APPDATA");
#else
        const char* home = std::getenv("HOME");
#endif
        if (home) {
#ifdef _WIN32
            fs::path dir = fs::path(home) / "simple-workout-tracker";
#else
            fs::path dir = fs::path(home) / ".local" / "share" / "simple-workout-tracker";
#endif
            fs::create_directories(dir);
            db_path = (dir / "workouts.db").string();
        } else {
            db_path = "workouts.db";
        }
    }

    try {
        sf::Database db(db_path);
        db.migrate();

        tui::App app(db);
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
