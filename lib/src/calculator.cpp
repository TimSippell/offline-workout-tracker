#include "owt/calculator.h"
#include <algorithm>

namespace sf {

double estimate_1rm_epley(double weight, int reps) {
    if (reps <= 0) return 0.0;
    if (reps == 1) return weight;
    return weight * (1.0 + reps / 30.0);
}

double estimate_1rm_brzycki(double weight, int reps) {
    if (reps <= 0) return 0.0;
    if (reps == 1) return weight;
    if (reps >= 37) return 0.0;
    return weight * (36.0 / (37.0 - reps));
}

double estimate_1rm(double weight, int reps) {
    if (reps <= 0) return 0.0;
    if (reps == 1) return weight;
    double e = estimate_1rm_epley(weight, reps);
    double b = estimate_1rm_brzycki(weight, reps);
    if (b <= 0.0) return e;
    return (e + b) / 2.0;
}

double estimate_1rm_rpe(double weight, int reps, double rpe) {
    if (rpe <= 0.0 || rpe > 10.0) return estimate_1rm(weight, reps);
    int effective_reps = reps + static_cast<int>(10.0 - rpe);
    return estimate_1rm(weight, effective_reps);
}

double calculate_volume(const std::vector<WorkoutSet>& sets) {
    double total = 0.0;
    for (const auto& s : sets) {
        if (s.weight && s.reps && *s.reps > 0 && *s.weight > 0.0) {
            total += *s.weight * *s.reps;
        }
    }
    return total;
}

ExerciseStats compute_stats(int64_t exercise_id, const std::vector<WorkoutSet>& sets) {
    ExerciseStats stats;
    stats.exercise_id = exercise_id;

    double best_1rm = 0.0;
    double best_weight = 0.0;
    int best_reps = 0;

    for (const auto& s : sets) {
        if (!s.weight || !s.reps) continue;
        double w = *s.weight;
        int r = *s.reps;
        if (w <= 0 || r <= 0) continue;

        double e1rm = s.rpe ? estimate_1rm_rpe(w, r, *s.rpe) : estimate_1rm(w, r);
        if (e1rm > best_1rm) best_1rm = e1rm;
        if (w > best_weight) {
            best_weight = w;
            best_reps = r;
        }
    }

    stats.estimated_1rm = best_1rm;
    stats.best_weight = best_weight;
    stats.best_reps_at_best_weight = best_reps;
    stats.total_volume = calculate_volume(sets);

    return stats;
}

} // namespace sf
