#pragma once

#include <ncurses.h>
#include <owt/owt.h>
#include <owt/data_io.h>
#include <optional>
#include <string>

namespace tui {

class DataView {
public:
    DataView(WINDOW* win, sf::Repository& repo);
    void run();

private:
    void do_export();
    void do_import();
    std::optional<sf::ExportScope> pick_scope(const std::string& action);
    static std::string expand_path(const std::string& raw);
    static bool is_valid_path(const std::string& path);

    WINDOW* win_;
    sf::Repository& repo_;
};

} // namespace tui
