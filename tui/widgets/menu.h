#pragma once

#include <ncurses.h>
#include <string>
#include <vector>
#include <functional>

namespace tui {

struct MenuItem {
    std::string label;
    std::function<void()> action;
};

class Menu {
public:
    Menu(WINDOW* win, std::vector<MenuItem> items);

    void draw();
    void handle_input(int ch);
    int selected() const { return selected_; }

private:
    WINDOW* win_;
    std::vector<MenuItem> items_;
    int selected_ = 0;
};

} // namespace tui
