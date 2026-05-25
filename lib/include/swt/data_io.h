#pragma once

#include "repository.h"
#include <string>

namespace sf {

struct ImportSummary {
    int new_exercises = 0;
    int existing_exercises = 0;
    int workouts = 0;
    int workout_sets = 0;
    int templates = 0;
    int template_sets = 0;
};

struct ImportResult {
    int workouts = 0;
    int templates = 0;
    int sets = 0;
};

std::string export_to_json(Repository& repo);
ImportSummary preview_import(Repository& repo, const std::string& json);
ImportResult import_from_json(Repository& repo, const std::string& json);

} // namespace sf
