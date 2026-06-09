#pragma once

#include <ncurses.h>
#include <owt/owt.h>

namespace tui {

class TemplateView {
public:
    TemplateView(WINDOW* win, sf::Repository& repo);
    void run();

private:
    void create_template();
    void edit_template(int64_t template_id);
    void add_exercise_to_template(int64_t template_id);

    WINDOW* win_;
    sf::Repository& repo_;
};

} // namespace tui
