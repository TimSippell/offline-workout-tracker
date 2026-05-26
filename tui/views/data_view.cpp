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

    while (running) {
        werase(win_);
        box(win_, 0, 0);

        Menu menu(win_, {
            {"  Export to JSON file", [&]{ do_export(); }},
            {"  Import from JSON file", [&]{ do_import(); }},
            {"  Back", [&]{ running = false; }},
        });

        mvwprintw(win_, getmaxy(win_) - 2, 2, "j/k:navigate | Enter:select | q:back");

        while (running) {
            werase(win_);
            box(win_, 0, 0);
            mvwprintw(win_, getmaxy(win_) - 2, 2, "j/k:navigate | Enter:select | q:back");
            menu.draw();
            int ch = wgetch(win_);
            if (ch == 'q') { running = false; break; }
            menu.handle_input(ch);
            break;
        }
    }
}

void DataView::do_export() {
    werase(win_);
    box(win_, 0, 0);

    std::string path = get_string_input(win_, 2, 2, "Export path: ", 200);
    if (path.empty()) return;

    if (path[0] == '~') {
#ifdef _WIN32
        const char* home = getenv("USERPROFILE");
#else
        const char* home = getenv("HOME");
#endif
        if (home) path = std::string(home) + path.substr(1);
    }

    if (std::filesystem::is_directory(path)) {
        if (path.back() != '/' && path.back() != '\\')
            path += '/';
        path += "workouts.json";
    }

    std::string json = sf::export_to_json(repo_);
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

    out << json;
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

    if (path[0] == '~') {
#ifdef _WIN32
        const char* home = getenv("USERPROFILE");
#else
        const char* home = getenv("HOME");
#endif
        if (home) path = std::string(home) + path.substr(1);
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
    mvwprintw(win_, row++, 2, "New exercises:      %d", summary.new_exercises);
    mvwprintw(win_, row++, 2, "Existing exercises:  %d (will be merged)", summary.existing_exercises);
    mvwprintw(win_, row++, 2, "Workouts:           %d (%d sets)", summary.workouts, summary.workout_sets);
    mvwprintw(win_, row++, 2, "Templates:          %d (%d sets)", summary.templates, summary.template_sets);

    row += 2;

    WINDOW* confirm_win = derwin(win_, 4, getmaxx(win_) - 4, row, 2);
    keypad(confirm_win, TRUE);

    bool done = false;
    bool confirmed = false;

    Menu confirm_menu(confirm_win, {
        {"  Yes, import", [&]{ confirmed = true; done = true; }},
        {"  Cancel",      [&]{ done = true; }},
    });

    while (!done) {
        werase(confirm_win);
        confirm_menu.draw();
        int ch = wgetch(confirm_win);
        if (ch == 'q') { done = true; break; }
        confirm_menu.handle_input(ch);
    }

    delwin(confirm_win);

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
