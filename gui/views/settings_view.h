#pragma once

#include <swt/swt.h>
#include <string>
#include <functional>

namespace gui {

class SettingsView {
public:
    explicit SettingsView(sf::Repository& repo);
    void render();
    void set_on_reset(std::function<void()> cb) { on_reset_ = std::move(cb); }

private:
    void render_import_export();
    void render_danger_zone();
    void do_export();
    void do_import();

    sf::Repository& repo_;
    int weight_unit_ = 0;
    bool loaded_ = false;

    char path_buf_[512] = {};
    int scope_selection_ = 2;
    std::string status_msg_;
    bool status_is_error_ = false;

    bool show_import_preview_ = false;
    sf::ImportSummary import_summary_;
    std::string import_json_;

    bool show_reset_confirm_ = false;
    bool reset_done_ = false;

    std::function<void()> on_reset_;
};

} // namespace gui
