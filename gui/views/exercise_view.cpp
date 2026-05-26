#include "exercise_view.h"
#include "imgui.h"
#include <swt/calculator.h>
#include <algorithm>
#include <cstring>

namespace gui {

static const char* categories[] = {"", "compound", "isolation", "cardio", "bodyweight"};
static const char* muscle_groups[] = {"", "chest", "back", "legs", "shoulders", "arms", "core", "fullbody"};
static const char* tracking_types[] = {"weight", "time"};

static int find_index(const char** arr, int count, const std::string& val) {
    for (int i = 0; i < count; i++)
        if (val == arr[i]) return i;
    return 0;
}

ExerciseView::ExerciseView(sf::Repository& repo) : repo_(repo) {}

void ExerciseView::render() {
    if (needs_refresh_) {
        exercises_ = repo_.list_exercises(filter_buf_);
        needs_refresh_ = false;
    }

    ImGui::Text("Exercises");
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::InputText("Filter", filter_buf_, sizeof(filter_buf_))) {
        needs_refresh_ = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Add")) {
        show_add_ = true;
        open_add_ = true;
        std::memset(add_name_, 0, sizeof(add_name_));
        add_category_ = 0;
        add_muscle_ = 0;
        add_tracking_ = 0;
    }

    ImGui::Spacing();

    if (exercises_.empty()) {
        ImGui::Text("No exercises found.");
    } else {
        if (ImGui::BeginTable("exercises", 4,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable,
                ImVec2(0, ImGui::GetContentRegionAvail().y - 40))) {

            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthFixed, 120);
            ImGui::TableSetupColumn("Muscle Group", ImGuiTableColumnFlags_WidthFixed, 120);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 70);
            ImGui::TableHeadersRow();

            for (int i = 0; i < static_cast<int>(exercises_.size()); i++) {
                const auto& ex = exercises_[i];
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                ImGui::PushID(static_cast<int>(ex.id));
                bool is_selected = (selected_ == i);
                if (ImGui::Selectable(ex.name.c_str(), is_selected,
                        ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {
                    selected_ = i;
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        detail_exercise_ = ex;
                        auto sets = repo_.get_sets_for_exercise(ex.id);
                        detail_stats_ = sf::compute_stats(ex.id, sets);
                        show_detail_ = true;
                        open_detail_ = true;
                    }
                }

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(ex.category.empty() ? "-" : ex.category.c_str());
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(ex.muscle_group.empty() ? "-" : ex.muscle_group.c_str());
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(ex.notes == "time" ? "Time" : "Weight");
                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        if (selected_ >= 0 && selected_ < static_cast<int>(exercises_.size())) {
            if (ImGui::Button("Edit")) {
                auto& ex = exercises_[selected_];
                edit_exercise_ = ex;
                std::strncpy(edit_name_, ex.name.c_str(), sizeof(edit_name_) - 1);
                edit_name_[sizeof(edit_name_) - 1] = '\0';
                edit_category_ = find_index(categories, IM_ARRAYSIZE(categories), ex.category);
                edit_muscle_ = find_index(muscle_groups, IM_ARRAYSIZE(muscle_groups), ex.muscle_group);
                edit_tracking_ = (ex.notes == "time") ? 1 : 0;
                show_edit_ = true;
                open_edit_ = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Delete")) {
                repo_.delete_exercise(exercises_[selected_].id);
                selected_ = -1;
                needs_refresh_ = true;
            }
        }
    }

    render_add_popup();
    render_edit_popup();
    render_detail_popup();
}

void ExerciseView::render_add_popup() {
    if (!show_add_) return;

    if (open_add_) {
        ImGui::OpenPopup("Add Exercise");
        open_add_ = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 290));

    if (ImGui::BeginPopupModal("Add Exercise", &show_add_, ImGuiWindowFlags_NoResize)) {
        ImGui::InputText("Name", add_name_, sizeof(add_name_));
        ImGui::Combo("Category", &add_category_, categories, IM_ARRAYSIZE(categories));
        ImGui::Combo("Muscle Group", &add_muscle_, muscle_groups, IM_ARRAYSIZE(muscle_groups));
        ImGui::Combo("Tracking", &add_tracking_, tracking_types, IM_ARRAYSIZE(tracking_types));

        ImGui::Spacing();
        if (ImGui::Button("Save", ImVec2(120, 0)) && add_name_[0] != '\0') {
            sf::Exercise ex;
            ex.name = add_name_;
            ex.category = categories[add_category_];
            ex.muscle_group = muscle_groups[add_muscle_];
            ex.notes = tracking_types[add_tracking_];
            repo_.add_exercise(ex);
            needs_refresh_ = true;
            show_add_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            show_add_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void ExerciseView::render_edit_popup() {
    if (!show_edit_) return;

    if (open_edit_) {
        ImGui::OpenPopup("Edit Exercise");
        open_edit_ = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 290));

    if (ImGui::BeginPopupModal("Edit Exercise", &show_edit_, ImGuiWindowFlags_NoResize)) {
        ImGui::InputText("Name", edit_name_, sizeof(edit_name_));
        ImGui::Combo("Category", &edit_category_, categories, IM_ARRAYSIZE(categories));
        ImGui::Combo("Muscle Group", &edit_muscle_, muscle_groups, IM_ARRAYSIZE(muscle_groups));
        ImGui::Combo("Tracking", &edit_tracking_, tracking_types, IM_ARRAYSIZE(tracking_types));

        ImGui::Spacing();
        if (ImGui::Button("Save", ImVec2(120, 0)) && edit_name_[0] != '\0') {
            edit_exercise_.name = edit_name_;
            edit_exercise_.category = categories[edit_category_];
            edit_exercise_.muscle_group = muscle_groups[edit_muscle_];
            edit_exercise_.notes = tracking_types[edit_tracking_];
            repo_.update_exercise(edit_exercise_);
            needs_refresh_ = true;
            show_edit_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            show_edit_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void ExerciseView::render_detail_popup() {
    if (!show_detail_) return;

    if (open_detail_) {
        ImGui::OpenPopup("Exercise Detail");
        open_detail_ = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 280));

    if (ImGui::BeginPopupModal("Exercise Detail", &show_detail_, ImGuiWindowFlags_NoResize)) {
        ImGui::Text("%s", detail_exercise_.name.c_str());
        ImGui::Separator();

        ImGui::Text("Category: %s", detail_exercise_.category.empty() ? "-" : detail_exercise_.category.c_str());
        ImGui::Text("Muscle: %s", detail_exercise_.muscle_group.empty() ? "-" : detail_exercise_.muscle_group.c_str());
        ImGui::Text("Tracking: %s", detail_exercise_.notes == "time" ? "Time" : "Weight");
        ImGui::Spacing();
        ImGui::Text("Estimated 1RM: %.1f", detail_stats_.estimated_1rm);
        ImGui::Text("Best weight: %.1f x %d", detail_stats_.best_weight, detail_stats_.best_reps_at_best_weight);
        ImGui::Text("Total volume: %.0f", detail_stats_.total_volume);
        ImGui::Text("Sessions: %d", detail_stats_.session_count);

        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(120, 0))) {
            show_detail_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

} // namespace gui
