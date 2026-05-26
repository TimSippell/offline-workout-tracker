#pragma once

#include <swt/swt.h>
#include <vector>

namespace gui {

class ProgressView {
public:
    explicit ProgressView(sf::Repository& repo);
    void render();

private:
    sf::Repository& repo_;
    std::vector<sf::Exercise> exercises_;
    bool needs_refresh_ = true;

    int selected_ = -1;
    std::vector<sf::ProgressionPoint> progression_;
};

} // namespace gui
