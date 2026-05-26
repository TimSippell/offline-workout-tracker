#include "exercise_view.h"
#include "imgui.h"
#include <swt/calculator.h>
#include <algorithm>
#include <cstring>

namespace gui {

static const char* categories[] = {"", "compound", "isolation", "cardio", "bodyweight"};
static const char* muscle_groups[] = {"", "chest", "back", "legs", "shoulders", "arms", "core", "fullbody"};

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
        std::memset(add_name_, 0, sizeof(add_name_));
        add_category_ = 0;
        add_muscle_ = 0;
    }

    ImGui::Spacing();

    if (exercises_.empty()) {
        ImGui::Text("No exercises found.");
    } else {
        if (ImGui::BeginTable("exercises", 3,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable,
                ImVec2(0, ImGui::GetContentRegionAvail().y - 40))) {

            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthFixed, 120);
            ImGui::TableSetupColumn("Muscle Group", ImGuiTableColumnFlags_WidthFixed, 120);
            ImGui::TableHeadersRow();

            for (int i = 0; i < static_cast<int>(exercises_.size()); i++) {
                const auto& ex = exercises_[i];
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                bool is_selected = (selected_ == i);
                if (ImGui::Selectable(ex.name.c_str(), is_selected,
                        ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {
                    selected_ = i;
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        detail_exercise_ = ex;
                        auto sets = repo_.get_sets_for_exercise(ex.id);
                        detail_stats_ = sf::compute_stats(ex.id, sets);
                        show_detail_ = true;
                    }
                }

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(ex.category.empty() ? "-" : ex.category.c_str());
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(ex.muscle_group.empty() ? "-" : ex.muscle_group.c_str());
            }
            ImGui::EndTable();
        }

        if (selected_ >= 0 && selected_ < static_cast<int>(exercises_.size())) {
            if (ImGui::Button("Delete")) {
                repo_.delete_exercise(exercises_[selected_].id);
                selected_ = -1;
                needs_refresh_ = true;
            }
        }
    }

    render_add_popup();
    render_detail_popup();
}

void ExerciseView::render_add_popup() {
    if (!show_add_) return;

    ImGui::OpenPopup("Add Exercise");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 250));

    if (ImGui::BeginPopupModal("Add Exercise", &show_add_, ImGuiWindowFlags_NoResize)) {
        ImGui::InputText("Name", add_name_, sizeof(add_name_));
        ImGui::Combo("Category", &add_category_, categories, IM_ARRAYSIZE(categories));
        ImGui::Combo("Muscle Group", &add_muscle_, muscle_groups, IM_ARRAYSIZE(muscle_groups));

        ImGui::Spacing();
        if (ImGui::Button("Save", ImVec2(120, 0)) && add_name_[0] != '\0') {
            sf::Exercise ex;
            ex.name = add_name_;
            ex.category = categories[add_category_];
            ex.muscle_group = muscle_groups[add_muscle_];
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

void ExerciseView::render_detail_popup() {
    if (!show_detail_) return;

    ImGui::OpenPopup("Exercise Detail");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 250));

    if (ImGui::BeginPopupModal("Exercise Detail", &show_detail_, ImGuiWindowFlags_NoResize)) {
        ImGui::Text("%s", detail_exercise_.name.c_str());
        ImGui::Separator();

        ImGui::Text("Category: %s", detail_exercise_.category.empty() ? "-" : detail_exercise_.category.c_str());
        ImGui::Text("Muscle: %s", detail_exercise_.muscle_group.empty() ? "-" : detail_exercise_.muscle_group.c_str());
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
