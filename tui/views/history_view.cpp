#include "history_view.h"
#include "../widgets/table.h"
#include <format>

namespace tui {

HistoryView::HistoryView(WINDOW* win, sf::Repository& repo)
    : win_(win), repo_(repo) {}

void HistoryView::run() {
    Table table(win_, {"Date", "Name", "Status"}, {20, 25, 10});
    bool running = true;

    while (running) {
        werase(win_);
        box(win_, 0, 0);

        auto workouts = repo_.list_workouts(50);

        if (workouts.empty()) {
            mvwprintw(win_, 2, 2, "No workouts yet.");
            mvwprintw(win_, 4, 2, "Press any key to go back...");
            wrefresh(win_);
            wgetch(win_);
            return;
        }

        int prev_selected = table.selected();
        std::vector<std::vector<std::string>> rows;
        for (const auto& w : workouts) {
            std::string status = w.finished_at.empty() ? "active" : "done";
            rows.push_back({w.started_at, w.name.empty() ? "(unnamed)" : w.name, status});
        }
        table.set_rows(std::move(rows));
        table.set_selected(std::min(prev_selected, static_cast<int>(workouts.size()) - 1));

        mvwprintw(win_, getmaxy(win_) - 2, 2, "j/k:navigate | Enter:detail | q:back");
        table.draw();

        int ch = wgetch(win_);
        switch (ch) {
            case 27: case 'q': running = false; break;
            case '\n': case KEY_ENTER:
                if (!workouts.empty()) {
                    show_workout_detail(workouts[table.selected()].id);
                }
                break;
            default:
                table.handle_input(ch);
                break;
        }
    }
}

void HistoryView::show_workout_detail(int64_t workout_id) {
    auto w = repo_.get_workout(workout_id);
    if (!w) return;

    werase(win_);
    box(win_, 0, 0);

    wattron(win_, A_BOLD);
    mvwprintw(win_, 1, 2, "Workout: %s", w->name.empty() ? "(unnamed)" : w->name.c_str());
    wattroff(win_, A_BOLD);
    mvwprintw(win_, 2, 2, "Started: %s", w->started_at.c_str());
    if (!w->finished_at.empty())
        mvwprintw(win_, 2, 35, "Finished: %s", w->finished_at.c_str());

    int row = 4;
    mvwprintw(win_, row++, 2, "%-4s %-20s %5s %7s %4s %7s", "#", "Exercise", "Reps", "Weight", "RPE", "e1RM");

    for (const auto& s : w->sets) {
        auto ex = repo_.get_exercise(s.exercise_id);
        std::string ex_name = ex ? ex->name : "?";
        if (ex_name.size() > 20) ex_name.resize(20);

        std::string reps_s = s.reps ? std::to_string(*s.reps) : "-";
        std::string wt_s = s.weight ? std::format("{:.1f}", *s.weight) : "-";
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

    double vol = sf::calculate_volume(w->sets);
    mvwprintw(win_, getmaxy(win_) - 3, 2, "Total volume: %.0f", vol);
    mvwprintw(win_, getmaxy(win_) - 2, 2, "Press any key to go back...");
    wrefresh(win_);
    wgetch(win_);
}

} // namespace tui
