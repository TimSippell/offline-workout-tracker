#pragma once

#include <swt/swt.h>
#include "views/workout_view.h"
#include "views/history_view.h"
#include "views/exercise_view.h"
#include "views/progress_view.h"
#include "views/settings_view.h"

namespace gui {

enum class Screen {
    Workout, History, Exercises, Progress, Settings
};

class App {
public:
    explicit App(sf::Database& db);
    void render();

private:
    void render_sidebar();
    void check_first_run();

    sf::Repository repo_;
    Screen current_screen_ = Screen::Workout;

    WorkoutView workout_view_;
    HistoryView history_view_;
    ExerciseView exercise_view_;
    ProgressView progress_view_;
    SettingsView settings_view_;

    bool first_run_checked_ = false;
    bool show_seed_popup_ = false;
};

} // namespace gui
