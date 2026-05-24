#pragma once

#include "database.h"
#include "models.h"
#include <optional>
#include <string>
#include <vector>

namespace sf {

class Repository {
public:
    explicit Repository(Database& db);

    // Exercises
    int64_t add_exercise(const Exercise& ex);
    std::optional<Exercise> get_exercise(int64_t id);
    std::optional<Exercise> find_exercise_by_name(const std::string& name);
    std::vector<Exercise> list_exercises(const std::string& filter = "");
    void update_exercise(const Exercise& ex);
    void delete_exercise(int64_t id);

    // Workouts
    int64_t start_workout(const std::string& name = "");
    void finish_workout(int64_t workout_id);
    std::optional<Workout> get_workout(int64_t id);
    std::vector<Workout> list_workouts(int limit = 20, int offset = 0);
    void update_workout(const Workout& w);
    void delete_workout(int64_t id);

    // Sets
    int64_t add_set(const WorkoutSet& s);
    void update_set(const WorkoutSet& s);
    void delete_set(int64_t id);
    std::vector<WorkoutSet> get_sets_for_workout(int64_t workout_id);
    std::vector<WorkoutSet> get_sets_for_exercise(int64_t exercise_id, int limit = 100);

    // History
    std::vector<ProgressionPoint> get_progression(int64_t exercise_id, int last_n_sessions = 20);

private:
    Database& db_;
};

} // namespace sf
