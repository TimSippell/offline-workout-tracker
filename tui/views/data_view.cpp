#include "data_view.h"
#include "../widgets/menu.h"
#include "../widgets/input.h"
#include <swt/data_io.h>
#include <fstream>
#include <filesystem>

namespace tui {

DataView::DataView(WINDOW* win, sf::Repository& repo)
    : win_(win), repo_(repo) {}

void DataView::run() {
    bool running = true;

    Menu menu(win_, {
        {"  Export to JSON file", [&]{ do_export(); }},
        {"  Import from JSON file", [&]{ do_import(); }},
        {"  Back", [&]{ running = false; }},
    });

    while (running) {
        werase(win_);
        box(win_, 0, 0);
        mvwprintw(win_, getmaxy(win_) - 2, 2, "j/k:navigate | Enter:select | q:back");
        menu.draw();
        int ch = wgetch(win_);
        if (ch == 'q' || ch == 27) { running = false; break; }
        menu.handle_input(ch);
    }
}

std::optional<sf::ExportScope> DataView::pick_scope(const std::string& action) {
    std::vector<std::pair<std::string, sf::ExportScope>> options = {
        {"  Workout history only",     sf::ExportScope::History},
        {"  Exercises + workouts",     sf::ExportScope::ExercisesAndWorkouts},
        {"  Everything (+ templates)", sf::ExportScope::All},
    };
    int selected = 2;
    int n = static_cast<int>(options.size());

    while (true) {
        werase(win_);
        box(win_, 0, 0);
        wattron(win_, A_BOLD);
        mvwprintw(win_, 2, 2, "What to %s?", action.c_str());
        wattroff(win_, A_BOLD);

        for (int i = 0; i < n; i++) {
            if (i == selected) wattron(win_, A_REVERSE);
            mvwprintw(win_, 4 + i, 4, "%-*s", getmaxx(win_) - 8, options[i].first.c_str());
            if (i == selected) wattroff(win_, A_REVERSE);
        }

        mvwprintw(win_, getmaxy(win_) - 2, 2, "j/k:navigate | Enter:select | Esc:cancel");
        wrefresh(win_);

        int ch = wgetch(win_);
        switch (ch) {
            case KEY_UP: case 'k':
                selected = (selected - 1 + n) % n; break;
            case KEY_DOWN: case 'j':
                selected = (selected + 1) % n; break;
            case '\n': case KEY_ENTER:
                return options[selected].second;
            case 27: case 'q':
                return std::nullopt;
        }
    }
}

bool DataView::is_valid_path(const std::string& path) {
    if (path.empty()) return false;
    for (char c : path) {
        if (c < 32 || c == 127) return false;
    }
    if (path[0] != '/' && path[0] != '~' && path[0] != '.') return false;
    return true;
}

std::string DataView::expand_path(const std::string& raw) {
    std::string path = raw;
    if (!path.empty() && path[0] == '~') {
#ifdef _WIN32
        const char* home = getenv("USERPROFILE");
#else
        const char* home = getenv("HOME");
#endif
        if (home) path = std::string(home) + path.substr(1);
    }
    return path;
}

void DataView::do_export() {
    auto scope = pick_scope("export");
    if (!scope) return;

    werase(win_);
    box(win_, 0, 0);

    std::string path = get_string_input(win_, 2, 2, "Export path: ", 200);
    if (path.empty()) return;
    path = expand_path(path);

    if (!is_valid_path(path)) {
        werase(win_);
        box(win_, 0, 0);
        wattron(win_, COLOR_PAIR(4));
        mvwprintw(win_, 2, 2, "Error: invalid path");
        wattroff(win_, COLOR_PAIR(4));
        mvwprintw(win_, 4, 2, "Press any key...");
        wrefresh(win_);
        wgetch(win_);
        return;
    }

    if (std::filesystem::is_directory(path)) {
        if (path.back() != '/' && path.back() != '\\')
            path += '/';
        path += "workouts.json";
    }

    std::ofstream out(path);
    if (!out) {
        werase(win_);
        box(win_, 0, 0);
        wattron(win_, COLOR_PAIR(4));
        mvwprintw(win_, 2, 2, "Error: could not write to %s", path.c_str());
        wattroff(win_, COLOR_PAIR(4));
        mvwprintw(win_, 4, 2, "Press any key...");
        wrefresh(win_);
        wgetch(win_);
        return;
    }

    sf::export_to_json(repo_, out, *scope);
    out.close();

    werase(win_);
    box(win_, 0, 0);
    wattron(win_, COLOR_PAIR(2));
    mvwprintw(win_, 2, 2, "Exported to %s", path.c_str());
    wattroff(win_, COLOR_PAIR(2));
    mvwprintw(win_, 4, 2, "Press any key...");
    wrefresh(win_);
    wgetch(win_);
}

void DataView::do_import() {
    werase(win_);
    box(win_, 0, 0);

    std::string path = get_string_input(win_, 2, 2, "Import path: ", 200);
    if (path.empty()) return;
    path = expand_path(path);

    if (!is_valid_path(path)) {
        werase(win_);
        box(win_, 0, 0);
        wattron(win_, COLOR_PAIR(4));
        mvwprintw(win_, 2, 2, "Error: invalid path");
        wattroff(win_, COLOR_PAIR(4));
        mvwprintw(win_, 4, 2, "Press any key...");
        wrefresh(win_);
        wgetch(win_);
        return;
    }

    std::ifstream in(path);
    if (!in) {
        werase(win_);
        box(win_, 0, 0);
        wattron(win_, COLOR_PAIR(4));
        mvwprintw(win_, 2, 2, "Error: could not read %s", path.c_str());
        wattroff(win_, COLOR_PAIR(4));
        mvwprintw(win_, 4, 2, "Press any key...");
        wrefresh(win_);
        wgetch(win_);
        return;
    }

    std::string json((std::istreambuf_iterator<char>(in)),
                      std::istreambuf_iterator<char>());
    in.close();

    sf::ImportSummary summary;
    try {
        summary = sf::preview_import(repo_, json);
    } catch (...) {
        werase(win_);
        box(win_, 0, 0);
        wattron(win_, COLOR_PAIR(4));
        mvwprintw(win_, 2, 2, "Error: invalid JSON file");
        wattroff(win_, COLOR_PAIR(4));
        mvwprintw(win_, 4, 2, "Press any key...");
        wrefresh(win_);
        wgetch(win_);
        return;
    }

    werase(win_);
    box(win_, 0, 0);
    wattron(win_, A_BOLD);
    mvwprintw(win_, 1, 2, "Import Preview");
    wattroff(win_, A_BOLD);

    int row = 3;
    if (summary.new_exercises || summary.existing_exercises) {
        mvwprintw(win_, row++, 2, "New exercises:      %d", summary.new_exercises);
        mvwprintw(win_, row++, 2, "Existing exercises:  %d (will be merged)", summary.existing_exercises);
    }
    if (summary.workouts) {
        mvwprintw(win_, row++, 2, "Workouts:           %d (%d sets)", summary.workouts, summary.workout_sets);
    }
    if (summary.templates) {
        mvwprintw(win_, row++, 2, "Templates:          %d (%d sets)", summary.templates, summary.template_sets);
    }

    row += 1;
    int confirm_sel = 0;
    bool done = false;
    bool confirmed = false;
    const char* confirm_opts[] = {"  Yes, import", "  Cancel"};

    while (!done) {
        for (int i = 0; i < 2; i++) {
            if (i == confirm_sel) wattron(win_, A_REVERSE);
            mvwprintw(win_, row + i, 4, "%-*s", getmaxx(win_) - 8, confirm_opts[i]);
            if (i == confirm_sel) wattroff(win_, A_REVERSE);
        }
        wrefresh(win_);

        int ch = wgetch(win_);
        switch (ch) {
            case KEY_UP: case 'k': confirm_sel = 0; break;
            case KEY_DOWN: case 'j': confirm_sel = 1; break;
            case '\n': case KEY_ENTER:
                confirmed = (confirm_sel == 0); done = true; break;
            case 27: case 'q': done = true; break;
        }
    }

    if (!confirmed) return;

    sf::ImportResult result;
    try {
        result = sf::import_from_json(repo_, json);
    } catch (...) {
        werase(win_);
        box(win_, 0, 0);
        wattron(win_, COLOR_PAIR(4));
        mvwprintw(win_, 2, 2, "Error: import failed");
        wattroff(win_, COLOR_PAIR(4));
        mvwprintw(win_, 4, 2, "Press any key...");
        wrefresh(win_);
        wgetch(win_);
        return;
    }

    werase(win_);
    box(win_, 0, 0);
    wattron(win_, COLOR_PAIR(2));
    mvwprintw(win_, 2, 2, "Import complete!");
    wattroff(win_, COLOR_PAIR(2));
    mvwprintw(win_, 4, 2, "Workouts: %d  Templates: %d  Sets: %d",
              result.workouts, result.templates, result.sets);
    mvwprintw(win_, 6, 2, "Press any key...");
    wrefresh(win_);
    wgetch(win_);
}

} // namespace tui
