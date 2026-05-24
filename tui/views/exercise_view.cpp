#include "exercise_view.h"
#include "../widgets/input.h"
#include "../widgets/table.h"
#include <swt/calculator.h>
#include <format>

namespace tui {

ExerciseView::ExerciseView(WINDOW* win, sf::Repository& repo)
    : win_(win), repo_(repo) {}

void ExerciseView::run() {
    int selected = 0;
    bool running = true;

    while (running) {
        werase(win_);
        box(win_, 0, 0);

        auto exercises = repo_.list_exercises();

        if (exercises.empty()) {
            mvwprintw(win_, 2, 2, "No exercises yet. Press 'a' to add one.");
        } else {
            Table table(win_, {"Name", "Category", "Muscle Group"}, {25, 15, 15});
            std::vector<std::vector<std::string>> rows;
            for (const auto& ex : exercises) {
                rows.push_back({ex.name, ex.category.empty() ? "-" : ex.category, ex.muscle_group.empty() ? "-" : ex.muscle_group});
            }
            table.set_rows(std::move(rows));
            table.draw();
        }

        mvwprintw(win_, getmaxy(win_) - 2, 2, "a:add | Enter:detail | d:delete | q:back");
        wrefresh(win_);

        int ch = wgetch(win_);
        auto exercises_now = repo_.list_exercises();

        switch (ch) {
            case 'q': running = false; break;
            case 'a': add_exercise(); break;
            case 'd':
                if (!exercises_now.empty() && selected < static_cast<int>(exercises_now.size())) {
                    repo_.delete_exercise(exercises_now[selected].id);
                }
                break;
            case '\n': case KEY_ENTER:
                if (!exercises_now.empty() && selected < static_cast<int>(exercises_now.size())) {
                    show_exercise_detail(exercises_now[selected].id);
                }
                break;
            case 'j': case KEY_DOWN:
                if (selected < static_cast<int>(exercises_now.size()) - 1) selected++;
                break;
            case 'k': case KEY_UP:
                if (selected > 0) selected--;
                break;
        }
    }
}

void ExerciseView::add_exercise() {
    werase(win_);
    box(win_, 0, 0);
    mvwprintw(win_, 1, 2, "Add Exercise");

    std::string name = get_string_input(win_, 3, 2, "Name: ");
    if (name.empty()) return;

    std::string category = get_string_input(win_, 4, 2, "Category (compound/isolation/cardio/bodyweight): ");
    std::string muscle = get_string_input(win_, 5, 2, "Muscle group (chest/back/legs/shoulders/arms/core): ");

    sf::Exercise ex;
    ex.name = name;
    ex.category = category;
    ex.muscle_group = muscle;
    repo_.add_exercise(ex);
}

void ExerciseView::show_exercise_detail(int64_t exercise_id) {
    auto ex = repo_.get_exercise(exercise_id);
    if (!ex) return;

    auto sets = repo_.get_sets_for_exercise(exercise_id);
    auto stats = sf::compute_stats(exercise_id, sets);

    werase(win_);
    box(win_, 0, 0);

    wattron(win_, A_BOLD);
    mvwprintw(win_, 1, 2, "%s", ex->name.c_str());
    wattroff(win_, A_BOLD);

    mvwprintw(win_, 2, 2, "Category: %s", ex->category.empty() ? "-" : ex->category.c_str());
    mvwprintw(win_, 3, 2, "Muscle: %s", ex->muscle_group.empty() ? "-" : ex->muscle_group.c_str());

    mvwprintw(win_, 5, 2, "Estimated 1RM: %.1f", stats.estimated_1rm);
    mvwprintw(win_, 6, 2, "Best weight: %.1f x %d", stats.best_weight, stats.best_reps_at_best_weight);
    mvwprintw(win_, 7, 2, "Total volume: %.0f", stats.total_volume);

    mvwprintw(win_, getmaxy(win_) - 2, 2, "Press any key to go back...");
    wrefresh(win_);
    wgetch(win_);
}

} // namespace tui
