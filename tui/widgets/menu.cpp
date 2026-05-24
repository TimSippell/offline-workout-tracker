#include "menu.h"

namespace tui {

Menu::Menu(WINDOW* win, std::vector<MenuItem> items)
    : win_(win), items_(std::move(items)) {}

void Menu::draw() {
    int max_y, max_x;
    getmaxyx(win_, max_y, max_x);
    (void)max_y;

    for (int i = 0; i < static_cast<int>(items_.size()); ++i) {
        if (i == selected_) wattron(win_, A_REVERSE);
        mvwprintw(win_, i + 1, 2, "%-*s", max_x - 4, items_[i].label.c_str());
        if (i == selected_) wattroff(win_, A_REVERSE);
    }
    wrefresh(win_);
}

void Menu::handle_input(int ch) {
    int n = static_cast<int>(items_.size());
    if (n == 0) return;

    switch (ch) {
        case KEY_UP:
        case 'k':
            selected_ = (selected_ - 1 + n) % n;
            break;
        case KEY_DOWN:
        case 'j':
            selected_ = (selected_ + 1) % n;
            break;
        case '\n':
        case KEY_ENTER:
            if (items_[selected_].action) items_[selected_].action();
            break;
    }
}

} // namespace tui
