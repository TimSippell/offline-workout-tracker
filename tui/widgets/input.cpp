#include "input.h"
#include <cstdlib>
#include <algorithm>

namespace tui {

std::string get_string_input(WINDOW* win, int y, int x, const std::string& prompt, int max_len) {
    mvwprintw(win, y, x, "%s", prompt.c_str());
    wrefresh(win);

    curs_set(1);
    keypad(win, TRUE);

    std::string buf;
    int cursor = 0;
    int px = x + static_cast<int>(prompt.size());
    int limit = std::min(max_len, 255);

    while (true) {
        mvwprintw(win, y, px, "%-*s", limit, buf.c_str());
        wmove(win, y, px + cursor);
        wrefresh(win);

        int ch = wgetch(win);
        if (ch == 27) {
            curs_set(0);
            return "";
        }
        if (ch == '\n' || ch == KEY_ENTER) {
            break;
        }
        if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (cursor > 0) {
                buf.erase(cursor - 1, 1);
                cursor--;
            }
        } else if (ch == KEY_LEFT) {
            if (cursor > 0) cursor--;
        } else if (ch == KEY_RIGHT) {
            if (cursor < static_cast<int>(buf.size())) cursor++;
        } else if (ch == KEY_HOME || ch == 1) {
            cursor = 0;
        } else if (ch == KEY_END || ch == 5) {
            cursor = static_cast<int>(buf.size());
        } else if (ch >= 32 && ch < 127 && static_cast<int>(buf.size()) < limit) {
            buf.insert(buf.begin() + cursor, static_cast<char>(ch));
            cursor++;
        }
    }

    curs_set(0);
    return buf;
}

std::optional<int> get_int_input(WINDOW* win, int y, int x, const std::string& prompt) {
    std::string s = get_string_input(win, y, x, prompt, 10);
    if (s.empty()) return std::nullopt;
    char* end = nullptr;
    long val = std::strtol(s.c_str(), &end, 10);
    if (end == s.c_str()) return std::nullopt;
    return static_cast<int>(val);
}

std::optional<double> get_double_input(WINDOW* win, int y, int x, const std::string& prompt) {
    std::string s = get_string_input(win, y, x, prompt, 10);
    if (s.empty()) return std::nullopt;
    char* end = nullptr;
    double val = std::strtod(s.c_str(), &end);
    if (end == s.c_str()) return std::nullopt;
    return val;
}

} // namespace tui
