#include <gtest/gtest.h>
#include "test_helpers.h"
#include <swt/defaults.h>

class DefaultsTest : public ::testing::Test {
protected:
    TestFixture f;
};

TEST_F(DefaultsTest, SeedExercises) {
    sf::seed_default_exercises(f.repo);
    auto exercises = f.repo.list_exercises();
    EXPECT_GE(exercises.size(), 25u);

    auto bench = f.repo.find_exercise_by_name("Bench Press");
    ASSERT_TRUE(bench.has_value());
    EXPECT_EQ(bench->category, "Barbell");
    EXPECT_EQ(bench->muscle_group, "Chest");
}

TEST_F(DefaultsTest, SeedTemplates) {
    sf::seed_default_exercises(f.repo);
    sf::seed_default_templates(f.repo);

    auto templates = f.repo.list_templates();
    EXPECT_EQ(templates.size(), 5u);

    bool found_push = false;
    for (auto& t : templates) {
        if (t.name == "Push Day") found_push = true;
        auto sets = f.repo.get_template_sets(t.id);
        EXPECT_GT(sets.size(), 0u);
    }
    EXPECT_TRUE(found_push);
}

TEST_F(DefaultsTest, SeedTemplatesCreatesExercisesIfMissing) {
    sf::seed_default_templates(f.repo);
    auto exercises = f.repo.list_exercises();
    EXPECT_GT(exercises.size(), 0u);
}

TEST_F(DefaultsTest, SeedTemplateSetsCoverAllExercises) {
    sf::seed_default_exercises(f.repo);
    sf::seed_default_templates(f.repo);

    auto templates = f.repo.list_templates();
    for (auto& t : templates) {
        auto sets = f.repo.get_template_sets(t.id);
        for (auto& s : sets) {
            auto ex = f.repo.get_exercise(s.exercise_id);
            EXPECT_TRUE(ex.has_value()) << "Template set references missing exercise id " << s.exercise_id;
        }
    }
}
