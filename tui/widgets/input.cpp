#include "input.h"
#include <cstdlib>

namespace tui {

std::string get_string_input(WINDOW* win, int y, int x, const std::string& prompt, int max_len) {
    mvwprintw(win, y, x, "%s", prompt.c_str());
    wrefresh(win);

    echo();
    curs_set(1);

    char buf[256] = {};
    int len = std::min(max_len, 255);
    mvwgetnstr(win, y, x + static_cast<int>(prompt.size()), buf, len);

    noecho();
    curs_set(0);
    return std::string(buf);
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
