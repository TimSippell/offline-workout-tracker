#pragma once

#include "repository.h"
#include <iosfwd>
#include <string>

namespace sf {

enum class ExportScope {
    History,
    ExercisesAndWorkouts,
    All
};

struct ImportSummary {
    int new_exercises = 0;
    int existing_exercises = 0;
    int workouts = 0;
    int workout_sets = 0;
    int templates = 0;
    int template_sets = 0;
};

struct ImportResult {
    int exercises = 0;
    int workouts = 0;
    int templates = 0;
    int sets = 0;
};

void export_to_json(Repository& repo, std::ostream& out, ExportScope scope = ExportScope::All);
ExportScope detect_import_scope(const std::string& json);
ImportSummary preview_import(Repository& repo, const std::string& json);
ImportResult import_from_json(Repository& repo, const std::string& json);

} // namespace sf
