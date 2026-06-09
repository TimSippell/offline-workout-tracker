#pragma once

#include <cstdint>
#include <string>

namespace sf {

enum class Category {
    Compound,
    Isolation,
    Cardio,
    Bodyweight,
};

enum class MuscleGroup {
    Chest,
    Back,
    Legs,
    Shoulders,
    Arms,
    Core,
    FullBody,
};

std::string category_to_string(Category c);
Category category_from_string(const std::string& s);

std::string muscle_group_to_string(MuscleGroup m);
MuscleGroup muscle_group_from_string(const std::string& s);

} // namespace sf
