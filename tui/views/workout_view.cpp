#include "workout_view.h"
#include "../widgets/input.h"
#include <swt/calculator.h>
#include <format>

namespace tui {

WorkoutView::WorkoutView(WINDOW* win, sf::Repository& repo)
    : win_(win), repo_(repo) {}

bool WorkoutView::choose_start_mode() {
    werase(win_);
    box(win_, 0, 0);
    mvwprintw(win_, 1, 2, "Start Workout");
    mvwprintw(win_, 3, 2, "1) Start blank workout");
    mvwprintw(win_, 4, 2, "2) Start from template");
    mvwprintw(win_, 5, 2, "q) Cancel");
    wrefresh(win_);

    int ch = wgetch(win_);
    if (ch == 'q' || ch == 27) return false;

    if (ch == '2') {
        auto templates = repo_.list_templates();
        if (templates.empty()) {
            mvwprintw(win_, 7, 2, "No templates. Create one first.");
            mvwprintw(win_, 8, 2, "Press any key...");
            wrefresh(win_);
            wgetch(win_);
            return false;
        }

        werase(win_);
        box(win_, 0, 0);
        mvwprintw(win_, 1, 2, "Choose Template:");
        int row = 3;
        for (int i = 0; i < static_cast<int>(templates.size()) && row < getmaxy(win_) - 4; i++) {
            auto sets = repo_.get_template_sets(templates[i].id);
            mvwprintw(win_, row++, 2, "%d) %s (%d sets)",
                      i + 1, templates[i].name.c_str(), static_cast<int>(sets.size()));
        }
        wrefresh(win_);

        auto pick = get_int_input(win_, row + 1, 2, "Pick #: ");
        if (!pick || *pick < 1 || *pick > static_cast<int>(templates.size())) return false;

        workout_id_ = repo_.start_workout_from_template(templates[*pick - 1].id);
        auto sets = repo_.get_sets_for_workout(workout_id_);
        set_count_ = static_cast<int>(sets.size());
        return true;
    }

    std::string name = get_string_input(win_, 7, 2, "Workout name (optional): ");
    workout_id_ = repo_.start_workout(name);
    return true;
}

void WorkoutView::run() {
    if (!choose_start_mode()) return;

    bool running = true;
    while (running) {
        werase(win_);
        box(win_, 0, 0);

        wattron(win_, A_BOLD);
        mvwprintw(win_, 1, 2, "Workout #%ld", workout_id_);
        wattroff(win_, A_BOLD);

        if (!current_exercise_name_.empty()) {
            mvwprintw(win_, 2, 2, "Exercise: %s (Set %d)", current_exercise_name_.c_str(), set_count_ + 1);
            show_previous_performance(current_exercise_id_);
        }

        show_sets();

        int max_y = getmaxy(win_);
        mvwprintw(win_, max_y - 2, 2, "a:add set | w:edit weight | e:exercise | f:finish | q:cancel");
        wrefresh(win_);

        int ch = wgetch(win_);
        switch (ch) {
            case 'e': select_exercise(); break;
            case 'a': add_set(); break;
            case 'w': {
                auto sets = repo_.get_sets_for_workout(workout_id_);
                if (!sets.empty()) {
                    werase(win_);
                    box(win_, 0, 0);
                    mvwprintw(win_, 1, 2, "Enter set # to edit weight:");
                    wrefresh(win_);
                    auto pick = get_int_input(win_, 2, 2, "Set #: ");
                    if (pick && *pick >= 1 && *pick <= static_cast<int>(sets.size())) {
                        edit_set_weight(sets[*pick - 1].id);
                    }
                }
                break;
            }
            case 'f':
                repo_.finish_workout(workout_id_);
                running = false;
                break;
            case 27: case 'q':
                running = false;
                break;
        }
    }
}

void WorkoutView::select_exercise() {
    werase(win_);
    box(win_, 0, 0);
    mvwprintw(win_, 1, 2, "Enter exercise name:");
    wrefresh(win_);

    std::string name = get_string_input(win_, 2, 2, "> ");
    if (name.empty()) return;

    auto ex = repo_.find_exercise_by_name(name);
    if (!ex) {
        sf::Exercise new_ex;
        new_ex.name = name;
        new_ex.id = repo_.add_exercise(new_ex);
        ex = new_ex;
    }

    current_exercise_id_ = ex->id;
    current_exercise_name_ = ex->name;
    set_count_ = 0;
}

void WorkoutView::add_set() {
    if (current_exercise_id_ == 0) {
        select_exercise();
        if (current_exercise_id_ == 0) return;
    }

    werase(win_);
    box(win_, 0, 0);
    mvwprintw(win_, 1, 2, "Set %d for %s", set_count_ + 1, current_exercise_name_.c_str());

    auto reps = get_int_input(win_, 3, 2, "Reps: ");
    auto weight = get_double_input(win_, 4, 2, "Weight: ");
    auto rpe = get_double_input(win_, 5, 2, "RPE (1-10, blank to skip): ");

    sf::WorkoutSet s;
    s.workout_id = workout_id_;
    s.exercise_id = current_exercise_id_;
    s.set_order = ++set_count_;
    s.reps = reps;
    s.weight = weight;
    s.rpe = rpe;

    repo_.add_set(s);

    if (weight && reps && *reps > 0 && *weight > 0) {
        double e1rm = rpe ? sf::estimate_1rm_rpe(*weight, *reps, *rpe) : sf::estimate_1rm(*weight, *reps);
        mvwprintw(win_, 7, 2, "e1RM: %.1f", e1rm);
        mvwprintw(win_, 8, 2, "Press any key...");
        wrefresh(win_);
        wgetch(win_);
    }
}

void WorkoutView::edit_set_weight(int64_t set_id) {
    werase(win_);
    box(win_, 0, 0);
    mvwprintw(win_, 1, 2, "Edit Set Weight");

    auto weight = get_double_input(win_, 3, 2, "Weight: ");
    auto rpe = get_double_input(win_, 4, 2, "RPE (blank to skip): ");

    auto sets = repo_.get_sets_for_workout(workout_id_);
    for (auto& s : sets) {
        if (s.id == set_id) {
            if (weight) s.weight = weight;
            if (rpe) s.rpe = rpe;
            repo_.update_set(s);
            break;
        }
    }
}

void WorkoutView::show_sets() {
    auto sets = repo_.get_sets_for_workout(workout_id_);
    if (sets.empty()) return;

    int row = 4;
    mvwprintw(win_, row++, 2, "%-4s %-20s %5s %7s %4s %7s", "#", "Exercise", "Reps", "Weight", "RPE", "e1RM");

    for (const auto& s : sets) {
        auto ex = repo_.get_exercise(s.exercise_id);
        std::string ex_name = ex ? ex->name : "?";
        if (ex_name.size() > 20) ex_name.resize(20);

        std::string reps_s = s.reps ? std::to_string(*s.reps) : "-";
        std::string wt_s = s.weight ? std::format("{:.1f}", *s.weight) : "---";
        std::string rpe_s = s.rpe ? std::format("{:.0f}", *s.rpe) : "-";
        std::string e1rm_s = "-";
        if (s.weight && s.reps && *s.reps > 0 && *s.weight > 0) {
            double e1rm = s.rpe ? sf::estimate_1rm_rpe(*s.weight, *s.reps, *s.rpe) : sf::estimate_1rm(*s.weight, *s.reps);
            e1rm_s = std::format("{:.1f}", e1rm);
        }

        mvwprintw(win_, row++, 2, "%-4d %-20s %5s %7s %4s %7s",
                  s.set_order, ex_name.c_str(), reps_s.c_str(), wt_s.c_str(), rpe_s.c_str(), e1rm_s.c_str());

        if (row >= getmaxy(win_) - 3) break;
    }
}

void WorkoutView::show_previous_performance(int64_t exercise_id) {
    auto prog = repo_.get_progression(exercise_id, 1);
    if (prog.empty()) return;

    int max_x = getmaxx(win_);
    mvwprintw(win_, 2, max_x - 35, "Last: e1RM=%.1f vol=%.0f", prog[0].estimated_1rm, prog[0].session_volume);
}

} // namespace tui
