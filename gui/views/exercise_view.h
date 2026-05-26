#pragma once

#include <swt/swt.h>
#include <vector>

namespace gui {

class ExerciseView {
public:
    explicit ExerciseView(sf::Repository& repo);
    void render();
    void refresh() { needs_refresh_ = true; }

private:
    void render_add_popup();
    void render_detail_popup();

    sf::Repository& repo_;
    std::vector<sf::Exercise> exercises_;
    bool needs_refresh_ = true;
    int selected_ = -1;

    char filter_buf_[128] = {};

    bool show_add_ = false;
    char add_name_[128] = {};
    int add_category_ = 0;
    int add_muscle_ = 0;

    bool show_detail_ = false;
    sf::Exercise detail_exercise_;
    sf::ExerciseStats detail_stats_;
};

} // namespace gui
