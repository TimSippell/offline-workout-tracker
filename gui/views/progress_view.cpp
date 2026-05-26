#include "progress_view.h"
#include "imgui.h"
#include "implot.h"
#include <algorithm>

namespace gui {

ProgressView::ProgressView(sf::Repository& repo) : repo_(repo) {}

void ProgressView::render() {
    if (needs_refresh_) {
        exercises_ = repo_.list_exercises();
        needs_refresh_ = false;
    }

    ImGui::Text("Progress");
    ImGui::Separator();
    ImGui::Spacing();

    if (exercises_.empty()) {
        ImGui::Text("No exercises yet.");
        return;
    }

    float list_width = 250.0f;

    ImGui::BeginChild("exercise_list", ImVec2(list_width, 0), ImGuiChildFlags_Borders);
    ImGui::Text("Select Exercise");
    ImGui::Separator();

    for (int i = 0; i < static_cast<int>(exercises_.size()); i++) {
        bool is_selected = (selected_ == i);
        if (ImGui::Selectable(exercises_[i].name.c_str(), is_selected)) {
            selected_ = i;
            progression_ = repo_.get_progression(exercises_[i].id, 20);
            std::reverse(progression_.begin(), progression_.end());
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("chart_area", ImVec2(0, 0));

    if (selected_ < 0 || selected_ >= static_cast<int>(exercises_.size())) {
        ImGui::Text("Select an exercise to view progress.");
        ImGui::EndChild();
        return;
    }

    ImGui::Text("Progress: %s", exercises_[selected_].name.c_str());
    ImGui::Spacing();

    if (progression_.empty()) {
        ImGui::Text("No data yet.");
        ImGui::EndChild();
        return;
    }

    if (ImGui::BeginTable("prog_data", 4,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg,
            ImVec2(0, 200))) {
        ImGui::TableSetupColumn("Date", ImGuiTableColumnFlags_WidthFixed, 120);
        ImGui::TableSetupColumn("e1RM", ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableSetupColumn("Best Wt", ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableSetupColumn("Volume", ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableHeadersRow();

        for (const auto& p : progression_) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::TextUnformatted(p.date.c_str());
            ImGui::TableNextColumn(); ImGui::Text("%.1f", p.estimated_1rm);
            ImGui::TableNextColumn(); ImGui::Text("%.1f", p.best_set_weight);
            ImGui::TableNextColumn(); ImGui::Text("%.0f", p.session_volume);
        }
        ImGui::EndTable();
    }

    ImGui::Spacing();

    if (progression_.size() >= 2) {
        std::vector<double> x_vals, e1rm_vals, best_wt_vals, volume_vals;
        for (int i = 0; i < static_cast<int>(progression_.size()); i++) {
            x_vals.push_back(static_cast<double>(i));
            e1rm_vals.push_back(progression_[i].estimated_1rm);
            best_wt_vals.push_back(progression_[i].best_set_weight);
            volume_vals.push_back(progression_[i].session_volume);
        }
        int n = static_cast<int>(x_vals.size());

        if (ImPlot::BeginPlot("Estimated 1RM", ImVec2(-1, 250))) {
            ImPlot::SetupAxes("Session", "Weight");
            ImPlot::PlotLine("e1RM", x_vals.data(), e1rm_vals.data(), n);
            ImPlot::PlotLine("Best Weight", x_vals.data(), best_wt_vals.data(), n);
            ImPlot::EndPlot();
        }

        if (ImPlot::BeginPlot("Volume", ImVec2(-1, 200))) {
            ImPlot::SetupAxes("Session", "Volume");
            ImPlot::PlotBars("Volume", x_vals.data(), volume_vals.data(), n, 0.6);
            ImPlot::EndPlot();
        }
    }

    ImGui::EndChild();
}

} // namespace gui
