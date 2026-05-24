#pragma once

#include <ncurses.h>
#include <swt/swt.h>

namespace tui {

class HistoryView {
public:
    HistoryView(WINDOW* win, sf::Repository& repo);
    void run();

private:
    void show_workout_detail(int64_t workout_id);

    WINDOW* win_;
    sf::Repository& repo_;
};

} // namespace tui
