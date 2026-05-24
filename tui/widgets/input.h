#pragma once

#include <ncurses.h>
#include <string>
#include <optional>

namespace tui {

std::string get_string_input(WINDOW* win, int y, int x, const std::string& prompt, int max_len = 40);
std::optional<int> get_int_input(WINDOW* win, int y, int x, const std::string& prompt);
std::optional<double> get_double_input(WINDOW* win, int y, int x, const std::string& prompt);

} // namespace tui
