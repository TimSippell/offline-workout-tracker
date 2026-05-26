#pragma once

#include <ncurses.h>
#include <swt/swt.h>

namespace tui {

class DataView {
public:
    DataView(WINDOW* win, sf::Repository& repo);
    void run();

private:
    void do_export();
    void do_import();

    WINDOW* win_;
    sf::Repository& repo_;
};

} // namespace tui
