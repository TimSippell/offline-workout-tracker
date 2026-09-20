import Foundation

/// Serializes the ids of completed sets to a scalar string so the active
/// workout's checked rows survive scene restoration (SceneStorage only holds
/// property-list scalars).
enum CompletedSets {
    static func decode(_ raw: String) -> Set<Int64> {
        raw.isEmpty ? [] : Set(raw.split(separator: ",").compactMap { Int64($0) })
    }

    static func encode(_ ids: Set<Int64>) -> String {
        ids.sorted().map(String.init).joined(separator: ",")
    }

    static func toggle(_ id: Int64, in raw: String) -> String {
        var ids = decode(raw)
        if ids.contains(id) {
            ids.remove(id)
        } else {
            ids.insert(id)
        }
        return encode(ids)
    }
}
