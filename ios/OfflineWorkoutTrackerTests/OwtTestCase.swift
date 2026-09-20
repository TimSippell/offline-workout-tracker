import XCTest
import OwtBridgeC
@testable import OfflineWorkoutTracker

/// Base class that points the C++ core at a throwaway SQLite file so tests
/// never touch the simulator's real Application Support database.
class OwtTestCase: XCTestCase {
    private var dbPath: String!

    override func setUpWithError() throws {
        try super.setUpWithError()
        let dir = FileManager.default.temporaryDirectory
            .appendingPathComponent("owt-tests-\(UUID().uuidString)", isDirectory: true)
        try FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true)
        dbPath = dir.appendingPathComponent("owt.db").path
        owt_init(dbPath)
    }

    override func tearDownWithError() throws {
        owt_close()
        let dir = URL(fileURLWithPath: dbPath).deletingLastPathComponent()
        try? FileManager.default.removeItem(at: dir)
        dbPath = nil
        try super.tearDownWithError()
    }

    /// Creates an exercise and returns its id, failing the test if the insert did not take.
    func makeExercise(name: String = "Bench Press") -> Int64 {
        let id = OwtBridge.shared.addExercise(name: name, category: "strength", muscleGroup: "chest", notes: "")
        XCTAssertGreaterThan(id, 0, "failed to insert exercise")
        return id
    }
}
