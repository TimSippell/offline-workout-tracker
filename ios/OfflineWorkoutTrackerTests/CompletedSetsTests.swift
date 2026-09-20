import XCTest
@testable import OfflineWorkoutTracker

/// The active workout's checked rows are persisted as a SceneStorage string.
final class CompletedSetsTests: XCTestCase {
    func testDecodeEmptyStringIsEmptySet() {
        XCTAssertEqual(CompletedSets.decode(""), [])
    }

    func testEncodeDecodeRoundTrip() {
        let ids: Set<Int64> = [3, 1, 42]
        XCTAssertEqual(CompletedSets.decode(CompletedSets.encode(ids)), ids)
    }

    func testEncodeIsStableRegardlessOfInsertionOrder() {
        XCTAssertEqual(CompletedSets.encode([42, 1, 3]), CompletedSets.encode([1, 3, 42]))
        XCTAssertEqual(CompletedSets.encode([42, 1, 3]), "1,3,42")
    }

    func testToggleAddsThenRemoves() {
        let added = CompletedSets.toggle(7, in: "")
        XCTAssertEqual(CompletedSets.decode(added), [7])

        let removed = CompletedSets.toggle(7, in: added)
        XCTAssertEqual(CompletedSets.decode(removed), [])
        XCTAssertEqual(removed, "")
    }

    func testToggleLeavesOtherIdsAlone() {
        let raw = CompletedSets.encode([1, 2, 3])
        XCTAssertEqual(CompletedSets.decode(CompletedSets.toggle(2, in: raw)), [1, 3])
    }

    func testDecodeIgnoresGarbageEntries() {
        XCTAssertEqual(CompletedSets.decode("1,,abc,2"), [1, 2])
    }
}
