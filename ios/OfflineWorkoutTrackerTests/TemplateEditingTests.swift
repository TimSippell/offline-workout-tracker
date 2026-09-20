import XCTest
@testable import OfflineWorkoutTracker

/// Covers the template rename / set editing the iOS app gained to match Android.
final class TemplateEditingTests: OwtTestCase {
    private var bridge: OwtBridge { OwtBridge.shared }

    func testUpdateTemplateRenamesAndKeepsNotes() {
        let id = bridge.createTemplate(name: "Push Day", notes: "heavy")

        bridge.updateTemplate(id: id, name: "Pull Day", notes: "heavy")

        let template = bridge.listTemplates().first { $0.id == id }
        XCTAssertEqual(template?.name, "Pull Day")
        XCTAssertEqual(template?.notes, "heavy")
    }

    func testUpdateTemplateClearsNotesWhenEmpty() {
        let id = bridge.createTemplate(name: "Legs", notes: "squat focus")

        bridge.updateTemplate(id: id, name: "Legs", notes: "")

        XCTAssertEqual(bridge.listTemplates().first { $0.id == id }?.notes, "")
    }

    func testUpdateTemplateSetWritesEveryField() {
        let exerciseId = makeExercise()
        let templateId = bridge.createTemplate(name: "Push Day")
        let setId = bridge.addTemplateSet(
            templateId: templateId, exerciseId: exerciseId, order: 0,
            reps: 5, weight: 60, rpe: 7, durationSecs: 30, restSecs: 90
        )

        bridge.updateTemplateSet(id: setId, reps: 8, weight: 72.5, rpe: 8.5, durationSecs: 45, restSecs: 120)

        let set = bridge.getTemplateSets(templateId: templateId).first { $0.id == setId }
        XCTAssertEqual(set?.reps, 8)
        XCTAssertEqual(set?.weight ?? 0, 72.5, accuracy: 0.001)
        XCTAssertEqual(set?.rpe ?? 0, 8.5, accuracy: 0.001)
        XCTAssertEqual(set?.durationSecs, 45)
        XCTAssertEqual(set?.restSecs, 120)
    }

    /// update_template_set no longer binds exercise_id / set_order, so editing a
    /// set must not move it to another exercise or reshuffle the template.
    func testUpdateTemplateSetPreservesExerciseAndOrder() {
        let bench = makeExercise(name: "Bench Press")
        let row = makeExercise(name: "Barbell Row")
        let templateId = bridge.createTemplate(name: "Upper")
        let benchSet = bridge.addTemplateSet(templateId: templateId, exerciseId: bench, order: 0, reps: 5, weight: 60, rpe: 0)
        let rowSet = bridge.addTemplateSet(templateId: templateId, exerciseId: row, order: 1, reps: 10, weight: 40, rpe: 0)

        bridge.updateTemplateSet(id: rowSet, reps: 12, weight: 45, rpe: 0)

        let sets = bridge.getTemplateSets(templateId: templateId)
        XCTAssertEqual(sets.map(\.id), [benchSet, rowSet])
        XCTAssertEqual(sets.last?.exerciseId, row)
        XCTAssertEqual(sets.last?.order, 1)
        XCTAssertEqual(sets.last?.reps, 12)
    }

    /// The edit sheet sends 0 for fields the user cleared; the bridge treats
    /// non-positive values as "unset" and clears them in storage.
    func testUpdateTemplateSetClearsOptionalFieldsWhenZero() {
        let exerciseId = makeExercise()
        let templateId = bridge.createTemplate(name: "Push Day")
        let setId = bridge.addTemplateSet(
            templateId: templateId, exerciseId: exerciseId, order: 0,
            reps: 5, weight: 60, rpe: 7, durationSecs: 30, restSecs: 90
        )

        bridge.updateTemplateSet(id: setId, reps: 5, weight: 60, rpe: 0, durationSecs: 0, restSecs: 0)

        let set = bridge.getTemplateSets(templateId: templateId).first { $0.id == setId }
        XCTAssertEqual(set?.rpe ?? -1, 0, accuracy: 0.001)
        XCTAssertEqual(set?.durationSecs, 0)
        XCTAssertEqual(set?.restSecs, 0)
        XCTAssertEqual(set?.reps, 5)
    }

    /// The builder converts display units to storage units before saving.
    func testUpdateTemplateSetRoundTripsPoundsThroughStorage() {
        let previousUnit = bridge.getWeightUnit()
        defer { bridge.setWeightUnit(previousUnit) }
        bridge.setWeightUnit("lb")

        let exerciseId = makeExercise()
        let templateId = bridge.createTemplate(name: "Push Day")
        let setId = bridge.addTemplateSet(templateId: templateId, exerciseId: exerciseId, order: 0, reps: 5, weight: 0, rpe: 0)

        bridge.updateTemplateSet(id: setId, reps: 5, weight: bridge.toStorageWeight(225), rpe: 0)

        let set = bridge.getTemplateSets(templateId: templateId).first { $0.id == setId }
        XCTAssertEqual(bridge.toDisplayWeight(set?.weight ?? 0), 225, accuracy: 0.1)
    }

    func testDeleteTemplateRemovesItsSets() {
        let exerciseId = makeExercise()
        let templateId = bridge.createTemplate(name: "Push Day")
        _ = bridge.addTemplateSet(templateId: templateId, exerciseId: exerciseId, order: 0, reps: 5, weight: 60, rpe: 0)

        bridge.deleteTemplate(id: templateId)

        XCTAssertTrue(bridge.listTemplates().contains { $0.id == templateId } == false)
        XCTAssertTrue(bridge.getTemplateSets(templateId: templateId).isEmpty)
    }
}
