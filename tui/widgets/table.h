#pragma once

#include <ncurses.h>
#include <string>
#include <vector>

namespace tui {

class Table {
public:
    Table(WINDOW* win, std::vector<std::string> headers, std::vector<int> col_widths);

    void set_rows(std::vector<std::vector<std::string>> rows);
    void draw();
    void handle_input(int ch);
    int selected() const { return selected_; }
    bool empty() const { return rows_.empty(); }

private:
    WINDOW* win_;
    std::vector<std::string> headers_;
    std::vector<int> col_widths_;
    std::vector<std::vector<std::string>> rows_;
    int selected_ = 0;
    int scroll_offset_ = 0;
};

} // namespace tui
