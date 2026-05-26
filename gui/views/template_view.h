#pragma once

#include <swt/swt.h>
#include <vector>

namespace gui {

class TemplateView {
public:
    explicit TemplateView(sf::Repository& repo);
    void render();
    void refresh() { needs_refresh_ = true; }

private:
    void render_edit_popup();
    void render_add_exercise_popup();

    sf::Repository& repo_;
    std::vector<sf::WorkoutTemplate> templates_;
    bool needs_refresh_ = true;

    bool show_edit_ = false;
    int64_t edit_template_id_ = 0;
    std::string edit_template_name_;
    std::vector<sf::TemplateSet> edit_sets_;
    int edit_selected_ = -1;

    bool show_add_exercise_ = false;
    std::vector<sf::Exercise> add_exercises_;
    int add_exercise_pick_ = 0;
    char add_num_sets_buf_[8] = {};
    char add_reps_buf_[8] = {};
    char add_rpe_buf_[8] = {};

    char create_name_buf_[128] = {};
};

} // namespace gui
