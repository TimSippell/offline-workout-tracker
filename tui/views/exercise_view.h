#pragma once

#include <ncurses.h>
#include <owt/owt.h>

namespace tui {

class ExerciseView {
public:
    ExerciseView(WINDOW* win, sf::Repository& repo);
    void run();

private:
    void add_exercise();
    void show_exercise_detail(int64_t exercise_id);

    WINDOW* win_;
    sf::Repository& repo_;
};

} // namespace tui
