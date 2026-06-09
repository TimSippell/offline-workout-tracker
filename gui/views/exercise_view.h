#pragma once

#include <owt/owt.h>
#include <vector>

namespace gui {

class ExerciseView {
public:
    explicit ExerciseView(sf::Repository& repo);
    void render();
    void refresh() { needs_refresh_ = true; }

private:
    void render_add_popup();
    void render_edit_popup();
    void render_detail_popup();

    sf::Repository& repo_;
    std::vector<sf::Exercise> exercises_;
    bool needs_refresh_ = true;
    int selected_ = -1;

    char filter_buf_[128] = {};

    bool show_add_ = false;
    bool open_add_ = false;
    char add_name_[128] = {};
    char add_category_[128] = {};
    char add_muscle_[128] = {};
    int add_tracking_ = 0;

    bool show_edit_ = false;
    bool open_edit_ = false;
    sf::Exercise edit_exercise_;
    char edit_name_[128] = {};
    char edit_category_[128] = {};
    char edit_muscle_[128] = {};
    int edit_tracking_ = 0;

    bool show_detail_ = false;
    bool open_detail_ = false;
    sf::Exercise detail_exercise_;
    sf::ExerciseStats detail_stats_;
};

} // namespace gui
