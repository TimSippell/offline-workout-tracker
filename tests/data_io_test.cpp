#include <gtest/gtest.h>
#include "test_helpers.h"
#include <swt/data_io.h>

class DataIOTest : public ::testing::Test {
protected:
    TestFixture f;
};

TEST_F(DataIOTest, ExportEmptyDB) {
    auto json = sf::export_to_json(f.repo);
    EXPECT_NE(json.find("\"exercises\":"), std::string::npos);
    EXPECT_NE(json.find("\"workouts\":"), std::string::npos);
    EXPECT_NE(json.find("\"templates\":"), std::string::npos);
}

TEST_F(DataIOTest, ExportWithData) {
    sf::Exercise ex; ex.name = "Bench Press"; ex.category = "Barbell";
    f.repo.add_exercise(ex);

    auto json = sf::export_to_json(f.repo);
    EXPECT_NE(json.find("Bench Press"), std::string::npos);
    EXPECT_NE(json.find("Barbell"), std::string::npos);
}

TEST_F(DataIOTest, ExportEscapesSpecialChars) {
    sf::Exercise ex; ex.name = "Bench \"Press\""; ex.category = "Bar\\bell";
    f.repo.add_exercise(ex);

    auto json = sf::export_to_json(f.repo);
    EXPECT_NE(json.find("Bench \\\"Press\\\""), std::string::npos);
    EXPECT_NE(json.find("Bar\\\\bell"), std::string::npos);
}

TEST_F(DataIOTest, RoundTrip) {
    sf::Exercise ex; ex.name = "Bench"; ex.category = "Barbell"; ex.muscle_group = "Chest"; ex.notes = "weight";
    int64_t eid = f.repo.add_exercise(ex);

    sf::WorkoutTemplate t; t.name = "Push";
    int64_t tid = f.repo.create_template(t);
    sf::TemplateSet ts; ts.template_id = tid; ts.exercise_id = eid; ts.set_order = 1; ts.reps = 8;
    f.repo.add_template_set(ts);

    int64_t wid = f.repo.start_workout("Test");
    sf::WorkoutSet ws; ws.workout_id = wid; ws.exercise_id = eid; ws.set_order = 1; ws.reps = 5; ws.weight = 100.0;
    f.repo.add_set(ws);
    f.repo.finish_workout(wid);

    auto json = sf::export_to_json(f.repo);

    TestFixture f2;
    auto result = sf::import_from_json(f2.repo, json);
    EXPECT_EQ(result.workouts, 1);
    EXPECT_EQ(result.templates, 1);
    EXPECT_EQ(result.sets, 1);

    auto exercises = f2.repo.list_exercises();
    ASSERT_EQ(exercises.size(), 1u);
    EXPECT_EQ(exercises[0].name, "Bench");
    EXPECT_EQ(exercises[0].category, "Barbell");

    auto templates = f2.repo.list_templates();
    ASSERT_EQ(templates.size(), 1u);
    EXPECT_EQ(templates[0].name, "Push");

    auto workouts = f2.repo.list_workouts();
    ASSERT_EQ(workouts.size(), 1u);
}

TEST_F(DataIOTest, PreviewImport) {
    sf::Exercise ex; ex.name = "Bench";
    f.repo.add_exercise(ex);

    std::string json = R"({
        "exercises": [
            {"id": 1, "name": "Bench", "category": "", "muscleGroup": "", "type": "weight"},
            {"id": 2, "name": "Squat", "category": "", "muscleGroup": "", "type": "weight"}
        ],
        "workouts": [
            {"id": 1, "name": "W", "startedAt": "", "finishedAt": "", "sets": [
                {"exerciseId": 1, "order": 1, "reps": 5, "weight": 100, "rpe": 0}
            ]}
        ],
        "templates": []
    })";

    auto summary = sf::preview_import(f.repo, json);
    EXPECT_EQ(summary.existing_exercises, 1);
    EXPECT_EQ(summary.new_exercises, 1);
    EXPECT_EQ(summary.workouts, 1);
    EXPECT_EQ(summary.workout_sets, 1);
}

TEST_F(DataIOTest, ImportDeduplicatesExercises) {
    sf::Exercise ex; ex.name = "Bench";
    f.repo.add_exercise(ex);

    std::string json = R"({
        "exercises": [
            {"id": 1, "name": "Bench", "category": "", "muscleGroup": "", "type": "weight"}
        ],
        "workouts": [],
        "templates": []
    })";

    sf::import_from_json(f.repo, json);
    auto exercises = f.repo.list_exercises();
    EXPECT_EQ(exercises.size(), 1u);
}

TEST_F(DataIOTest, ImportUnknownExerciseIdSkipsSet) {
    std::string json = R"({
        "exercises": [],
        "workouts": [
            {"id": 1, "name": "W", "startedAt": "", "finishedAt": "", "sets": [
                {"exerciseId": 999, "order": 1, "reps": 5, "weight": 100, "rpe": 0}
            ]}
        ],
        "templates": []
    })";

    auto result = sf::import_from_json(f.repo, json);
    EXPECT_EQ(result.workouts, 1);
    EXPECT_EQ(result.sets, 0);
}
