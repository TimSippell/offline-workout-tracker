#pragma once

#include <swt/swt.h>
#include <vector>

namespace gui {

class HistoryView {
public:
    explicit HistoryView(sf::Repository& repo);
    void render();

private:
    void render_detail();

    sf::Repository& repo_;
    std::vector<sf::Workout> workouts_;
    bool needs_refresh_ = true;

    int selected_ = -1;
    bool show_detail_ = false;
    sf::Workout detail_workout_;
};

} // namespace gui
