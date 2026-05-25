#pragma once

#include <swt/swt.h>
#include <ncurses.h>

namespace tui {

enum class Screen {
    MainMenu,
    Workout,
    History,
    Exercises,
    Templates,
    Progress,
    Quit,
};

class App {
public:
    App(sf::Database& db);
    ~App();

    void run();

private:
    void init_ncurses();
    void shutdown_ncurses();
    void draw_header(const std::string& title);
    void draw_footer(const std::string& hint);

    void prompt_seed();
    void screen_main_menu();
    void screen_workout();
    void screen_history();
    void screen_exercises();
    void screen_templates();
    void screen_progress();

    sf::Repository repo_;
    Screen current_screen_ = Screen::MainMenu;
    WINDOW* main_win_ = nullptr;
};

} // namespace tui
