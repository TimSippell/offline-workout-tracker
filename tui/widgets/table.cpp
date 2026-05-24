#include "table.h"
#include <algorithm>

namespace tui {

Table::Table(WINDOW* win, std::vector<std::string> headers, std::vector<int> col_widths)
    : win_(win), headers_(std::move(headers)), col_widths_(std::move(col_widths)) {}

void Table::set_rows(std::vector<std::vector<std::string>> rows) {
    rows_ = std::move(rows);
    selected_ = 0;
    scroll_offset_ = 0;
}

void Table::draw() {
    int max_y, max_x;
    getmaxyx(win_, max_y, max_x);
    (void)max_x;

    int x = 1;
    wattron(win_, A_BOLD);
    for (size_t c = 0; c < headers_.size(); ++c) {
        mvwprintw(win_, 1, x, "%-*s", col_widths_[c], headers_[c].c_str());
        x += col_widths_[c] + 1;
    }
    wattroff(win_, A_BOLD);

    int visible = max_y - 3;
    for (int i = 0; i < visible && (i + scroll_offset_) < static_cast<int>(rows_.size()); ++i) {
        int row_idx = i + scroll_offset_;
        if (row_idx == selected_) wattron(win_, A_REVERSE);

        x = 1;
        for (size_t c = 0; c < headers_.size() && c < rows_[row_idx].size(); ++c) {
            mvwprintw(win_, i + 2, x, "%-*s", col_widths_[c], rows_[row_idx][c].c_str());
            x += col_widths_[c] + 1;
        }

        if (row_idx == selected_) wattroff(win_, A_REVERSE);
    }
    wrefresh(win_);
}

void Table::handle_input(int ch) {
    int n = static_cast<int>(rows_.size());
    if (n == 0) return;

    int max_y, max_x;
    getmaxyx(win_, max_y, max_x);
    (void)max_x;
    int visible = max_y - 3;

    switch (ch) {
        case KEY_UP:
        case 'k':
            if (selected_ > 0) {
                selected_--;
                if (selected_ < scroll_offset_) scroll_offset_ = selected_;
            }
            break;
        case KEY_DOWN:
        case 'j':
            if (selected_ < n - 1) {
                selected_++;
                if (selected_ >= scroll_offset_ + visible)
                    scroll_offset_ = selected_ - visible + 1;
            }
            break;
    }
}

} // namespace tui
