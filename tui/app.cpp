#include "app.h"
#include "widgets/menu.h"
#include "widgets/input.h"
#include "widgets/table.h"
#include "views/workout_view.h"
#include "views/history_view.h"
#include "views/exercise_view.h"
#include "views/progress_view.h"
#include "views/template_view.h"
#include "views/data_view.h"
#include <swt/defaults.h>
#include <format>

namespace tui {

App::App(sf::Database& db) : repo_(db) {}

App::~App() {
    shutdown_ncurses();
}

void App::init_ncurses() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_CYAN, -1);
        init_pair(2, COLOR_GREEN, -1);
        init_pair(3, COLOR_YELLOW, -1);
        init_pair(4, COLOR_RED, -1);
    }
}

void App::shutdown_ncurses() {
    if (main_win_) {
        delwin(main_win_);
        main_win_ = nullptr;
    }
    endwin();
}

void App::draw_header(const std::string& title) {
    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(0, 0, " SIMPLE-WORKOUT-TRACKER ");
    attroff(COLOR_PAIR(1) | A_BOLD);
    mvprintw(0, 25, "| %s", title.c_str());

    int max_x = getmaxx(stdscr);
    mvhline(1, 0, ACS_HLINE, max_x);
    refresh();
}

void App::draw_footer(const std::string& hint) {
    int max_y = getmaxy(stdscr);
    mvhline(max_y - 2, 0, ACS_HLINE, getmaxx(stdscr));
    mvprintw(max_y - 1, 1, "%s", hint.c_str());
    refresh();
}

void App::prompt_seed() {
    draw_header("Welcome");
    draw_footer("j/k: navigate | Enter: select");

    int max_y = getmaxy(stdscr);
    int max_x = getmaxx(stdscr);

    if (main_win_) delwin(main_win_);
    main_win_ = newwin(max_y - 3, max_x, 2, 0);
    keypad(main_win_, TRUE);

    mvwprintw(main_win_, 1, 2, "No exercises or templates found.");
    mvwprintw(main_win_, 2, 2, "Load default exercises and workout templates?");
    wrefresh(main_win_);

    WINDOW* menu_win = derwin(main_win_, max_y - 7, max_x, 4, 0);
    keypad(menu_win, TRUE);

    bool done = false;
    bool seed = false;

    Menu menu(menu_win, {
        {"  Yes, load defaults", [&]{ seed = true; done = true; }},
        {"  No, start empty",    [&]{ done = true; }},
    });

    while (!done) {
        werase(menu_win);
        menu.draw();
        int ch = wgetch(menu_win);
        menu.handle_input(ch);
    }

    delwin(menu_win);

    if (seed) {
        sf::seed_default_exercises(repo_);
        sf::seed_default_templates(repo_);
    }
    repo_.set_setup_complete(true);
}

void App::run() {
    init_ncurses();

    if (!repo_.is_setup_complete()) {
        auto exercises = repo_.list_exercises();
        auto templates = repo_.list_templates();
        if (exercises.empty() && templates.empty()) {
            prompt_seed();
        } else {
            repo_.set_setup_complete(true);
        }
    }

    while (current_screen_ != Screen::Quit) {
        clear();

        switch (current_screen_) {
            case Screen::MainMenu: screen_main_menu(); break;
            case Screen::Workout:  screen_workout(); break;
            case Screen::History:  screen_history(); break;
            case Screen::Exercises: screen_exercises(); break;
            case Screen::Templates: screen_templates(); break;
            case Screen::Progress: screen_progress(); break;
            case Screen::Data: screen_data(); break;
            case Screen::Quit: break;
        }
    }
}

void App::screen_main_menu() {
    draw_header("Home");
    draw_footer("j/k: navigate | Enter: select | q: quit");

    int max_y = getmaxy(stdscr);
    int max_x = getmaxx(stdscr);

    if (main_win_) delwin(main_win_);
    main_win_ = newwin(max_y - 3, max_x, 2, 0);
    keypad(main_win_, TRUE);

    Screen next = Screen::MainMenu;

    Menu menu(main_win_, {
        {"  New Workout", [&]{ next = Screen::Workout; }},
        {"  Templates",   [&]{ next = Screen::Templates; }},
        {"  History",     [&]{ next = Screen::History; }},
        {"  Exercises",   [&]{ next = Screen::Exercises; }},
        {"  Progress",    [&]{ next = Screen::Progress; }},
        {"  Import/Export", [&]{ next = Screen::Data; }},
        {"  Quit",        [&]{ next = Screen::Quit; }},
    });

    while (next == Screen::MainMenu) {
        werase(main_win_);
        box(main_win_, 0, 0);
        menu.draw();

        int ch = wgetch(main_win_);
        if (ch == 'q') { next = Screen::Quit; break; }
        menu.handle_input(ch);
    }

    current_screen_ = next;
}

void App::screen_workout() {
    WorkoutView view(main_win_, repo_);
    draw_header("Workout");

    int max_y = getmaxy(stdscr);
    int max_x = getmaxx(stdscr);
    if (main_win_) delwin(main_win_);
    main_win_ = newwin(max_y - 3, max_x, 2, 0);
    keypad(main_win_, TRUE);

    view.set_window(main_win_);
    view.run();
    current_screen_ = Screen::MainMenu;
}

void App::screen_history() {
    draw_header("History");

    int max_y = getmaxy(stdscr);
    int max_x = getmaxx(stdscr);
    if (main_win_) delwin(main_win_);
    main_win_ = newwin(max_y - 3, max_x, 2, 0);
    keypad(main_win_, TRUE);

    HistoryView view(main_win_, repo_);
    view.run();
    current_screen_ = Screen::MainMenu;
}

void App::screen_exercises() {
    draw_header("Exercises");

    int max_y = getmaxy(stdscr);
    int max_x = getmaxx(stdscr);
    if (main_win_) delwin(main_win_);
    main_win_ = newwin(max_y - 3, max_x, 2, 0);
    keypad(main_win_, TRUE);

    ExerciseView view(main_win_, repo_);
    view.run();
    current_screen_ = Screen::MainMenu;
}

void App::screen_templates() {
    draw_header("Templates");

    int max_y = getmaxy(stdscr);
    int max_x = getmaxx(stdscr);
    if (main_win_) delwin(main_win_);
    main_win_ = newwin(max_y - 3, max_x, 2, 0);
    keypad(main_win_, TRUE);

    TemplateView view(main_win_, repo_);
    view.run();
    current_screen_ = Screen::MainMenu;
}

void App::screen_progress() {
    draw_header("Progress");

    int max_y = getmaxy(stdscr);
    int max_x = getmaxx(stdscr);
    if (main_win_) delwin(main_win_);
    main_win_ = newwin(max_y - 3, max_x, 2, 0);
    keypad(main_win_, TRUE);

    ProgressView view(main_win_, repo_);
    view.run();
    current_screen_ = Screen::MainMenu;
}

void App::screen_data() {
    draw_header("Import/Export");

    int max_y = getmaxy(stdscr);
    int max_x = getmaxx(stdscr);
    if (main_win_) delwin(main_win_);
    main_win_ = newwin(max_y - 3, max_x, 2, 0);
    keypad(main_win_, TRUE);

    DataView view(main_win_, repo_);
    view.run();
    current_screen_ = Screen::MainMenu;
}

} // namespace tui
