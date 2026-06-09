#pragma once

#include <ncurses.h>
#include <owt/owt.h>

namespace tui {

class WorkoutView {
public:
    WorkoutView(WINDOW* win, sf::Repository& repo);
    void set_window(WINDOW* win) { win_ = win; }
    void run();

private:
    bool choose_start_mode();
    void select_exercise();
    void add_set();
    void edit_set_weight(int64_t set_id);
    void show_sets();
    void show_previous_performance(int64_t exercise_id);

    WINDOW* win_;
    sf::Repository& repo_;
    int64_t workout_id_ = 0;
    int64_t current_exercise_id_ = 0;
    std::string current_exercise_name_;
    int set_count_ = 0;
};

} // namespace tui
