#include "owt/defaults.h"

namespace sf {

struct DefaultExercise {
    const char* name;
    const char* category;
    const char* muscle;
    const char* type;
};

static const DefaultExercise default_exercises[] = {
    {"Bench Press",        "Barbell",    "Chest",     "weight"},
    {"Incline Bench Press","Barbell",    "Chest",     "weight"},
    {"Dumbbell Fly",       "Dumbbell",   "Chest",     "weight"},
    {"Squat",              "Barbell",    "Legs",      "weight"},
    {"Front Squat",        "Barbell",    "Legs",      "weight"},
    {"Leg Press",          "Machine",    "Legs",      "weight"},
    {"Romanian Deadlift",  "Barbell",    "Legs",      "weight"},
    {"Leg Curl",           "Machine",    "Legs",      "weight"},
    {"Leg Extension",      "Machine",    "Legs",      "weight"},
    {"Calf Raise",         "Machine",    "Legs",      "weight"},
    {"Deadlift",           "Barbell",    "Back",      "weight"},
    {"Barbell Row",        "Barbell",    "Back",      "weight"},
    {"Pull Up",            "Bodyweight", "Back",      "weight"},
    {"Lat Pulldown",       "Cable",      "Back",      "weight"},
    {"Seated Cable Row",   "Cable",      "Back",      "weight"},
    {"Overhead Press",     "Barbell",    "Shoulders", "weight"},
    {"Lateral Raise",      "Dumbbell",   "Shoulders", "weight"},
    {"Face Pull",          "Cable",      "Shoulders", "weight"},
    {"Bicep Curl",         "Dumbbell",   "Arms",      "weight"},
    {"Barbell Curl",       "Barbell",    "Arms",      "weight"},
    {"Hammer Curl",        "Dumbbell",   "Arms",      "weight"},
    {"Tricep Pushdown",    "Cable",      "Arms",      "weight"},
    {"Skull Crusher",      "Barbell",    "Arms",      "weight"},
    {"Dip",                "Bodyweight", "Arms",      "weight"},
    {"Plank",              "Bodyweight", "Core",      "time"},
    {"Hanging Leg Raise",  "Bodyweight", "Core",      "weight"},
    {"Cable Crunch",       "Cable",      "Core",      "weight"},
    {"Running",            "Cardio",     "Cardio",    "time"},
    {"Cycling",            "Cardio",     "Cardio",    "time"},
    {"Rowing",             "Cardio",     "Cardio",    "time"},
};

void seed_default_exercises(Repository& repo) {
    for (const auto& def : default_exercises) {
        Exercise ex;
        ex.name = def.name;
        ex.category = def.category;
        ex.muscle_group = def.muscle;
        ex.notes = def.type;
        repo.add_exercise(ex);
    }
}

struct DefaultTemplateSet {
    const char* exercise_name;
    int sets;
    int reps;
};

struct DefaultTemplate {
    const char* name;
    const DefaultTemplateSet* exercises;
    int exercise_count;
};

static const DefaultTemplateSet push_sets[] = {
    {"Bench Press", 4, 8}, {"Overhead Press", 3, 10},
    {"Incline Bench Press", 3, 10}, {"Lateral Raise", 3, 15},
    {"Tricep Pushdown", 3, 12},
};

static const DefaultTemplateSet pull_sets[] = {
    {"Deadlift", 3, 5}, {"Barbell Row", 4, 8},
    {"Pull Up", 3, 8}, {"Face Pull", 3, 15},
    {"Bicep Curl", 3, 12},
};

static const DefaultTemplateSet leg_sets[] = {
    {"Squat", 4, 6}, {"Romanian Deadlift", 3, 10},
    {"Leg Press", 3, 12}, {"Leg Curl", 3, 12},
    {"Calf Raise", 4, 15},
};

static const DefaultTemplateSet upper_sets[] = {
    {"Bench Press", 4, 8}, {"Barbell Row", 4, 8},
    {"Overhead Press", 3, 10}, {"Lat Pulldown", 3, 10},
    {"Bicep Curl", 2, 12}, {"Tricep Pushdown", 2, 12},
};

static const DefaultTemplateSet lower_sets[] = {
    {"Squat", 4, 6}, {"Romanian Deadlift", 3, 10},
    {"Leg Press", 3, 12}, {"Leg Extension", 3, 12},
    {"Leg Curl", 3, 12}, {"Calf Raise", 4, 15},
};

static const DefaultTemplate default_templates[] = {
    {"Push Day",    push_sets,  5},
    {"Pull Day",    pull_sets,  5},
    {"Leg Day",     leg_sets,   5},
    {"Upper Body",  upper_sets, 6},
    {"Lower Body",  lower_sets, 6},
};

void seed_default_templates(Repository& repo) {
    auto exercises = repo.list_exercises();

    for (const auto& tmpl : default_templates) {
        WorkoutTemplate wt;
        wt.name = tmpl.name;
        int64_t template_id = repo.create_template(wt);

        int order = 1;
        for (int i = 0; i < tmpl.exercise_count; i++) {
            const auto& def = tmpl.exercises[i];

            int64_t exercise_id = 0;
            auto found = repo.find_exercise_by_name(def.exercise_name);
            if (found) {
                exercise_id = found->id;
            } else {
                Exercise ex;
                ex.name = def.exercise_name;
                ex.notes = "weight";
                exercise_id = repo.add_exercise(ex);
            }

            for (int s = 0; s < def.sets; s++) {
                TemplateSet ts;
                ts.template_id = template_id;
                ts.exercise_id = exercise_id;
                ts.set_order = order++;
                ts.reps = def.reps;
                repo.add_template_set(ts);
            }
        }
    }
}

} // namespace sf
