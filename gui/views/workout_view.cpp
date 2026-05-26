#include "workout_view.h"
#include "imgui.h"
#include <swt/calculator.h>
#include <format>
#include <cstring>
#include <cstdlib>

namespace gui {

WorkoutView::WorkoutView(sf::Repository& repo) : repo_(repo), template_view_(repo) {
    auto active = repo_.get_active_workout();
    if (active) {
        workout_id_ = active->id;
        sets_ = repo_.get_sets_for_workout(workout_id_);
        set_count_ = static_cast<int>(sets_.size());
    }
}

void WorkoutView::render() {
    if (workout_id_ == 0) {
        auto active = repo_.get_active_workout();
        if (active) {
            workout_id_ = active->id;
            needs_refresh_ = true;
        }
    }

    if (needs_refresh_ && workout_id_ != 0) {
        sets_ = repo_.get_sets_for_workout(workout_id_);
        needs_refresh_ = false;
    }

    if (workout_id_ == 0) {
        render_no_workout();
    } else {
        render_active_workout();
    }

    render_exercise_picker();
    render_add_set_popup();
    render_edit_weight_popup();
    render_template_picker();
}

void WorkoutView::render_no_workout() {
    ImGui::Text("Workout");
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("No active workout.");
    ImGui::Spacing();

    if (ImGui::Button("Start Blank Workout", ImVec2(200, 36))) {
        ImGui::OpenPopup("Workout Name");
    }

    ImGui::SameLine();
    if (ImGui::Button("Start from Template", ImVec2(200, 36))) {
        picker_templates_ = repo_.list_templates();
        if (picker_templates_.empty()) {
            // no templates — just start blank
        } else {
            show_template_picker_ = true;
        }
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Workout Name", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Name (optional)", workout_name_buf_, sizeof(workout_name_buf_));
        ImGui::Spacing();

        if (ImGui::Button("Start", ImVec2(120, 0))) {
            workout_id_ = repo_.start_workout(workout_name_buf_);
            std::memset(workout_name_buf_, 0, sizeof(workout_name_buf_));
            needs_refresh_ = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            std::memset(workout_name_buf_, 0, sizeof(workout_name_buf_));
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::SeparatorText("Templates");
    ImGui::Spacing();
    template_view_.render();
}

void WorkoutView::render_active_workout() {
    ImGui::Text("Workout #%ld", workout_id_);

    if (!current_exercise_name_.empty()) {
        ImGui::SameLine();
        ImGui::Text("  |  Exercise: %s (Set %d)", current_exercise_name_.c_str(), set_count_ + 1);

        auto prog = repo_.get_progression(current_exercise_id_, 1);
        if (!prog.empty()) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.7f, 1.0f), "  Last: e1RM=%.1f vol=%.0f",
                prog[0].estimated_1rm, prog[0].session_volume);
        }
    }

    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Finish Workout")) {
        repo_.finish_workout(workout_id_);
        workout_id_ = 0;
        current_exercise_id_ = 0;
        current_exercise_name_.clear();
        set_count_ = 0;
        sets_.clear();
        if (on_finish_) on_finish_();
        return;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
        repo_.delete_workout(workout_id_);
        workout_id_ = 0;
        current_exercise_id_ = 0;
        current_exercise_name_.clear();
        set_count_ = 0;
        sets_.clear();
        return;
    }

    ImGui::Spacing();

    if (sets_.empty()) {
        ImGui::Text("No sets yet. Select an exercise and add sets.");
        return;
    }

    if (ImGui::BeginTable("sets", 7,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
            ImVec2(0, ImGui::GetContentRegionAvail().y))) {

        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 40);
        ImGui::TableSetupColumn("Exercise", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Reps", ImGuiTableColumnFlags_WidthFixed, 60);
        ImGui::TableSetupColumn("Weight", ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableSetupColumn("RPE", ImGuiTableColumnFlags_WidthFixed, 50);
        ImGui::TableSetupColumn("e1RM", ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 50);
        ImGui::TableHeadersRow();

        for (int i = 0; i < static_cast<int>(sets_.size()); i++) {
            const auto& s = sets_[i];
            auto ex = repo_.get_exercise(s.exercise_id);
            std::string ex_name = ex ? ex->name : "?";

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%d", s.set_order);

            ImGui::TableNextColumn();
            ImGui::TextUnformatted(ex_name.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%s", s.reps ? std::to_string(*s.reps).c_str() : "-");

            ImGui::TableNextColumn();
            ImGui::Text("%s", s.weight ? std::format("{:.1f}", *s.weight).c_str() : "-");

            ImGui::TableNextColumn();
            ImGui::Text("%s", s.rpe ? std::format("{:.0f}", *s.rpe).c_str() : "-");

            ImGui::TableNextColumn();
            if (s.weight && s.reps && *s.reps > 0 && *s.weight > 0) {
                double e1rm = s.rpe ? sf::estimate_1rm_rpe(*s.weight, *s.reps, *s.rpe)
                                    : sf::estimate_1rm(*s.weight, *s.reps);
                ImGui::Text("%.1f", e1rm);
            } else {
                ImGui::Text("-");
            }

            ImGui::TableNextColumn();
            ImGui::PushID(i);
            if (ImGui::SmallButton("Edit")) {
                edit_set_id_ = s.id;
                show_edit_weight_ = true;
                if (s.weight) snprintf(edit_weight_buf_, sizeof(edit_weight_buf_), "%.1f", *s.weight);
                else std::memset(edit_weight_buf_, 0, sizeof(edit_weight_buf_));
                if (s.rpe) snprintf(edit_rpe_buf_, sizeof(edit_rpe_buf_), "%.0f", *s.rpe);
                else std::memset(edit_rpe_buf_, 0, sizeof(edit_rpe_buf_));
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}

void WorkoutView::render_exercise_picker() {
    if (!show_exercise_picker_) return;

    ImGui::OpenPopup("Pick Exercise");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 500));

    if (ImGui::BeginPopupModal("Pick Exercise", &show_exercise_picker_, ImGuiWindowFlags_NoResize)) {
        if (ImGui::InputText("Search", picker_filter_, sizeof(picker_filter_))) {
            picker_exercises_ = repo_.list_exercises(picker_filter_);
        }

        ImGui::Spacing();

        for (int i = 0; i < static_cast<int>(picker_exercises_.size()); i++) {
            if (ImGui::Selectable(picker_exercises_[i].name.c_str())) {
                current_exercise_id_ = picker_exercises_[i].id;
                current_exercise_name_ = picker_exercises_[i].name;
                set_count_ = 0;
                show_exercise_picker_ = false;
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Or type new name and press Enter:");

        static char new_exercise_buf[128] = {};
        if (ImGui::InputText("##new_ex", new_exercise_buf, sizeof(new_exercise_buf),
                ImGuiInputTextFlags_EnterReturnsTrue) && new_exercise_buf[0] != '\0') {
            auto found = repo_.find_exercise_by_name(new_exercise_buf);
            if (found) {
                current_exercise_id_ = found->id;
                current_exercise_name_ = found->name;
            } else {
                sf::Exercise ex;
                ex.name = new_exercise_buf;
                ex.id = repo_.add_exercise(ex);
                current_exercise_id_ = ex.id;
                current_exercise_name_ = ex.name;
            }
            set_count_ = 0;
            std::memset(new_exercise_buf, 0, sizeof(new_exercise_buf));
            show_exercise_picker_ = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void WorkoutView::render_add_set_popup() {
    if (!show_add_set_) return;

    ImGui::OpenPopup("Add Set");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(350, 220));

    if (ImGui::BeginPopupModal("Add Set", &show_add_set_, ImGuiWindowFlags_NoResize)) {
        ImGui::Text("Set %d for %s", set_count_ + 1, current_exercise_name_.c_str());
        ImGui::Spacing();

        ImGui::InputText("Reps", set_reps_buf_, sizeof(set_reps_buf_));
        ImGui::InputText("Weight", set_weight_buf_, sizeof(set_weight_buf_));
        ImGui::InputText("RPE (optional)", set_rpe_buf_, sizeof(set_rpe_buf_));

        ImGui::Spacing();
        if (ImGui::Button("Save", ImVec2(120, 0))) {
            sf::WorkoutSet s;
            s.workout_id = workout_id_;
            s.exercise_id = current_exercise_id_;
            s.set_order = ++set_count_;

            if (set_reps_buf_[0]) s.reps = std::atoi(set_reps_buf_);
            if (set_weight_buf_[0]) s.weight = std::atof(set_weight_buf_);
            if (set_rpe_buf_[0]) s.rpe = std::atof(set_rpe_buf_);

            repo_.add_set(s);
            needs_refresh_ = true;
            show_add_set_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            show_add_set_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void WorkoutView::render_edit_weight_popup() {
    if (!show_edit_weight_) return;

    ImGui::OpenPopup("Edit Set");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(350, 180));

    if (ImGui::BeginPopupModal("Edit Set", &show_edit_weight_, ImGuiWindowFlags_NoResize)) {
        ImGui::InputText("Weight", edit_weight_buf_, sizeof(edit_weight_buf_));
        ImGui::InputText("RPE", edit_rpe_buf_, sizeof(edit_rpe_buf_));

        ImGui::Spacing();
        if (ImGui::Button("Save", ImVec2(120, 0))) {
            for (auto& s : sets_) {
                if (s.id == edit_set_id_) {
                    if (edit_weight_buf_[0]) s.weight = std::atof(edit_weight_buf_);
                    if (edit_rpe_buf_[0]) s.rpe = std::atof(edit_rpe_buf_);
                    repo_.update_set(s);
                    break;
                }
            }
            needs_refresh_ = true;
            show_edit_weight_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            show_edit_weight_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void WorkoutView::render_template_picker() {
    if (!show_template_picker_) return;

    ImGui::OpenPopup("Pick Template");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 400));

    if (ImGui::BeginPopupModal("Pick Template", &show_template_picker_, ImGuiWindowFlags_NoResize)) {
        for (int i = 0; i < static_cast<int>(picker_templates_.size()); i++) {
            auto& t = picker_templates_[i];
            auto sets = repo_.get_template_sets(t.id);
            std::string label = t.name + " (" + std::to_string(sets.size()) + " sets)";

            ImGui::PushID(static_cast<int>(t.id));
            if (ImGui::Selectable(label.c_str())) {
                workout_id_ = repo_.start_workout_from_template(t.id);
                needs_refresh_ = true;
                auto ws = repo_.get_sets_for_workout(workout_id_);
                set_count_ = static_cast<int>(ws.size());
                show_template_picker_ = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopID();
        }
        ImGui::EndPopup();
    }
}

} // namespace gui
