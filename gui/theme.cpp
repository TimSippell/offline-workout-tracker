#include "theme.h"
#include "imgui.h"

namespace gui {

void setup_theme() {
    ImGui::StyleColorsDark();
    auto& style = ImGui::GetStyle();

    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.WindowPadding = ImVec2(12, 12);
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 6);

    auto& colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.16f, 0.36f, 0.36f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.45f, 0.45f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.24f, 0.52f, 0.52f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.16f, 0.36f, 0.36f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.45f, 0.45f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.24f, 0.52f, 0.52f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.20f, 0.24f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.14f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.20f, 0.45f, 0.45f, 1.00f);
    colors[ImGuiCol_TabSelected] = ImVec4(0.18f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.30f, 0.80f, 0.70f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.30f, 0.80f, 0.70f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.45f, 0.45f, 0.40f);
}

} // namespace gui
