#pragma once

#include "models.h"
#include <vector>

namespace sf {

double estimate_1rm_epley(double weight, int reps);
double estimate_1rm_brzycki(double weight, int reps);
double estimate_1rm(double weight, int reps);
double estimate_1rm_rpe(double weight, int reps, double rpe);

double calculate_volume(const std::vector<WorkoutSet>& sets);
ExerciseStats compute_stats(int64_t exercise_id, const std::vector<WorkoutSet>& sets);

} // namespace sf
