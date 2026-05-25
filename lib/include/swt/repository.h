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
    std::optional<Workout> get_active_workout();
    std::vector<Workout> list_workouts(int limit = 20, int offset = 0);
    void update_workout(const Workout& w);
    void delete_workout(int64_t id);

    // Sets
    int64_t add_set(const WorkoutSet& s);
    void update_set(const WorkoutSet& s);
    void delete_set(int64_t id);
    std::vector<WorkoutSet> get_sets_for_workout(int64_t workout_id);
    std::vector<WorkoutSet> get_sets_for_exercise(int64_t exercise_id, int limit = 100);

    // Templates
    int64_t create_template(const WorkoutTemplate& t);
    std::optional<WorkoutTemplate> get_template(int64_t id);
    std::vector<WorkoutTemplate> list_templates();
    void update_template(const WorkoutTemplate& t);
    void delete_template(int64_t id);

    // Template sets
    int64_t add_template_set(const TemplateSet& s);
    void update_template_set(const TemplateSet& s);
    void delete_template_set(int64_t id);
    std::vector<TemplateSet> get_template_sets(int64_t template_id);

    // Start workout from template
    int64_t start_workout_from_template(int64_t template_id, const std::string& name = "");

    // History
    std::vector<ProgressionPoint> get_progression(int64_t exercise_id, int last_n_sessions = 20);

    // Settings
    std::string get_setting(const std::string& key, const std::string& default_value = "");
    void set_setting(const std::string& key, const std::string& value);
    std::string get_weight_unit();
    void set_weight_unit(const std::string& unit);
    double get_one_rep_max(int64_t exercise_id);
    void set_one_rep_max(int64_t exercise_id, double weight);
    bool is_setup_complete();
    void set_setup_complete(bool complete);

private:
    Database& db_;
};

} // namespace sf
