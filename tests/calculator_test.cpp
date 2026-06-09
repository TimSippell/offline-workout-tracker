#include <gtest/gtest.h>
#include <owt/calculator.h>

TEST(Estimate1RM, EpleyReturnsWeightForSingleRep) {
    EXPECT_DOUBLE_EQ(sf::estimate_1rm_epley(100.0, 1), 100.0);
}

TEST(Estimate1RM, EpleyZeroReps) {
    EXPECT_DOUBLE_EQ(sf::estimate_1rm_epley(100.0, 0), 0.0);
}

TEST(Estimate1RM, EpleyNegativeReps) {
    EXPECT_DOUBLE_EQ(sf::estimate_1rm_epley(100.0, -1), 0.0);
}

TEST(Estimate1RM, EpleyFormula) {
    // 100 * (1 + 5/30) = 116.666...
    EXPECT_NEAR(sf::estimate_1rm_epley(100.0, 5), 116.6667, 0.001);
}

TEST(Estimate1RM, BrzyckiReturnsWeightForSingleRep) {
    EXPECT_DOUBLE_EQ(sf::estimate_1rm_brzycki(100.0, 1), 100.0);
}

TEST(Estimate1RM, BrzyckiZeroReps) {
    EXPECT_DOUBLE_EQ(sf::estimate_1rm_brzycki(100.0, 0), 0.0);
}

TEST(Estimate1RM, BrzyckiHighReps) {
    EXPECT_DOUBLE_EQ(sf::estimate_1rm_brzycki(100.0, 37), 0.0);
    EXPECT_DOUBLE_EQ(sf::estimate_1rm_brzycki(100.0, 40), 0.0);
}

TEST(Estimate1RM, BrzyckiFormula) {
    // 100 * (36 / (37 - 5)) = 112.5
    EXPECT_DOUBLE_EQ(sf::estimate_1rm_brzycki(100.0, 5), 112.5);
}

TEST(Estimate1RM, CombinedAveragesEpleyAndBrzycki) {
    double e = sf::estimate_1rm_epley(100.0, 5);
    double b = sf::estimate_1rm_brzycki(100.0, 5);
    EXPECT_NEAR(sf::estimate_1rm(100.0, 5), (e + b) / 2.0, 0.001);
}

TEST(Estimate1RM, CombinedSingleRep) {
    EXPECT_DOUBLE_EQ(sf::estimate_1rm(100.0, 1), 100.0);
}

TEST(Estimate1RM, CombinedFallsBackToEpleyWhenBrzyckiZero) {
    double result = sf::estimate_1rm(100.0, 37);
    EXPECT_DOUBLE_EQ(result, sf::estimate_1rm_epley(100.0, 37));
}

TEST(Estimate1RM, RpeAdjustsEffectiveReps) {
    // RPE 8 -> effective_reps = 5 + (10-8) = 7
    double expected = sf::estimate_1rm(100.0, 7);
    EXPECT_NEAR(sf::estimate_1rm_rpe(100.0, 5, 8.0), expected, 0.001);
}

TEST(Estimate1RM, RpeMaxEffort) {
    // RPE 10 -> effective_reps = 5 + 0 = 5
    double expected = sf::estimate_1rm(100.0, 5);
    EXPECT_NEAR(sf::estimate_1rm_rpe(100.0, 5, 10.0), expected, 0.001);
}

TEST(Estimate1RM, RpeInvalidFallsBack) {
    double expected = sf::estimate_1rm(100.0, 5);
    EXPECT_NEAR(sf::estimate_1rm_rpe(100.0, 5, 0.0), expected, 0.001);
    EXPECT_NEAR(sf::estimate_1rm_rpe(100.0, 5, 11.0), expected, 0.001);
}

TEST(Volume, CalculatesWeightTimesReps) {
    std::vector<sf::WorkoutSet> sets;
    sf::WorkoutSet s1; s1.weight = 100.0; s1.reps = 5;
    sf::WorkoutSet s2; s2.weight = 80.0; s2.reps = 10;
    sets = {s1, s2};
    EXPECT_DOUBLE_EQ(sf::calculate_volume(sets), 1300.0);
}

TEST(Volume, SkipsMissingFields) {
    std::vector<sf::WorkoutSet> sets;
    sf::WorkoutSet s1; s1.weight = 100.0;
    sf::WorkoutSet s2; s2.reps = 10;
    sf::WorkoutSet s3;
    sets = {s1, s2, s3};
    EXPECT_DOUBLE_EQ(sf::calculate_volume(sets), 0.0);
}

TEST(Volume, SkipsZeroValues) {
    std::vector<sf::WorkoutSet> sets;
    sf::WorkoutSet s1; s1.weight = 0.0; s1.reps = 5;
    sf::WorkoutSet s2; s2.weight = 100.0; s2.reps = 0;
    sets = {s1, s2};
    EXPECT_DOUBLE_EQ(sf::calculate_volume(sets), 0.0);
}

TEST(Volume, EmptySets) {
    std::vector<sf::WorkoutSet> sets;
    EXPECT_DOUBLE_EQ(sf::calculate_volume(sets), 0.0);
}

TEST(Stats, ComputesBestWeight) {
    std::vector<sf::WorkoutSet> sets;
    sf::WorkoutSet s1; s1.weight = 100.0; s1.reps = 5;
    sf::WorkoutSet s2; s2.weight = 120.0; s2.reps = 3;
    sets = {s1, s2};

    auto stats = sf::compute_stats(1, sets);
    EXPECT_DOUBLE_EQ(stats.best_weight, 120.0);
    EXPECT_EQ(stats.best_reps_at_best_weight, 3);
}

TEST(Stats, TracksBest1RM) {
    std::vector<sf::WorkoutSet> sets;
    sf::WorkoutSet s1; s1.weight = 100.0; s1.reps = 10;
    sf::WorkoutSet s2; s2.weight = 130.0; s2.reps = 1;
    sets = {s1, s2};

    auto stats = sf::compute_stats(1, sets);
    EXPECT_GT(stats.estimated_1rm, 130.0);
}

TEST(Stats, UsesRpeWhenPresent) {
    std::vector<sf::WorkoutSet> sets;
    sf::WorkoutSet s1; s1.weight = 100.0; s1.reps = 5; s1.rpe = 8.0;
    sets = {s1};

    auto stats = sf::compute_stats(1, sets);
    double expected = sf::estimate_1rm_rpe(100.0, 5, 8.0);
    EXPECT_NEAR(stats.estimated_1rm, expected, 0.001);
}

TEST(Stats, EmptySets) {
    auto stats = sf::compute_stats(1, {});
    EXPECT_DOUBLE_EQ(stats.estimated_1rm, 0.0);
    EXPECT_DOUBLE_EQ(stats.best_weight, 0.0);
    EXPECT_DOUBLE_EQ(stats.total_volume, 0.0);
}
