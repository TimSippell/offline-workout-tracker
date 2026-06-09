#include "progress_view.h"
#include "../widgets/table.h"
#include <owt/calculator.h>
#include <format>
#include <algorithm>

namespace tui {

ProgressView::ProgressView(WINDOW* win, sf::Repository& repo)
    : win_(win), repo_(repo) {}

void ProgressView::run() {
    auto exercises = repo_.list_exercises();

    if (exercises.empty()) {
        werase(win_);
        box(win_, 0, 0);
        mvwprintw(win_, 2, 2, "No exercises yet.");
        mvwprintw(win_, 4, 2, "Press any key to go back...");
        wrefresh(win_);
        wgetch(win_);
        return;
    }

    int selected = 0;
    bool running = true;

    while (running) {
        werase(win_);
        box(win_, 0, 0);

        mvwprintw(win_, 1, 2, "Select exercise to view progress:");

        int visible = getmaxy(win_) - 6;
        int scroll = std::max(0, selected - visible + 1);
        for (int i = scroll; i < static_cast<int>(exercises.size()) && (i - scroll) < visible; ++i) {
            if (i == selected) wattron(win_, A_REVERSE);
            mvwprintw(win_, (i - scroll) + 3, 4, "%-30s", exercises[i].name.c_str());
            if (i == selected) wattroff(win_, A_REVERSE);
        }

        mvwprintw(win_, getmaxy(win_) - 2, 2, "j/k:navigate | Enter:view | q:back");
        wrefresh(win_);

        int ch = wgetch(win_);
        switch (ch) {
            case 27: case 'q': running = false; break;
            case 'j': case KEY_DOWN:
                if (selected < static_cast<int>(exercises.size()) - 1) selected++;
                break;
            case 'k': case KEY_UP:
                if (selected > 0) selected--;
                break;
            case '\n': case KEY_ENTER:
                show_exercise_progress(exercises[selected].id, exercises[selected].name);
                break;
        }
    }
}

void ProgressView::show_exercise_progress(int64_t exercise_id, const std::string& name) {
    auto prog = repo_.get_progression(exercise_id, 20);

    werase(win_);
    box(win_, 0, 0);

    wattron(win_, A_BOLD);
    mvwprintw(win_, 1, 2, "Progress: %s", name.c_str());
    wattroff(win_, A_BOLD);

    if (prog.empty()) {
        mvwprintw(win_, 3, 2, "No data yet.");
        mvwprintw(win_, 5, 2, "Press any key...");
        wrefresh(win_);
        wgetch(win_);
        return;
    }

    // Reverse so oldest is first (chronological)
    std::reverse(prog.begin(), prog.end());

    mvwprintw(win_, 3, 2, "%-12s %8s %8s %8s", "Date", "e1RM", "Best Wt", "Volume");

    int row = 4;
    for (const auto& p : prog) {
        mvwprintw(win_, row++, 2, "%-12s %8.1f %8.1f %8.0f",
                  p.date.c_str(), p.estimated_1rm, p.best_set_weight, p.session_volume);
        if (row >= getmaxy(win_) - 4) break;
    }

    // Simple ASCII bar chart for e1RM
    if (prog.size() >= 2) {
        double max_1rm = 0;
        for (const auto& p : prog) if (p.estimated_1rm > max_1rm) max_1rm = p.estimated_1rm;

        row += 1;
        mvwprintw(win_, row++, 2, "e1RM trend:");
        int chart_width = getmaxx(win_) - 6;

        for (const auto& p : prog) {
            if (row >= getmaxy(win_) - 2) break;
            int bar_len = max_1rm > 0 ? static_cast<int>((p.estimated_1rm / max_1rm) * (chart_width - 12)) : 0;
            mvwprintw(win_, row, 2, "%.5s ", p.date.c_str() + 5);

            wattron(win_, COLOR_PAIR(2));
            for (int b = 0; b < bar_len; ++b) waddch(win_, ACS_BLOCK);
            wattroff(win_, COLOR_PAIR(2));

            row++;
        }
    }

    mvwprintw(win_, getmaxy(win_) - 2, 2, "Press any key to go back...");
    wrefresh(win_);
    wgetch(win_);
}

} // namespace tui
