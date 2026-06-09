#include "history_view.h"
#include "imgui.h"
#include <owt/calculator.h>
#include <format>
#include <cstring>
#include <cstdio>
#include <cstdlib>

namespace gui {

HistoryView::HistoryView(sf::Repository& repo) : repo_(repo) {}

void HistoryView::render() {
    if (needs_refresh_) {
        workouts_ = repo_.list_workouts(50);
        needs_refresh_ = false;
    }

    ImGui::Text("History");
    ImGui::Separator();
    ImGui::Spacing();

    if (workouts_.empty()) {
        ImGui::Text("No workouts yet.");
        return;
    }

    ImGui::BeginChild("history_list", ImVec2(0, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_Border);
    for (int i = 0; i < static_cast<int>(workouts_.size()); i++) {
        const auto& w = workouts_[i];
        bool is_expanded = (expanded_id_ == w.id);

        ImGui::PushID(static_cast<int>(w.id));

        std::string label = w.started_at;
        if (!w.name.empty()) label += "  —  " + w.name;
        if (w.finished_at.empty()) label += "  [active]";

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (is_expanded) flags |= ImGuiTreeNodeFlags_DefaultOpen;

        bool open = ImGui::TreeNodeEx(label.c_str(), flags);

        if (open) {
            if (!is_expanded) {
                expanded_id_ = w.id;
                expanded_sets_ = repo_.get_sets_for_workout(w.id);
            }

            if (expanded_sets_.empty()) {
                ImGui::TextDisabled("No sets");
            } else {
                if (ImGui::BeginTable("##sets", 8,
                        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerH)) {
                    ImGui::TableSetupColumn("Exercise", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Reps", ImGuiTableColumnFlags_WidthFixed, 50);
                    ImGui::TableSetupColumn("Weight", ImGuiTableColumnFlags_WidthFixed, 70);
                    ImGui::TableSetupColumn("Duration", ImGuiTableColumnFlags_WidthFixed, 60);
                    ImGui::TableSetupColumn("Rest", ImGuiTableColumnFlags_WidthFixed, 50);
                    ImGui::TableSetupColumn("RPE", ImGuiTableColumnFlags_WidthFixed, 50);
                    ImGui::TableSetupColumn("e1RM", ImGuiTableColumnFlags_WidthFixed, 60);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 40);
                    ImGui::TableHeadersRow();

                    for (auto& s : expanded_sets_) {
                        auto ex = repo_.get_exercise(s.exercise_id);

                        ImGui::PushID(static_cast<int>(s.id));
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(ex ? ex->name.c_str() : "?");

                        ImGui::TableNextColumn();
                        ImGui::Text("%s", s.reps ? std::to_string(*s.reps).c_str() : "-");

                        ImGui::TableNextColumn();
                        ImGui::Text("%s", s.weight ? std::format("{:.1f}", *s.weight).c_str() : "-");

                        ImGui::TableNextColumn();
                        ImGui::Text("%s", s.duration_secs ? (std::to_string(*s.duration_secs) + "s").c_str() : "-");

                        ImGui::TableNextColumn();
                        ImGui::Text("%s", s.rest_secs ? (std::to_string(*s.rest_secs) + "s").c_str() : "-");

                        ImGui::TableNextColumn();
                        ImGui::Text("%s", s.rpe ? std::format("{:.0f}", *s.rpe).c_str() : "-");

                        ImGui::TableNextColumn();
                        if (s.weight && s.reps && *s.reps > 0 && *s.weight > 0) {
                            double e1rm = s.rpe ? sf::estimate_1rm_rpe(*s.weight, *s.reps, *s.rpe)
                                                : sf::estimate_1rm(*s.weight, *s.reps);
                            ImGui::Text("%.1f", e1rm);
                        } else {
                            ImGui::TextUnformatted("-");
                        }

                        ImGui::TableNextColumn();
                        if (ImGui::SmallButton("Edit")) {
                            edit_set_ = s;
                            std::memset(edit_reps_buf_, 0, sizeof(edit_reps_buf_));
                            std::memset(edit_weight_buf_, 0, sizeof(edit_weight_buf_));
                            std::memset(edit_duration_buf_, 0, sizeof(edit_duration_buf_));
                            std::memset(edit_rest_buf_, 0, sizeof(edit_rest_buf_));
                            std::memset(edit_rpe_buf_, 0, sizeof(edit_rpe_buf_));
                            if (s.reps) snprintf(edit_reps_buf_, sizeof(edit_reps_buf_), "%d", *s.reps);
                            if (s.weight) snprintf(edit_weight_buf_, sizeof(edit_weight_buf_), "%.1f", *s.weight);
                            if (s.duration_secs) snprintf(edit_duration_buf_, sizeof(edit_duration_buf_), "%d", *s.duration_secs);
                            if (s.rest_secs) snprintf(edit_rest_buf_, sizeof(edit_rest_buf_), "%d", *s.rest_secs);
                            if (s.rpe) snprintf(edit_rpe_buf_, sizeof(edit_rpe_buf_), "%.0f", *s.rpe);
                            show_edit_set_ = true;
                            open_edit_set_ = true;
                        }
                        ImGui::PopID();
                    }
                    ImGui::EndTable();
                }
            }
            ImGui::TreePop();
        } else if (is_expanded) {
            expanded_id_ = -1;
            expanded_sets_.clear();
        }

        ImGui::PopID();
    }
    ImGui::EndChild();

    render_edit_set_popup();
}

void HistoryView::render_edit_set_popup() {
    if (!show_edit_set_) return;

    if (open_edit_set_) {
        ImGui::OpenPopup("Edit Set");
        open_edit_set_ = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(350, 280));

    if (ImGui::BeginPopupModal("Edit Set", &show_edit_set_, ImGuiWindowFlags_NoResize)) {
        ImGui::InputText("Reps", edit_reps_buf_, sizeof(edit_reps_buf_));
        ImGui::InputText("Weight", edit_weight_buf_, sizeof(edit_weight_buf_));
        ImGui::InputText("Duration (secs)", edit_duration_buf_, sizeof(edit_duration_buf_));
        ImGui::InputText("Rest (secs)", edit_rest_buf_, sizeof(edit_rest_buf_));
        ImGui::InputText("RPE", edit_rpe_buf_, sizeof(edit_rpe_buf_));

        ImGui::Spacing();
        if (ImGui::Button("Save", ImVec2(120, 0))) {
            edit_set_.reps = edit_reps_buf_[0] ? std::atoi(edit_reps_buf_) : std::optional<int>{};
            edit_set_.weight = edit_weight_buf_[0] ? std::atof(edit_weight_buf_) : std::optional<double>{};
            edit_set_.duration_secs = edit_duration_buf_[0] ? std::atoi(edit_duration_buf_) : std::optional<int>{};
            edit_set_.rest_secs = edit_rest_buf_[0] ? std::atoi(edit_rest_buf_) : std::optional<int>{};
            edit_set_.rpe = edit_rpe_buf_[0] ? std::atof(edit_rpe_buf_) : std::optional<double>{};
            repo_.update_set(edit_set_);
            expanded_sets_ = repo_.get_sets_for_workout(edit_set_.workout_id);
            show_edit_set_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            show_edit_set_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

} // namespace gui
