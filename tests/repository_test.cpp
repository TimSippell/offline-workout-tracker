#include <gtest/gtest.h>
#include "test_helpers.h"

class RepositoryTest : public ::testing::Test {
protected:
    TestFixture f;
};

// --- Exercises ---

TEST_F(RepositoryTest, AddAndGetExercise) {
    sf::Exercise ex;
    ex.name = "Bench Press";
    ex.category = "Barbell";
    ex.muscle_group = "Chest";
    int64_t id = f.repo.add_exercise(ex);

    auto got = f.repo.get_exercise(id);
    ASSERT_TRUE(got.has_value());
    EXPECT_EQ(got->name, "Bench Press");
    EXPECT_EQ(got->category, "Barbell");
    EXPECT_EQ(got->muscle_group, "Chest");
}

TEST_F(RepositoryTest, FindExerciseByName) {
    sf::Exercise ex;
    ex.name = "Squat";
    f.repo.add_exercise(ex);

    auto found = f.repo.find_exercise_by_name("squat");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name, "Squat");
}

TEST_F(RepositoryTest, FindExerciseByNameNotFound) {
    EXPECT_FALSE(f.repo.find_exercise_by_name("Nonexistent").has_value());
}

TEST_F(RepositoryTest, ListExercises) {
    sf::Exercise e1; e1.name = "Bench Press"; f.repo.add_exercise(e1);
    sf::Exercise e2; e2.name = "Squat"; f.repo.add_exercise(e2);

    auto list = f.repo.list_exercises();
    EXPECT_EQ(list.size(), 2u);
}

TEST_F(RepositoryTest, ListExercisesFilter) {
    sf::Exercise e1; e1.name = "Bench Press"; f.repo.add_exercise(e1);
    sf::Exercise e2; e2.name = "Squat"; f.repo.add_exercise(e2);

    auto list = f.repo.list_exercises("bench");
    EXPECT_EQ(list.size(), 1u);
    EXPECT_EQ(list[0].name, "Bench Press");
}

TEST_F(RepositoryTest, UpdateExercise) {
    sf::Exercise ex;
    ex.name = "Bench";
    int64_t id = f.repo.add_exercise(ex);

    ex.id = id;
    ex.name = "Bench Press";
    ex.category = "Barbell";
    f.repo.update_exercise(ex);

    auto got = f.repo.get_exercise(id);
    ASSERT_TRUE(got.has_value());
    EXPECT_EQ(got->name, "Bench Press");
    EXPECT_EQ(got->category, "Barbell");
}

TEST_F(RepositoryTest, DeleteExercise) {
    sf::Exercise ex;
    ex.name = "Bench";
    int64_t id = f.repo.add_exercise(ex);

    f.repo.delete_exercise(id);
    EXPECT_FALSE(f.repo.get_exercise(id).has_value());
}

// --- Workouts ---

TEST_F(RepositoryTest, StartAndFinishWorkout) {
    int64_t id = f.repo.start_workout("Test Workout");
    auto active = f.repo.get_active_workout();
    ASSERT_TRUE(active.has_value());
    EXPECT_EQ(active->id, id);

    f.repo.finish_workout(id);
    EXPECT_FALSE(f.repo.get_active_workout().has_value());

    auto finished = f.repo.get_workout(id);
    ASSERT_TRUE(finished.has_value());
    EXPECT_FALSE(finished->finished_at.empty());
}

TEST_F(RepositoryTest, ListWorkoutsOnlyFinished) {
    int64_t w1 = f.repo.start_workout("Finished");
    f.repo.finish_workout(w1);
    f.repo.start_workout("Active");

    auto list = f.repo.list_workouts();
    EXPECT_EQ(list.size(), 1u);
    EXPECT_EQ(list[0].name, "Finished");
}

TEST_F(RepositoryTest, DeleteWorkout) {
    int64_t id = f.repo.start_workout("Delete Me");
    f.repo.delete_workout(id);
    EXPECT_FALSE(f.repo.get_workout(id).has_value());
}

// --- Sets ---

TEST_F(RepositoryTest, AddAndGetSets) {
    sf::Exercise ex; ex.name = "Bench"; int64_t eid = f.repo.add_exercise(ex);
    int64_t wid = f.repo.start_workout("W");

    sf::WorkoutSet s;
    s.workout_id = wid;
    s.exercise_id = eid;
    s.set_order = 1;
    s.reps = 5;
    s.weight = 100.0;
    s.rpe = 8.0;
    s.duration_secs = 30;
    s.rest_secs = 90;
    int64_t sid = f.repo.add_set(s);

    auto sets = f.repo.get_sets_for_workout(wid);
    ASSERT_EQ(sets.size(), 1u);
    EXPECT_EQ(sets[0].id, sid);
    EXPECT_EQ(sets[0].reps, 5);
    EXPECT_DOUBLE_EQ(*sets[0].weight, 100.0);
    EXPECT_DOUBLE_EQ(*sets[0].rpe, 8.0);
    EXPECT_EQ(sets[0].duration_secs, 30);
    EXPECT_EQ(sets[0].rest_secs, 90);
}

TEST_F(RepositoryTest, UpdateSet) {
    sf::Exercise ex; ex.name = "Bench"; int64_t eid = f.repo.add_exercise(ex);
    int64_t wid = f.repo.start_workout("W");

    sf::WorkoutSet s;
    s.workout_id = wid; s.exercise_id = eid; s.set_order = 1; s.reps = 5;
    int64_t sid = f.repo.add_set(s);

    s.id = sid; s.reps = 8; s.weight = 90.0; s.duration_secs = 60; s.rest_secs = 120;
    f.repo.update_set(s);

    auto sets = f.repo.get_sets_for_workout(wid);
    ASSERT_EQ(sets.size(), 1u);
    EXPECT_EQ(sets[0].reps, 8);
    EXPECT_DOUBLE_EQ(*sets[0].weight, 90.0);
    EXPECT_EQ(sets[0].duration_secs, 60);
    EXPECT_EQ(sets[0].rest_secs, 120);
}

TEST_F(RepositoryTest, SetTimingFieldsOptional) {
    sf::Exercise ex; ex.name = "Bench"; int64_t eid = f.repo.add_exercise(ex);
    int64_t wid = f.repo.start_workout("W");

    sf::WorkoutSet s;
    s.workout_id = wid; s.exercise_id = eid; s.set_order = 1; s.reps = 5;
    f.repo.add_set(s);

    auto sets = f.repo.get_sets_for_workout(wid);
    ASSERT_EQ(sets.size(), 1u);
    EXPECT_FALSE(sets[0].duration_secs.has_value());
    EXPECT_FALSE(sets[0].rest_secs.has_value());
}

TEST_F(RepositoryTest, DeleteSet) {
    sf::Exercise ex; ex.name = "Bench"; int64_t eid = f.repo.add_exercise(ex);
    int64_t wid = f.repo.start_workout("W");

    sf::WorkoutSet s; s.workout_id = wid; s.exercise_id = eid; s.set_order = 1;
    int64_t sid = f.repo.add_set(s);
    f.repo.delete_set(sid);

    auto sets = f.repo.get_sets_for_workout(wid);
    EXPECT_TRUE(sets.empty());
}

TEST_F(RepositoryTest, GetSetsForExercise) {
    sf::Exercise ex; ex.name = "Bench"; int64_t eid = f.repo.add_exercise(ex);
    int64_t wid = f.repo.start_workout("W");

    sf::WorkoutSet s1; s1.workout_id = wid; s1.exercise_id = eid; s1.set_order = 1; s1.weight = 100.0; s1.reps = 5;
    sf::WorkoutSet s2; s2.workout_id = wid; s2.exercise_id = eid; s2.set_order = 2; s2.weight = 110.0; s2.reps = 3;
    f.repo.add_set(s1);
    f.repo.add_set(s2);

    auto sets = f.repo.get_sets_for_exercise(eid);
    EXPECT_EQ(sets.size(), 2u);
}

// --- Templates ---

TEST_F(RepositoryTest, CreateAndGetTemplate) {
    sf::WorkoutTemplate t;
    t.name = "Push Day";
    int64_t tid = f.repo.create_template(t);

    auto got = f.repo.get_template(tid);
    ASSERT_TRUE(got.has_value());
    EXPECT_EQ(got->name, "Push Day");
}

TEST_F(RepositoryTest, ListTemplates) {
    sf::WorkoutTemplate t1; t1.name = "Push"; f.repo.create_template(t1);
    sf::WorkoutTemplate t2; t2.name = "Pull"; f.repo.create_template(t2);

    auto list = f.repo.list_templates();
    EXPECT_EQ(list.size(), 2u);
}

TEST_F(RepositoryTest, DeleteTemplate) {
    sf::WorkoutTemplate t; t.name = "Delete Me";
    int64_t tid = f.repo.create_template(t);
    f.repo.delete_template(tid);
    EXPECT_FALSE(f.repo.get_template(tid).has_value());
}

TEST_F(RepositoryTest, TemplateSets) {
    sf::Exercise ex; ex.name = "Bench"; int64_t eid = f.repo.add_exercise(ex);
    sf::WorkoutTemplate t; t.name = "Push"; int64_t tid = f.repo.create_template(t);

    sf::TemplateSet ts;
    ts.template_id = tid; ts.exercise_id = eid; ts.set_order = 1; ts.reps = 8;
    ts.duration_secs = 45; ts.rest_secs = 60;
    int64_t tsid = f.repo.add_template_set(ts);

    auto sets = f.repo.get_template_sets(tid);
    ASSERT_EQ(sets.size(), 1u);
    EXPECT_EQ(sets[0].reps, 8);
    EXPECT_EQ(sets[0].duration_secs, 45);
    EXPECT_EQ(sets[0].rest_secs, 60);

    ts.id = tsid; ts.reps = 10; ts.duration_secs = 50; ts.rest_secs = 90;
    f.repo.update_template_set(ts);
    sets = f.repo.get_template_sets(tid);
    EXPECT_EQ(sets[0].reps, 10);
    EXPECT_EQ(sets[0].duration_secs, 50);
    EXPECT_EQ(sets[0].rest_secs, 90);

    f.repo.delete_template_set(tsid);
    sets = f.repo.get_template_sets(tid);
    EXPECT_TRUE(sets.empty());
}

TEST_F(RepositoryTest, StartWorkoutFromTemplate) {
    sf::Exercise ex; ex.name = "Bench"; int64_t eid = f.repo.add_exercise(ex);
    sf::WorkoutTemplate t; t.name = "Push"; int64_t tid = f.repo.create_template(t);

    sf::TemplateSet ts1; ts1.template_id = tid; ts1.exercise_id = eid; ts1.set_order = 1; ts1.reps = 8; ts1.weight = 80.0; ts1.duration_secs = 30; ts1.rest_secs = 90;
    sf::TemplateSet ts2; ts2.template_id = tid; ts2.exercise_id = eid; ts2.set_order = 2; ts2.reps = 8; ts2.weight = 80.0;
    f.repo.add_template_set(ts1);
    f.repo.add_template_set(ts2);

    int64_t wid = f.repo.start_workout_from_template(tid);
    auto w = f.repo.get_workout(wid);
    ASSERT_TRUE(w.has_value());
    EXPECT_EQ(w->name, "Push");
    EXPECT_EQ(w->sets.size(), 2u);
    EXPECT_EQ(w->sets[0].reps, 8);
    EXPECT_DOUBLE_EQ(*w->sets[0].weight, 80.0);
    EXPECT_EQ(w->sets[0].duration_secs, 30);
    EXPECT_EQ(w->sets[0].rest_secs, 90);
    EXPECT_FALSE(w->sets[1].duration_secs.has_value());
    EXPECT_FALSE(w->sets[1].rest_secs.has_value());
}

TEST_F(RepositoryTest, StartWorkoutFromTemplateCustomName) {
    sf::Exercise ex; ex.name = "Bench"; int64_t eid = f.repo.add_exercise(ex);
    sf::WorkoutTemplate t; t.name = "Push"; int64_t tid = f.repo.create_template(t);

    sf::TemplateSet ts; ts.template_id = tid; ts.exercise_id = eid; ts.set_order = 1;
    f.repo.add_template_set(ts);

    int64_t wid = f.repo.start_workout_from_template(tid, "Monday Push");
    auto w = f.repo.get_workout(wid);
    EXPECT_EQ(w->name, "Monday Push");
}

// --- Settings ---

TEST_F(RepositoryTest, SettingsDefaultValue) {
    EXPECT_EQ(f.repo.get_setting("missing", "default"), "default");
}

TEST_F(RepositoryTest, SetAndGetSetting) {
    f.repo.set_setting("foo", "bar");
    EXPECT_EQ(f.repo.get_setting("foo"), "bar");
}

TEST_F(RepositoryTest, SettingOverwrite) {
    f.repo.set_setting("foo", "bar");
    f.repo.set_setting("foo", "baz");
    EXPECT_EQ(f.repo.get_setting("foo"), "baz");
}

TEST_F(RepositoryTest, WeightUnit) {
    EXPECT_EQ(f.repo.get_weight_unit(), "kg");
    f.repo.set_weight_unit("lbs");
    EXPECT_EQ(f.repo.get_weight_unit(), "lbs");
}

TEST_F(RepositoryTest, OneRepMax) {
    sf::Exercise ex; ex.name = "Bench"; int64_t eid = f.repo.add_exercise(ex);
    EXPECT_DOUBLE_EQ(f.repo.get_one_rep_max(eid), 0.0);

    f.repo.set_one_rep_max(eid, 120.5);
    EXPECT_NEAR(f.repo.get_one_rep_max(eid), 120.5, 0.1);
}

TEST_F(RepositoryTest, SetupComplete) {
    EXPECT_FALSE(f.repo.is_setup_complete());
    f.repo.set_setup_complete(true);
    EXPECT_TRUE(f.repo.is_setup_complete());
    f.repo.set_setup_complete(false);
    EXPECT_FALSE(f.repo.is_setup_complete());
}

// --- Progression ---

TEST_F(RepositoryTest, ProgressionEmpty) {
    sf::Exercise ex; ex.name = "Bench"; int64_t eid = f.repo.add_exercise(ex);
    auto prog = f.repo.get_progression(eid);
    EXPECT_TRUE(prog.empty());
}

TEST_F(RepositoryTest, ProgressionWithData) {
    sf::Exercise ex; ex.name = "Bench"; int64_t eid = f.repo.add_exercise(ex);
    int64_t wid = f.repo.start_workout("W");

    sf::WorkoutSet s; s.workout_id = wid; s.exercise_id = eid; s.set_order = 1; s.weight = 100.0; s.reps = 5;
    f.repo.add_set(s);
    f.repo.finish_workout(wid);

    auto prog = f.repo.get_progression(eid);
    ASSERT_EQ(prog.size(), 1u);
    EXPECT_GT(prog[0].estimated_1rm, 0.0);
    EXPECT_DOUBLE_EQ(prog[0].best_set_weight, 100.0);
    EXPECT_DOUBLE_EQ(prog[0].session_volume, 500.0);
}
