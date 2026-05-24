#include "app.h"
#include "widgets/menu.h"
#include "widgets/input.h"
#include "widgets/table.h"
#include "views/workout_view.h"
#include "views/history_view.h"
#include "views/exercise_view.h"
#include "views/progress_view.h"
#include "views/template_view.h"
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

void App::run() {
    init_ncurses();

    while (current_screen_ != Screen::Quit) {
        clear();

        switch (current_screen_) {
            case Screen::MainMenu: screen_main_menu(); break;
            case Screen::Workout:  screen_workout(); break;
            case Screen::History:  screen_history(); break;
            case Screen::Exercises: screen_exercises(); break;
            case Screen::Templates: screen_templates(); break;
            case Screen::Progress: screen_progress(); break;
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

} // namespace tui
