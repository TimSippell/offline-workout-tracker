#include "template_view.h"
#include "../widgets/input.h"
#include "../widgets/table.h"
#include <format>

namespace tui {

TemplateView::TemplateView(WINDOW* win, sf::Repository& repo)
    : win_(win), repo_(repo) {}

void TemplateView::run() {
    int selected = 0;
    bool running = true;

    while (running) {
        werase(win_);
        box(win_, 0, 0);

        auto templates = repo_.list_templates();

        if (templates.empty()) {
            mvwprintw(win_, 2, 2, "No templates yet. Press 'a' to create one.");
        } else {
            int max_row = getmaxy(win_) - 3;
            int row = 2;
            int scroll = std::max(0, selected - (max_row - 2));
            for (int i = scroll; i < static_cast<int>(templates.size()) && row < max_row; i++) {
                if (i == selected) wattron(win_, A_REVERSE);
                auto sets = repo_.get_template_sets(templates[i].id);
                mvwprintw(win_, row++, 2, "%-30s  %d sets",
                          templates[i].name.c_str(), static_cast<int>(sets.size()));
                if (i == selected) wattroff(win_, A_REVERSE);
            }
        }

        mvwprintw(win_, getmaxy(win_) - 2, 2, "a:create | Enter:edit | d:delete | q:back");
        wrefresh(win_);

        int ch = wgetch(win_);
        auto templates_now = repo_.list_templates();

        switch (ch) {
            case 'q': running = false; break;
            case 'a': create_template(); break;
            case 'd':
                if (!templates_now.empty() && selected < static_cast<int>(templates_now.size())) {
                    repo_.delete_template(templates_now[selected].id);
                    if (selected > 0) selected--;
                }
                break;
            case '\n': case KEY_ENTER:
                if (!templates_now.empty() && selected < static_cast<int>(templates_now.size())) {
                    edit_template(templates_now[selected].id);
                }
                break;
            case 'j': case KEY_DOWN:
                if (selected < static_cast<int>(templates_now.size()) - 1) selected++;
                break;
            case 'k': case KEY_UP:
                if (selected > 0) selected--;
                break;
        }
    }
}

void TemplateView::create_template() {
    werase(win_);
    box(win_, 0, 0);
    mvwprintw(win_, 1, 2, "Create Template");

    std::string name = get_string_input(win_, 3, 2, "Name: ");
    if (name.empty()) return;

    sf::WorkoutTemplate t;
    t.name = name;
    int64_t id = repo_.create_template(t);

    add_exercise_to_template(id);
}

void TemplateView::edit_template(int64_t template_id) {
    bool editing = true;

    while (editing) {
        werase(win_);
        box(win_, 0, 0);

        auto tmpl = repo_.get_template(template_id);
        if (!tmpl) return;

        wattron(win_, A_BOLD);
        mvwprintw(win_, 1, 2, "Template: %s", tmpl->name.c_str());
        wattroff(win_, A_BOLD);

        auto sets = repo_.get_template_sets(template_id);
        if (sets.empty()) {
            mvwprintw(win_, 3, 2, "No exercises. Press 'a' to add.");
        } else {
            int row = 3;
            mvwprintw(win_, row++, 2, "%-4s %-20s %5s %4s", "#", "Exercise", "Reps", "RPE");
            for (const auto& s : sets) {
                auto ex = repo_.get_exercise(s.exercise_id);
                std::string ex_name = ex ? ex->name : "?";
                if (ex_name.size() > 20) ex_name.resize(20);
                std::string reps_s = s.reps ? std::to_string(*s.reps) : "-";
                std::string rpe_s = s.rpe ? std::format("{:.0f}", *s.rpe) : "-";
                mvwprintw(win_, row++, 2, "%-4d %-20s %5s %4s",
                          s.set_order, ex_name.c_str(), reps_s.c_str(), rpe_s.c_str());
                if (row >= getmaxy(win_) - 3) break;
            }
        }

        mvwprintw(win_, getmaxy(win_) - 2, 2, "a:add exercise | q:done");
        wrefresh(win_);

        int ch = wgetch(win_);
        switch (ch) {
            case 'q': editing = false; break;
            case 'a': add_exercise_to_template(template_id); break;
        }
    }
}

void TemplateView::add_exercise_to_template(int64_t template_id) {
    werase(win_);
    box(win_, 0, 0);
    mvwprintw(win_, 1, 2, "Add Exercise to Template");

    auto exercises = repo_.list_exercises();
    if (exercises.empty()) {
        mvwprintw(win_, 3, 2, "No exercises defined. Create some first.");
        mvwprintw(win_, getmaxy(win_) - 2, 2, "Press any key...");
        wrefresh(win_);
        wgetch(win_);
        return;
    }

    int row = 3;
    for (int i = 0; i < static_cast<int>(exercises.size()) && row < getmaxy(win_) - 6; i++) {
        mvwprintw(win_, row++, 2, "%d) %s", i + 1, exercises[i].name.c_str());
    }

    mvwprintw(win_, row + 1, 2, "Pick exercise #:");
    wrefresh(win_);

    auto pick = get_int_input(win_, row + 1, 19, "");
    if (!pick || *pick < 1 || *pick > static_cast<int>(exercises.size())) return;

    auto& ex = exercises[*pick - 1];

    auto num_sets = get_int_input(win_, row + 2, 2, "Number of sets: ");
    if (!num_sets || *num_sets <= 0) return;

    auto reps = get_int_input(win_, row + 3, 2, "Reps per set: ");
    auto rpe = get_double_input(win_, row + 4, 2, "Target RPE (blank to skip): ");

    auto existing = repo_.get_template_sets(template_id);
    int order = static_cast<int>(existing.size());

    for (int i = 0; i < *num_sets; i++) {
        sf::TemplateSet ts;
        ts.template_id = template_id;
        ts.exercise_id = ex.id;
        ts.set_order = ++order;
        ts.reps = reps;
        ts.rpe = rpe;
        repo_.add_template_set(ts);
    }
}

} // namespace tui
