#include "template_view.h"
#include "imgui.h"
#include <cstring>
#include <cstdlib>
#include <format>

namespace gui {

TemplateView::TemplateView(sf::Repository& repo) : repo_(repo) {}

void TemplateView::render() {
    if (needs_refresh_) {
        templates_ = repo_.list_templates();
        needs_refresh_ = false;
    }

    ImGui::Text("Templates");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::InputText("Name", create_name_buf_, sizeof(create_name_buf_));
    ImGui::SameLine();
    if (ImGui::Button("Create") && create_name_buf_[0] != '\0') {
        sf::WorkoutTemplate t;
        t.name = create_name_buf_;
        int64_t id = repo_.create_template(t);
        std::memset(create_name_buf_, 0, sizeof(create_name_buf_));
        needs_refresh_ = true;

        edit_template_id_ = id;
        edit_template_name_ = t.name;
        edit_sets_.clear();
        edit_selected_ = -1;
        show_edit_ = true;
        open_edit_popup_ = true;
    }

    ImGui::Spacing();

    if (templates_.empty()) {
        ImGui::Text("No templates yet.");
    } else {
        ImGui::BeginChild("template_list", ImVec2(0, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_Border);
        for (int i = 0; i < static_cast<int>(templates_.size()); i++) {
            const auto& t = templates_[i];
            auto sets = repo_.get_template_sets(t.id);
            bool is_expanded = (expanded_id_ == t.id);

            ImGui::PushID(static_cast<int>(t.id));

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowOverlap;
            if (is_expanded) flags |= ImGuiTreeNodeFlags_DefaultOpen;

            bool open = ImGui::TreeNodeEx(t.name.c_str(), flags);

            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 90);
            if (ImGui::SmallButton("Edit")) {
                edit_template_id_ = t.id;
                edit_template_name_ = t.name;
                edit_sets_ = sets;
                edit_selected_ = -1;
                show_edit_ = true;
                open_edit_popup_ = true;
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Delete")) {
                repo_.clear_template_ref(t.id);
                repo_.delete_template(t.id);
                needs_refresh_ = true;
            }

            if (open) {
                expanded_id_ = t.id;
                if (sets.empty()) {
                    ImGui::TextDisabled("No exercises");
                } else {
                    if (ImGui::BeginTable("##sets", 5, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerH)) {
                        ImGui::TableSetupColumn("Exercise", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("Reps", ImGuiTableColumnFlags_WidthFixed, 50);
                        ImGui::TableSetupColumn("Duration", ImGuiTableColumnFlags_WidthFixed, 60);
                        ImGui::TableSetupColumn("Rest", ImGuiTableColumnFlags_WidthFixed, 50);
                        ImGui::TableSetupColumn("RPE", ImGuiTableColumnFlags_WidthFixed, 50);
                        ImGui::TableHeadersRow();

                        for (const auto& s : sets) {
                            auto ex = repo_.get_exercise(s.exercise_id);
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::TextUnformatted(ex ? ex->name.c_str() : "?");
                            ImGui::TableNextColumn();
                            ImGui::Text("%s", s.reps ? std::to_string(*s.reps).c_str() : "-");
                            ImGui::TableNextColumn();
                            ImGui::Text("%s", s.duration_secs ? (std::to_string(*s.duration_secs) + "s").c_str() : "-");
                            ImGui::TableNextColumn();
                            ImGui::Text("%s", s.rest_secs ? (std::to_string(*s.rest_secs) + "s").c_str() : "-");
                            ImGui::TableNextColumn();
                            ImGui::Text("%s", s.rpe ? std::format("{:.0f}", *s.rpe).c_str() : "-");
                        }
                        ImGui::EndTable();
                    }
                }
                ImGui::TreePop();
            } else if (is_expanded) {
                expanded_id_ = -1;
            }

            ImGui::PopID();
        }
        ImGui::EndChild();
    }

    render_edit_popup();
}

void TemplateView::render_edit_popup() {
    if (!show_edit_) return;

    if (open_edit_popup_) {
        ImGui::OpenPopup("Edit Template");
        open_edit_popup_ = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(600, 500));

    if (ImGui::BeginPopupModal("Edit Template", &show_edit_, ImGuiWindowFlags_NoResize)) {
        ImGui::Text("Template: %s", edit_template_name_.c_str());
        ImGui::Separator();

        if (ImGui::Button("Add Exercise")) {
            add_exercises_ = repo_.list_exercises();
            if (!add_exercises_.empty()) {
                add_exercise_pick_ = 0;
                std::memset(add_num_sets_buf_, 0, sizeof(add_num_sets_buf_));
                std::memset(add_reps_buf_, 0, sizeof(add_reps_buf_));
                std::memset(add_rpe_buf_, 0, sizeof(add_rpe_buf_));
                std::memset(add_time_buf_, 0, sizeof(add_time_buf_));
                std::memset(add_rest_buf_, 0, sizeof(add_rest_buf_));
                show_add_exercise_ = true;
                open_add_exercise_popup_ = true;
            }
        }

        ImGui::Spacing();

        if (edit_sets_.empty()) {
            ImGui::Text("No exercises yet.");
        } else {
            if (ImGui::BeginTable("tmpl_sets", 6,
                    ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
                    ImVec2(0, ImGui::GetContentRegionAvail().y - 80))) {

                ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 40);
                ImGui::TableSetupColumn("Exercise", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Reps", ImGuiTableColumnFlags_WidthFixed, 50);
                ImGui::TableSetupColumn("Duration", ImGuiTableColumnFlags_WidthFixed, 60);
                ImGui::TableSetupColumn("Rest", ImGuiTableColumnFlags_WidthFixed, 50);
                ImGui::TableSetupColumn("RPE", ImGuiTableColumnFlags_WidthFixed, 50);
                ImGui::TableHeadersRow();

                for (int i = 0; i < static_cast<int>(edit_sets_.size()); i++) {
                    const auto& s = edit_sets_[i];
                    auto ex = repo_.get_exercise(s.exercise_id);

                    ImGui::PushID(i);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", s.set_order);

                    ImGui::TableNextColumn();
                    bool sel = (edit_selected_ == i);
                    if (ImGui::Selectable(ex ? ex->name.c_str() : "?", sel, ImGuiSelectableFlags_SpanAllColumns)) {
                        edit_selected_ = i;
                    }

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", s.reps ? std::to_string(*s.reps).c_str() : "-");

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", s.duration_secs ? (std::to_string(*s.duration_secs) + "s").c_str() : "-");

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", s.rest_secs ? (std::to_string(*s.rest_secs) + "s").c_str() : "-");

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", s.rpe ? std::format("{:.0f}", *s.rpe).c_str() : "-");
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }

            if (edit_selected_ >= 0 && edit_selected_ < static_cast<int>(edit_sets_.size())) {
                if (ImGui::Button("Delete Set")) {
                    repo_.delete_template_set(edit_sets_[edit_selected_].id);
                    edit_sets_ = repo_.get_template_sets(edit_template_id_);
                    edit_selected_ = -1;
                }
                ImGui::SameLine();
                if (edit_selected_ > 0 && ImGui::Button("Move Up")) {
                    repo_.swap_template_set_order(
                        edit_sets_[edit_selected_].id, edit_sets_[edit_selected_].set_order,
                        edit_sets_[edit_selected_ - 1].id, edit_sets_[edit_selected_ - 1].set_order);
                    edit_sets_ = repo_.get_template_sets(edit_template_id_);
                    edit_selected_--;
                }
                ImGui::SameLine();
                if (edit_selected_ < static_cast<int>(edit_sets_.size()) - 1 && ImGui::Button("Move Down")) {
                    repo_.swap_template_set_order(
                        edit_sets_[edit_selected_].id, edit_sets_[edit_selected_].set_order,
                        edit_sets_[edit_selected_ + 1].id, edit_sets_[edit_selected_ + 1].set_order);
                    edit_sets_ = repo_.get_template_sets(edit_template_id_);
                    edit_selected_++;
                }
            }
        }

        ImGui::Spacing();
        if (ImGui::Button("Done", ImVec2(120, 0))) {
            show_edit_ = false;
            needs_refresh_ = true;
            ImGui::CloseCurrentPopup();
        }

        render_add_exercise_popup();
        ImGui::EndPopup();
    }
}

void TemplateView::render_add_exercise_popup() {
    if (!show_add_exercise_) return;

    if (open_add_exercise_popup_) {
        ImGui::OpenPopup("Add Exercise to Template");
        open_add_exercise_popup_ = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 280));

    if (ImGui::BeginPopupModal("Add Exercise to Template", &show_add_exercise_, ImGuiWindowFlags_NoResize)) {
        std::vector<const char*> names;
        for (auto& e : add_exercises_) names.push_back(e.name.c_str());
        ImGui::Combo("Exercise", &add_exercise_pick_, names.data(), static_cast<int>(names.size()));

        ImGui::InputText("Number of sets", add_num_sets_buf_, sizeof(add_num_sets_buf_));
        ImGui::InputText("Reps per set", add_reps_buf_, sizeof(add_reps_buf_));
        ImGui::InputText("Duration (secs)", add_time_buf_, sizeof(add_time_buf_));
        ImGui::InputText("Rest (secs)", add_rest_buf_, sizeof(add_rest_buf_));
        ImGui::InputText("Target RPE (optional)", add_rpe_buf_, sizeof(add_rpe_buf_));

        ImGui::Spacing();
        if (ImGui::Button("Add", ImVec2(120, 0))) {
            if (add_exercise_pick_ < static_cast<int>(add_exercises_.size())) {
                int order = static_cast<int>(edit_sets_.size());
                int num_sets = add_num_sets_buf_[0] ? std::atoi(add_num_sets_buf_) : 1;
                if (num_sets > 0) {
                    for (int i = 0; i < num_sets; i++) {
                        sf::TemplateSet ts;
                        ts.template_id = edit_template_id_;
                        ts.exercise_id = add_exercises_[add_exercise_pick_].id;
                        ts.set_order = ++order;
                        if (add_reps_buf_[0]) ts.reps = std::atoi(add_reps_buf_);
                        if (add_time_buf_[0]) ts.duration_secs = std::atoi(add_time_buf_);
                        if (add_rest_buf_[0]) ts.rest_secs = std::atoi(add_rest_buf_);
                        if (add_rpe_buf_[0]) ts.rpe = std::atof(add_rpe_buf_);
                        repo_.add_template_set(ts);
                    }
                }
                edit_sets_ = repo_.get_template_sets(edit_template_id_);
            }
            show_add_exercise_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            show_add_exercise_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

} // namespace gui
