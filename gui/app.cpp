#include "app.h"
#include "imgui.h"
#include <swt/defaults.h>

namespace gui {

App::App(sf::Database& db)
    : repo_(db),
      workout_view_(repo_),
      history_view_(repo_),
      exercise_view_(repo_),
      progress_view_(repo_),
      settings_view_(repo_) {
    workout_view_.set_on_finish([this]() {
        history_view_.refresh();
        progress_view_.refresh();
    });
    settings_view_.set_on_reset([this]() {
        workout_view_.refresh_templates();
        history_view_.refresh();
        exercise_view_.refresh();
        progress_view_.refresh();
        first_run_checked_ = false;
    });
}

void App::render() {
    if (!first_run_checked_ && current_screen_ != Screen::Settings) {
        check_first_run();
        first_run_checked_ = true;
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    float sidebar_w = 180.0f;

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(ImVec2(sidebar_w, viewport->WorkSize.y));
    ImGui::Begin("##sidebar", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    render_sidebar();
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + sidebar_w, viewport->WorkPos.y));
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x - sidebar_w, viewport->WorkSize.y));
    ImGui::Begin("##content", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    switch (current_screen_) {
        case Screen::Workout:   workout_view_.render(); break;
        case Screen::History:   history_view_.render(); break;
        case Screen::Exercises: exercise_view_.render(); break;
        case Screen::Progress:  progress_view_.render(); break;
        case Screen::Settings:  settings_view_.render(); break;
    }

    ImGui::End();

    if (show_seed_popup_) {
        if (open_seed_popup_) {
            ImGui::OpenPopup("Welcome");
            open_seed_popup_ = false;
        }
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(400, 160));

        if (ImGui::BeginPopupModal("Welcome", nullptr, ImGuiWindowFlags_NoResize)) {
            ImGui::TextWrapped("No exercises or templates found. Load defaults?");
            ImGui::Spacing();
            ImGui::Spacing();

            if (ImGui::Button("Yes, load defaults", ImVec2(180, 0))) {
                sf::seed_default_exercises(repo_);
                sf::seed_default_templates(repo_);
                repo_.set_setup_complete(true);
                show_seed_popup_ = false;
                exercise_view_.refresh();
                workout_view_.refresh_templates();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("No, start empty", ImVec2(180, 0))) {
                repo_.set_setup_complete(true);
                show_seed_popup_ = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}

void App::render_sidebar() {
    ImGui::Text("WORKOUT TRACKER");
    ImGui::Separator();
    ImGui::Spacing();

    struct NavItem { const char* label; Screen screen; };
    NavItem items[] = {
        {"Workout",     Screen::Workout},
        {"History",     Screen::History},
        {"Exercises",   Screen::Exercises},
        {"Progress",    Screen::Progress},
        {"Settings",    Screen::Settings},
    };

    for (auto& item : items) {
        bool selected = (current_screen_ == item.screen);
        if (ImGui::Selectable(item.label, selected, 0, ImVec2(0, 28))) {
            current_screen_ = item.screen;
        }
    }
}

void App::check_first_run() {
    if (!repo_.is_setup_complete()) {
        auto exercises = repo_.list_exercises();
        auto templates = repo_.list_templates();
        if (exercises.empty() && templates.empty()) {
            show_seed_popup_ = true;
            open_seed_popup_ = true;
        } else {
            repo_.set_setup_complete(true);
        }
    }
}

} // namespace gui
