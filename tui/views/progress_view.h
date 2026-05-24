#pragma once

#include <ncurses.h>
#include <swt/swt.h>

namespace tui {

class ProgressView {
public:
    ProgressView(WINDOW* win, sf::Repository& repo);
    void run();

private:
    void show_exercise_progress(int64_t exercise_id, const std::string& name);

    WINDOW* win_;
    sf::Repository& repo_;
};

} // namespace tui
