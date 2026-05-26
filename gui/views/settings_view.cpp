#include "settings_view.h"
#include "imgui.h"
#include <swt/data_io.h>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <sqlite3.h>

namespace gui {

static std::string expand_path(const std::string& raw) {
    std::string path = raw;
    if (!path.empty() && path[0] == '~') {
#ifdef _WIN32
        const char* home = std::getenv("USERPROFILE");
#else
        const char* home = std::getenv("HOME");
#endif
        if (home) path = std::string(home) + path.substr(1);
    }
    return path;
}

SettingsView::SettingsView(sf::Repository& repo) : repo_(repo) {}

void SettingsView::render() {
    if (!loaded_) {
        std::string unit = repo_.get_weight_unit();
        weight_unit_ = (unit == "lbs") ? 1 : 0;
        loaded_ = true;
    }

    ImGui::Text("Settings");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Weight Unit:");
    int prev = weight_unit_;
    ImGui::RadioButton("kg", &weight_unit_, 0);
    ImGui::SameLine();
    ImGui::RadioButton("lbs", &weight_unit_, 1);

    if (weight_unit_ != prev) {
        repo_.set_weight_unit(weight_unit_ == 1 ? "lbs" : "kg");
    }

    ImGui::Spacing();
    ImGui::Spacing();
    render_import_export();

    ImGui::Spacing();
    ImGui::Spacing();
    render_danger_zone();
}

void SettingsView::render_import_export() {
    ImGui::SeparatorText("Import / Export");
    ImGui::Spacing();

    ImGui::InputText("File path", path_buf_, sizeof(path_buf_));
    ImGui::Spacing();

    const char* scopes[] = {"Workout history only", "Exercises + workouts", "Everything (+ templates)"};
    ImGui::Text("Scope:");
    for (int i = 0; i < 3; i++) {
        ImGui::RadioButton(scopes[i], &scope_selection_, i);
        if (i < 2) ImGui::SameLine();
    }

    ImGui::Spacing();

    if (ImGui::Button("Export", ImVec2(150, 32))) {
        do_export();
    }
    ImGui::SameLine();
    if (ImGui::Button("Import", ImVec2(150, 32))) {
        do_import();
    }

    ImGui::Spacing();

    if (!status_msg_.empty()) {
        if (status_is_error_)
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", status_msg_.c_str());
        else
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.5f, 1.0f), "%s", status_msg_.c_str());
    }

    if (show_import_preview_) {
        ImGui::OpenPopup("Import Preview");
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(400, 300));

        if (ImGui::BeginPopupModal("Import Preview", &show_import_preview_, ImGuiWindowFlags_NoResize)) {
            ImGui::Text("Import Preview");
            ImGui::Separator();

            if (import_summary_.new_exercises || import_summary_.existing_exercises) {
                ImGui::Text("New exercises: %d", import_summary_.new_exercises);
                ImGui::Text("Existing exercises: %d (will be merged)", import_summary_.existing_exercises);
            }
            if (import_summary_.workouts)
                ImGui::Text("Workouts: %d (%d sets)", import_summary_.workouts, import_summary_.workout_sets);
            if (import_summary_.templates)
                ImGui::Text("Templates: %d (%d sets)", import_summary_.templates, import_summary_.template_sets);

            ImGui::Spacing();
            if (ImGui::Button("Yes, import", ImVec2(150, 0))) {
                try {
                    auto result = sf::import_from_json(repo_, import_json_);
                    status_msg_ = "Imported " + std::to_string(result.workouts) + " workouts, "
                                + std::to_string(result.templates) + " templates, "
                                + std::to_string(result.sets) + " sets.";
                    status_is_error_ = false;
                } catch (...) {
                    status_msg_ = "Import failed.";
                    status_is_error_ = true;
                }
                show_import_preview_ = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(150, 0))) {
                show_import_preview_ = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}

void SettingsView::render_danger_zone() {
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.5f, 0.1f, 0.1f, 1.0f));
    ImGui::SeparatorText("Danger Zone");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    ImGui::TextWrapped("This will permanently delete all exercises, workouts, templates, and settings.");
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
    if (ImGui::Button("Reset All Data", ImVec2(200, 36))) {
        show_reset_confirm_ = true;
        reset_done_ = false;
    }
    ImGui::PopStyleColor(3);

    if (show_reset_confirm_) {
        ImGui::OpenPopup("Confirm Reset");
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(420, 160));

        if (ImGui::BeginPopupModal("Confirm Reset", &show_reset_confirm_, ImGuiWindowFlags_NoResize)) {
            if (!reset_done_) {
                ImGui::TextWrapped("Are you sure? This cannot be undone. All data will be permanently deleted.");
                ImGui::Spacing();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.15f, 0.15f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
                if (ImGui::Button("Yes, delete everything", ImVec2(200, 0))) {
                    const char* sql =
                        "DELETE FROM workout_set;"
                        "DELETE FROM template_set;"
                        "DELETE FROM workout;"
                        "DELETE FROM workout_template;"
                        "DELETE FROM exercise;"
                        "DELETE FROM settings;";
                    sqlite3_exec(repo_.db_handle(), sql, nullptr, nullptr, nullptr);
                    reset_done_ = true;
                    loaded_ = false;
                    weight_unit_ = 0;
                }
                ImGui::PopStyleColor(3);

                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    show_reset_confirm_ = false;
                    ImGui::CloseCurrentPopup();
                }
            } else {
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.5f, 1.0f), "All data has been reset.");
                ImGui::Spacing();
                if (ImGui::Button("OK", ImVec2(120, 0))) {
                    show_reset_confirm_ = false;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndPopup();
        }
    }
}

void SettingsView::do_export() {
    std::string path = expand_path(path_buf_);
    if (path.empty()) {
        status_msg_ = "Enter a file path.";
        status_is_error_ = true;
        return;
    }

    if (std::filesystem::is_directory(path)) {
        if (path.back() != '/' && path.back() != '\\') path += '/';
        path += "workouts.json";
    }

    std::ofstream out(path);
    if (!out) {
        status_msg_ = "Could not write to: " + path;
        status_is_error_ = true;
        return;
    }

    sf::ExportScope scope;
    switch (scope_selection_) {
        case 0: scope = sf::ExportScope::History; break;
        case 1: scope = sf::ExportScope::ExercisesAndWorkouts; break;
        default: scope = sf::ExportScope::All; break;
    }

    sf::export_to_json(repo_, out, scope);
    out.close();

    status_msg_ = "Exported to: " + path;
    status_is_error_ = false;
}

void SettingsView::do_import() {
    std::string path = expand_path(path_buf_);
    if (path.empty()) {
        status_msg_ = "Enter a file path.";
        status_is_error_ = true;
        return;
    }

    std::ifstream in(path);
    if (!in) {
        status_msg_ = "Could not read: " + path;
        status_is_error_ = true;
        return;
    }

    import_json_ = std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    try {
        import_summary_ = sf::preview_import(repo_, import_json_);
        show_import_preview_ = true;
        status_msg_.clear();
    } catch (...) {
        status_msg_ = "Invalid JSON file.";
        status_is_error_ = true;
    }
}

} // namespace gui
