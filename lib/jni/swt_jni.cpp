#include <jni.h>
#include <string>
#include <android/log.h>
#include "swt/swt.h"

#define TAG "SWT-JNI"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static sf::Database* g_db = nullptr;
static sf::Repository* g_repo = nullptr;

extern "C" {

JNIEXPORT void JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeInit(JNIEnv* env, jobject, jstring dbPath) {
    const char* path = env->GetStringUTFChars(dbPath, nullptr);
    try {
        g_db = new sf::Database(path);
        g_db->migrate();
        g_repo = new sf::Repository(*g_db);
    } catch (const std::exception& e) {
        LOGE("Init failed: %s", e.what());
    }
    env->ReleaseStringUTFChars(dbPath, path);
}

JNIEXPORT void JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeClose(JNIEnv*, jobject) {
    delete g_repo;
    delete g_db;
    g_repo = nullptr;
    g_db = nullptr;
}

// --- Exercises ---

static jobject makeExercise(JNIEnv* env, const sf::Exercise& ex) {
    jclass cls = env->FindClass("com/timsippell/swt/bridge/SwtBridge$Exercise");
    jmethodID ctor = env->GetMethodID(cls, "<init>",
        "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    return env->NewObject(cls, ctor,
        (jlong)ex.id,
        env->NewStringUTF(ex.name.c_str()),
        env->NewStringUTF(ex.category.c_str()),
        env->NewStringUTF(ex.muscle_group.c_str()),
        env->NewStringUTF(ex.notes.c_str()));
}

JNIEXPORT jlong JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeAddExercise(JNIEnv* env, jobject,
        jstring name, jstring category, jstring muscleGroup, jstring notes) {
    if (!g_repo) return -1;
    sf::Exercise ex;
    ex.name = env->GetStringUTFChars(name, nullptr);
    ex.category = env->GetStringUTFChars(category, nullptr);
    ex.muscle_group = env->GetStringUTFChars(muscleGroup, nullptr);
    ex.notes = env->GetStringUTFChars(notes, nullptr);
    return g_repo->add_exercise(ex);
}

JNIEXPORT jobjectArray JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeListExercises(JNIEnv* env, jobject, jstring filter) {
    if (!g_repo) return nullptr;
    const char* f = env->GetStringUTFChars(filter, nullptr);
    auto exercises = g_repo->list_exercises(f);
    env->ReleaseStringUTFChars(filter, f);

    jclass cls = env->FindClass("com/timsippell/swt/bridge/SwtBridge$Exercise");
    jobjectArray arr = env->NewObjectArray(exercises.size(), cls, nullptr);
    for (size_t i = 0; i < exercises.size(); i++) {
        env->SetObjectArrayElement(arr, i, makeExercise(env, exercises[i]));
    }
    return arr;
}

JNIEXPORT void JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeDeleteExercise(JNIEnv*, jobject, jlong id) {
    if (g_repo) g_repo->delete_exercise(id);
}

// --- Workouts ---

JNIEXPORT jlong JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeStartWorkout(JNIEnv* env, jobject, jstring name) {
    if (!g_repo) return -1;
    const char* n = env->GetStringUTFChars(name, nullptr);
    auto id = g_repo->start_workout(n);
    env->ReleaseStringUTFChars(name, n);
    return id;
}

JNIEXPORT void JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeFinishWorkout(JNIEnv*, jobject, jlong id) {
    if (g_repo) g_repo->finish_workout(id);
}

static jobject makeWorkout(JNIEnv* env, const sf::Workout& w) {
    jclass cls = env->FindClass("com/timsippell/swt/bridge/SwtBridge$Workout");
    jmethodID ctor = env->GetMethodID(cls, "<init>",
        "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V");
    return env->NewObject(cls, ctor,
        (jlong)w.id,
        env->NewStringUTF(w.name.c_str()),
        env->NewStringUTF(w.started_at.c_str()),
        env->NewStringUTF(w.finished_at.c_str()),
        env->NewStringUTF(w.notes.c_str()),
        (jint)w.sets.size());
}

JNIEXPORT jobject JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeGetActiveWorkout(JNIEnv* env, jobject) {
    if (!g_repo) return nullptr;
    auto w = g_repo->get_active_workout();
    if (!w) return nullptr;
    return makeWorkout(env, *w);
}

JNIEXPORT jobjectArray JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeListWorkouts(JNIEnv* env, jobject, jint limit, jint offset) {
    if (!g_repo) return nullptr;
    auto workouts = g_repo->list_workouts(limit, offset);

    jclass cls = env->FindClass("com/timsippell/swt/bridge/SwtBridge$Workout");
    jobjectArray arr = env->NewObjectArray(workouts.size(), cls, nullptr);
    for (size_t i = 0; i < workouts.size(); i++) {
        env->SetObjectArrayElement(arr, i, makeWorkout(env, workouts[i]));
    }
    return arr;
}

JNIEXPORT void JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeDeleteWorkout(JNIEnv*, jobject, jlong id) {
    if (g_repo) g_repo->delete_workout(id);
}

// --- Sets ---

JNIEXPORT jlong JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeAddSet(JNIEnv*, jobject,
        jlong workoutId, jlong exerciseId, jint order, jint reps, jdouble weight, jdouble rpe) {
    if (!g_repo) return -1;
    sf::WorkoutSet s;
    s.workout_id = workoutId;
    s.exercise_id = exerciseId;
    s.set_order = order;
    if (reps > 0) s.reps = reps;
    if (weight > 0) s.weight = weight;
    if (rpe > 0) s.rpe = rpe;
    return g_repo->add_set(s);
}

static jobject makeWorkoutSet(JNIEnv* env, const sf::WorkoutSet& s) {
    jclass cls = env->FindClass("com/timsippell/swt/bridge/SwtBridge$WorkoutSet");
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(JJJIIDD)V");
    return env->NewObject(cls, ctor,
        (jlong)s.id, (jlong)s.workout_id, (jlong)s.exercise_id,
        (jint)s.set_order, (jint)s.reps.value_or(0),
        (jdouble)s.weight.value_or(0.0), (jdouble)s.rpe.value_or(0.0));
}

JNIEXPORT jobjectArray JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeGetSetsForWorkout(JNIEnv* env, jobject, jlong workoutId) {
    if (!g_repo) return nullptr;
    auto sets = g_repo->get_sets_for_workout(workoutId);

    jclass cls = env->FindClass("com/timsippell/swt/bridge/SwtBridge$WorkoutSet");
    jobjectArray arr = env->NewObjectArray(sets.size(), cls, nullptr);
    for (size_t i = 0; i < sets.size(); i++) {
        env->SetObjectArrayElement(arr, i, makeWorkoutSet(env, sets[i]));
    }
    return arr;
}

JNIEXPORT void JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeDeleteSet(JNIEnv*, jobject, jlong id) {
    if (g_repo) g_repo->delete_set(id);
}

// --- Stats ---

JNIEXPORT jobject JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeGetStats(JNIEnv* env, jobject, jlong exerciseId) {
    if (!g_repo) return nullptr;
    auto sets = g_repo->get_sets_for_exercise(exerciseId);
    auto stats = sf::compute_stats(exerciseId, sets);

    jclass cls = env->FindClass("com/timsippell/swt/bridge/SwtBridge$ExerciseStats");
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(JDDDI)V");
    return env->NewObject(cls, ctor,
        (jlong)stats.exercise_id,
        stats.estimated_1rm, stats.best_weight,
        stats.total_volume, stats.session_count);
}

JNIEXPORT jobjectArray JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeGetProgression(JNIEnv* env, jobject, jlong exerciseId, jint sessions) {
    if (!g_repo) return nullptr;
    auto points = g_repo->get_progression(exerciseId, sessions);

    jclass cls = env->FindClass("com/timsippell/swt/bridge/SwtBridge$ProgressionPoint");
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Ljava/lang/String;DDD)V");
    jobjectArray arr = env->NewObjectArray(points.size(), cls, nullptr);
    for (size_t i = 0; i < points.size(); i++) {
        jobject obj = env->NewObject(cls, ctor,
            env->NewStringUTF(points[i].date.c_str()),
            points[i].estimated_1rm, points[i].best_set_weight, points[i].session_volume);
        env->SetObjectArrayElement(arr, i, obj);
    }
    return arr;
}

// --- Templates ---

static jobject makeWorkoutTemplate(JNIEnv* env, const sf::WorkoutTemplate& t) {
    jclass cls = env->FindClass("com/timsippell/swt/bridge/SwtBridge$WorkoutTemplate");
    jmethodID ctor = env->GetMethodID(cls, "<init>",
        "(JLjava/lang/String;Ljava/lang/String;I)V");
    return env->NewObject(cls, ctor,
        (jlong)t.id,
        env->NewStringUTF(t.name.c_str()),
        env->NewStringUTF(t.notes.c_str()),
        (jint)t.sets.size());
}

static jobject makeTemplateSet(JNIEnv* env, const sf::TemplateSet& s) {
    jclass cls = env->FindClass("com/timsippell/swt/bridge/SwtBridge$TemplateSet");
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(JJJIIDD)V");
    return env->NewObject(cls, ctor,
        (jlong)s.id, (jlong)s.template_id, (jlong)s.exercise_id,
        (jint)s.set_order, (jint)s.reps.value_or(0),
        (jdouble)s.weight.value_or(0.0), (jdouble)s.rpe.value_or(0.0));
}

JNIEXPORT jlong JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeCreateTemplate(JNIEnv* env, jobject,
        jstring name, jstring notes) {
    if (!g_repo) return -1;
    sf::WorkoutTemplate t;
    const char* n = env->GetStringUTFChars(name, nullptr);
    const char* nt = env->GetStringUTFChars(notes, nullptr);
    t.name = n;
    t.notes = nt;
    env->ReleaseStringUTFChars(name, n);
    env->ReleaseStringUTFChars(notes, nt);
    return g_repo->create_template(t);
}

JNIEXPORT jobjectArray JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeListTemplates(JNIEnv* env, jobject) {
    if (!g_repo) return nullptr;
    auto templates = g_repo->list_templates();

    jclass cls = env->FindClass("com/timsippell/swt/bridge/SwtBridge$WorkoutTemplate");
    jobjectArray arr = env->NewObjectArray(templates.size(), cls, nullptr);
    for (size_t i = 0; i < templates.size(); i++) {
        env->SetObjectArrayElement(arr, i, makeWorkoutTemplate(env, templates[i]));
    }
    return arr;
}

JNIEXPORT jobjectArray JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeGetTemplateSets(JNIEnv* env, jobject, jlong templateId) {
    if (!g_repo) return nullptr;
    auto sets = g_repo->get_template_sets(templateId);

    jclass cls = env->FindClass("com/timsippell/swt/bridge/SwtBridge$TemplateSet");
    jobjectArray arr = env->NewObjectArray(sets.size(), cls, nullptr);
    for (size_t i = 0; i < sets.size(); i++) {
        env->SetObjectArrayElement(arr, i, makeTemplateSet(env, sets[i]));
    }
    return arr;
}

JNIEXPORT void JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeDeleteTemplate(JNIEnv*, jobject, jlong id) {
    if (g_repo) g_repo->delete_template(id);
}

JNIEXPORT jlong JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeAddTemplateSet(JNIEnv*, jobject,
        jlong templateId, jlong exerciseId, jint order, jint reps, jdouble weight, jdouble rpe) {
    if (!g_repo) return -1;
    sf::TemplateSet s;
    s.template_id = templateId;
    s.exercise_id = exerciseId;
    s.set_order = order;
    if (reps > 0) s.reps = reps;
    if (weight > 0) s.weight = weight;
    if (rpe > 0) s.rpe = rpe;
    return g_repo->add_template_set(s);
}

JNIEXPORT void JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeDeleteTemplateSet(JNIEnv*, jobject, jlong id) {
    if (g_repo) g_repo->delete_template_set(id);
}

JNIEXPORT jlong JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeStartWorkoutFromTemplate(JNIEnv* env, jobject,
        jlong templateId, jstring name) {
    if (!g_repo) return -1;
    const char* n = env->GetStringUTFChars(name, nullptr);
    auto id = g_repo->start_workout_from_template(templateId, n);
    env->ReleaseStringUTFChars(name, n);
    return id;
}

JNIEXPORT void JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeUpdateSet(JNIEnv*, jobject,
        jlong id, jint reps, jdouble weight, jdouble rpe) {
    if (!g_repo) return;
    sf::WorkoutSet s;
    s.id = id;
    if (reps > 0) s.reps = reps;
    if (weight > 0) s.weight = weight;
    if (rpe > 0) s.rpe = rpe;
    g_repo->update_set(s);
}

JNIEXPORT void JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeUpdateExercise(JNIEnv* env, jobject,
        jlong id, jstring name, jstring category, jstring muscleGroup, jstring notes) {
    if (!g_repo) return;
    const char* n = env->GetStringUTFChars(name, nullptr);
    const char* c = env->GetStringUTFChars(category, nullptr);
    const char* m = env->GetStringUTFChars(muscleGroup, nullptr);
    const char* nt = env->GetStringUTFChars(notes, nullptr);
    sf::Exercise ex;
    ex.id = id;
    ex.name = n;
    ex.category = c;
    ex.muscle_group = m;
    ex.notes = nt;
    g_repo->update_exercise(ex);
    env->ReleaseStringUTFChars(name, n);
    env->ReleaseStringUTFChars(category, c);
    env->ReleaseStringUTFChars(muscleGroup, m);
    env->ReleaseStringUTFChars(notes, nt);
}

// --- Weight Conversion ---

JNIEXPORT jdouble JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeToDisplayWeight(JNIEnv* env, jobject,
        jdouble storedKg, jstring unit) {
    const char* u = env->GetStringUTFChars(unit, nullptr);
    double result = sf::to_display_weight(storedKg, u);
    env->ReleaseStringUTFChars(unit, u);
    return result;
}

JNIEXPORT jdouble JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeToStorageWeight(JNIEnv* env, jobject,
        jdouble displayValue, jstring unit) {
    const char* u = env->GetStringUTFChars(unit, nullptr);
    double result = sf::to_storage_weight(displayValue, u);
    env->ReleaseStringUTFChars(unit, u);
    return result;
}

// --- Settings ---

JNIEXPORT jstring JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeGetWeightUnit(JNIEnv* env, jobject) {
    if (!g_repo) return env->NewStringUTF("kg");
    return env->NewStringUTF(g_repo->get_weight_unit().c_str());
}

JNIEXPORT void JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeSetWeightUnit(JNIEnv* env, jobject, jstring unit) {
    if (!g_repo) return;
    const char* u = env->GetStringUTFChars(unit, nullptr);
    g_repo->set_weight_unit(u);
    env->ReleaseStringUTFChars(unit, u);
}

JNIEXPORT jdouble JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeGetOneRepMax(JNIEnv*, jobject, jlong exerciseId) {
    if (!g_repo) return 0.0;
    return g_repo->get_one_rep_max(exerciseId);
}

JNIEXPORT void JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeSetOneRepMax(JNIEnv*, jobject,
        jlong exerciseId, jdouble weight) {
    if (g_repo) g_repo->set_one_rep_max(exerciseId, weight);
}

JNIEXPORT jboolean JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeIsSetupComplete(JNIEnv*, jobject) {
    if (!g_repo) return JNI_FALSE;
    return g_repo->is_setup_complete() ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeSetSetupComplete(JNIEnv*, jobject, jboolean complete) {
    if (g_repo) g_repo->set_setup_complete(complete == JNI_TRUE);
}

// --- Defaults ---

JNIEXPORT void JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeSeedDefaultExercises(JNIEnv*, jobject) {
    if (g_repo) sf::seed_default_exercises(*g_repo);
}

JNIEXPORT void JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeSeedDefaultTemplates(JNIEnv*, jobject) {
    if (g_repo) sf::seed_default_templates(*g_repo);
}

// --- Export/Import ---

JNIEXPORT jstring JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeExportToJson(JNIEnv* env, jobject) {
    if (!g_repo) return env->NewStringUTF("{}");
    return env->NewStringUTF(sf::export_to_json(*g_repo).c_str());
}

JNIEXPORT jobject JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativePreviewImport(JNIEnv* env, jobject, jstring json) {
    if (!g_repo) return nullptr;
    const char* j = env->GetStringUTFChars(json, nullptr);
    auto summary = sf::preview_import(*g_repo, j);
    env->ReleaseStringUTFChars(json, j);

    jclass cls = env->FindClass("com/timsippell/swt/bridge/SwtBridge$ImportSummary");
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(IIIIII)V");
    return env->NewObject(cls, ctor,
        summary.new_exercises, summary.existing_exercises,
        summary.workouts, summary.workout_sets,
        summary.templates, summary.template_sets);
}

JNIEXPORT jstring JNICALL
Java_com_timsippell_swt_bridge_SwtBridge_nativeImportFromJson(JNIEnv* env, jobject, jstring json) {
    if (!g_repo) return env->NewStringUTF("Error: not initialized");
    const char* j = env->GetStringUTFChars(json, nullptr);
    try {
        auto result = sf::import_from_json(*g_repo, j);
        env->ReleaseStringUTFChars(json, j);
        std::string msg = "Imported " + std::to_string(result.workouts) + " workouts, "
            + std::to_string(result.templates) + " templates, "
            + std::to_string(result.sets) + " sets";
        return env->NewStringUTF(msg.c_str());
    } catch (const std::exception& e) {
        env->ReleaseStringUTFChars(json, j);
        std::string msg = "Import failed: " + std::string(e.what());
        return env->NewStringUTF(msg.c_str());
    }
}

} // extern "C"
