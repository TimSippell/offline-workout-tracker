#pragma once

#include <owt/owt.h>
#include <vector>

namespace gui {

class HistoryView {
public:
    explicit HistoryView(sf::Repository& repo);
    void render();
    void refresh() { needs_refresh_ = true; expanded_id_ = -1; expanded_sets_.clear(); }

private:
    void render_edit_set_popup();

    sf::Repository& repo_;
    std::vector<sf::Workout> workouts_;
    bool needs_refresh_ = true;
    int64_t expanded_id_ = -1;
    std::vector<sf::WorkoutSet> expanded_sets_;

    bool show_edit_set_ = false;
    bool open_edit_set_ = false;
    sf::WorkoutSet edit_set_;
    char edit_reps_buf_[16] = {};
    char edit_weight_buf_[16] = {};
    char edit_duration_buf_[16] = {};
    char edit_rest_buf_[16] = {};
    char edit_rpe_buf_[16] = {};
};

} // namespace gui
