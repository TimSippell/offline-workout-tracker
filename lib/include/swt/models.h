#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace sf {

struct Exercise {
    int64_t id = 0;
    std::string name;
    std::string category;
    std::string muscle_group;
    std::string notes;
};

struct WorkoutSet {
    int64_t id = 0;
    int64_t workout_id = 0;
    int64_t exercise_id = 0;
    int set_order = 0;
    std::optional<int> reps;
    std::optional<double> weight;
    std::optional<double> rpe;
    std::optional<int> rest_secs;
    std::string tempo;
    std::string notes;
};

struct Workout {
    int64_t id = 0;
    std::string name;
    std::string started_at;
    std::string finished_at;
    std::string notes;
    std::vector<WorkoutSet> sets;
};

struct ExerciseStats {
    int64_t exercise_id = 0;
    double estimated_1rm = 0.0;
    double best_weight = 0.0;
    int best_reps_at_best_weight = 0;
    double total_volume = 0.0;
    int session_count = 0;
};

struct ProgressionPoint {
    std::string date;
    double estimated_1rm = 0.0;
    double best_set_weight = 0.0;
    double session_volume = 0.0;
};

} // namespace sf
