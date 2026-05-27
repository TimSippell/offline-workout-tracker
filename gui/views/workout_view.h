#pragma once

#include <swt/swt.h>
#include "template_view.h"
#include <vector>
#include <string>
#include <functional>

namespace gui {

class WorkoutView {
public:
    explicit WorkoutView(sf::Repository& repo);
    void render();
    void refresh_templates() { template_view_.refresh(); }
    void set_on_finish(std::function<void()> cb) { on_finish_ = std::move(cb); }

private:
    void render_no_workout();
    void render_active_workout();
    void render_exercise_picker();
    void render_add_set_popup();
    void render_edit_weight_popup();
    void render_template_picker();

    sf::Repository& repo_;
    TemplateView template_view_;

    int64_t workout_id_ = 0;
    int64_t current_exercise_id_ = 0;
    std::string current_exercise_name_;
    int set_count_ = 0;

    std::vector<sf::WorkoutSet> sets_;
    bool needs_refresh_ = true;

    bool show_exercise_picker_ = false;
    std::vector<sf::Exercise> picker_exercises_;
    char picker_filter_[128] = {};

    bool show_add_set_ = false;
    char set_reps_buf_[16] = {};
    char set_weight_buf_[16] = {};
    char set_duration_buf_[16] = {};
    char set_rest_buf_[16] = {};
    char set_rpe_buf_[16] = {};

    bool show_edit_weight_ = false;
    int64_t edit_set_id_ = 0;
    char edit_weight_buf_[16] = {};
    char edit_duration_buf_[16] = {};
    char edit_rest_buf_[16] = {};
    char edit_rpe_buf_[16] = {};

    bool show_template_picker_ = false;
    std::vector<sf::WorkoutTemplate> picker_templates_;

    char workout_name_buf_[128] = {};

    std::function<void()> on_finish_;
};

} // namespace gui
