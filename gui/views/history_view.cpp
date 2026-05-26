#include "history_view.h"
#include "imgui.h"
#include <swt/calculator.h>
#include <format>

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

    if (ImGui::BeginTable("history", 3,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable,
            ImVec2(0, ImGui::GetContentRegionAvail().y))) {

        ImGui::TableSetupColumn("Date", ImGuiTableColumnFlags_WidthFixed, 180);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableHeadersRow();

        for (int i = 0; i < static_cast<int>(workouts_.size()); i++) {
            const auto& w = workouts_[i];
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            bool is_selected = (selected_ == i);
            if (ImGui::Selectable(w.started_at.c_str(), is_selected,
                    ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {
                selected_ = i;
                if (ImGui::IsMouseDoubleClicked(0)) {
                    auto full = repo_.get_workout(w.id);
                    if (full) {
                        detail_workout_ = *full;
                        show_detail_ = true;
                    }
                }
            }

            ImGui::TableNextColumn();
            ImGui::TextUnformatted(w.name.empty() ? "(unnamed)" : w.name.c_str());
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(w.finished_at.empty() ? "active" : "done");
        }
        ImGui::EndTable();
    }

    render_detail();
}

void HistoryView::render_detail() {
    if (!show_detail_) return;

    ImGui::OpenPopup("Workout Detail");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(700, 500));

    if (ImGui::BeginPopupModal("Workout Detail", &show_detail_, ImGuiWindowFlags_NoResize)) {
        ImGui::Text("Workout: %s", detail_workout_.name.empty() ? "(unnamed)" : detail_workout_.name.c_str());
        ImGui::Text("Started: %s", detail_workout_.started_at.c_str());
        if (!detail_workout_.finished_at.empty())
            ImGui::Text("Finished: %s", detail_workout_.finished_at.c_str());

        ImGui::Spacing();

        if (ImGui::BeginTable("detail_sets", 6,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
                ImVec2(0, ImGui::GetContentRegionAvail().y - 60))) {

            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 40);
            ImGui::TableSetupColumn("Exercise", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Reps", ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableSetupColumn("Weight", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableSetupColumn("RPE", ImGuiTableColumnFlags_WidthFixed, 50);
            ImGui::TableSetupColumn("e1RM", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableHeadersRow();

            for (const auto& s : detail_workout_.sets) {
                auto ex = repo_.get_exercise(s.exercise_id);
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::Text("%d", s.set_order);

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(ex ? ex->name.c_str() : "?");

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
            }
            ImGui::EndTable();
        }

        double vol = sf::calculate_volume(detail_workout_.sets);
        ImGui::Text("Total volume: %.0f", vol);

        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(120, 0))) {
            show_detail_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

} // namespace gui
