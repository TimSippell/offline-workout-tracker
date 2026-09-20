import SwiftUI

/// Add and edit share one presentation context, so a single piece of state
/// drives both — stacked `.sheet` modifiers leave one of them unreachable.
private enum BuilderSheet: Identifiable {
    case add
    case edit(TemplateSet)

    var id: String {
        switch self {
        case .add: return "add"
        case .edit(let set): return "edit-\(set.id)"
        }
    }
}

struct TemplateBuilderView: View {
    let templateId: Int64
    var showsDoneButton: Bool = true
    var onDismiss: () -> Void = {}

    @State private var sets: [TemplateSet] = []
    @State private var exercises: [Exercise] = []
    @State private var sheet: BuilderSheet?

    var body: some View {
        List {
            if sets.isEmpty {
                Text("No exercises yet. Tap + to add one.")
                    .foregroundStyle(.secondary)
                    .padding()
            }
            ForEach(Array(sets.enumerated()), id: \.element.id) { index, set in
                let exerciseName = exercises.first(where: { $0.id == set.exerciseId })?.name ?? "Unknown"
                TemplateSetRow(
                    exerciseName: exerciseName,
                    set: set,
                    isFirst: index == 0,
                    isLast: index == sets.count - 1,
                    onEdit: { sheet = .edit(set) },
                    onMoveUp: {
                        let prev = sets[index - 1]
                        OwtBridge.shared.swapTemplateSetOrder(idA: set.id, orderA: set.order, idB: prev.id, orderB: prev.order)
                        reload()
                    },
                    onMoveDown: {
                        let next = sets[index + 1]
                        OwtBridge.shared.swapTemplateSetOrder(idA: set.id, orderA: set.order, idB: next.id, orderB: next.order)
                        reload()
                    },
                    onDelete: {
                        OwtBridge.shared.deleteTemplateSet(id: set.id)
                        reload()
                    }
                )
            }
        }
        .navigationTitle("Edit Template")
        .navigationBarTitleDisplayMode(.inline)
        .toolbar {
            if showsDoneButton {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Done") { onDismiss() }
                }
            }
            ToolbarItem(placement: .primaryAction) {
                Button { sheet = .add } label: {
                    Image(systemName: "plus")
                }
            }
        }
        .onAppear {
            exercises = OwtBridge.shared.listExercises()
            reload()
        }
        .sheet(item: $sheet) { destination in
            switch destination {
            case .add:
                AddTemplateSetSheet(exercises: exercises) { exerciseId, numSets, reps, weight, rpe, durationSecs, restSecs in
                    for i in 1...numSets {
                        _ = OwtBridge.shared.addTemplateSet(
                            templateId: templateId, exerciseId: exerciseId,
                            order: sets.count + i, reps: reps,
                            weight: OwtBridge.shared.toStorageWeight(weight),
                            rpe: rpe, durationSecs: durationSecs, restSecs: restSecs
                        )
                    }
                    reload()
                }
            case .edit(let set):
                EditTemplateSetSheet(set: set, exercises: exercises) { reps, weight, rpe, durationSecs, restSecs in
                    OwtBridge.shared.updateTemplateSet(
                        id: set.id, reps: reps,
                        weight: OwtBridge.shared.toStorageWeight(weight),
                        rpe: rpe, durationSecs: durationSecs, restSecs: restSecs
                    )
                    reload()
                }
            }
        }
    }

    private func reload() {
        sets = OwtBridge.shared.getTemplateSets(templateId: templateId)
    }
}

private struct TemplateSetRow: View {
    let exerciseName: String
    let set: TemplateSet
    let isFirst: Bool
    let isLast: Bool
    let onEdit: () -> Void
    let onMoveUp: () -> Void
    let onMoveDown: () -> Void
    let onDelete: () -> Void

    var body: some View {
        let displayWeight = OwtBridge.shared.toDisplayWeight(set.weight)
        let weightUnit = OwtBridge.shared.getWeightUnit()

        HStack {
            VStack(alignment: .leading, spacing: 2) {
                Text(exerciseName).font(.subheadline.weight(.medium))
                Text(formatTemplateSetDetails(set: set, displayWeight: displayWeight, weightUnit: weightUnit))
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
            Spacer()
            VStack(spacing: 0) {
                Button { onMoveUp() } label: {
                    Image(systemName: "chevron.up")
                        .font(.caption)
                }
                .disabled(isFirst)
                Button { onMoveDown() } label: {
                    Image(systemName: "chevron.down")
                        .font(.caption)
                }
                .disabled(isLast)
            }
            .buttonStyle(.borderless)
            Button(role: .destructive) { onDelete() } label: {
                Image(systemName: "trash")
            }
            .buttonStyle(.borderless)
        }
        .contentShape(Rectangle())
        .onTapGesture { onEdit() }
    }
}

private func formatTemplateSetDetails(set: TemplateSet, displayWeight: Double, weightUnit: String) -> String {
    var parts: [String] = []
    if set.reps > 0 { parts.append("\(set.reps) reps") }
    if set.weight > 0 { parts.append(String(format: "%.1f %@", displayWeight, weightUnit)) }
    if set.durationSecs > 0 { parts.append("\(set.durationSecs)s") }
    if set.restSecs > 0 { parts.append("rest \(set.restSecs)s") }
    if set.rpe > 0 { parts.append(String(format: "@RPE %.0f", set.rpe)) }
    return parts.joined(separator: " \u{2022} ")
}

struct AddTemplateSetSheet: View {
    let exercises: [Exercise]
    let onAdd: (Int64, Int, Int, Double, Double, Int, Int) -> Void

    @Environment(\.dismiss) private var dismiss
    @State private var selectedExercise: Exercise?
    @State private var numSets = "3"
    @State private var reps = ""
    @State private var weight = ""
    @State private var duration = ""
    @State private var rest = ""
    @State private var rpe = ""

    var body: some View {
        NavigationStack {
            Form {
                Picker("Exercise", selection: $selectedExercise) {
                    Text("Select exercise").tag(nil as Exercise?)
                    ForEach(exercises) { ex in
                        Text(ex.name).tag(ex as Exercise?)
                    }
                }
                TextField("Number of sets", text: $numSets).keyboardType(.numberPad)
                TextField("Reps per set", text: $reps).keyboardType(.numberPad)
                TextField("Weight (\(OwtBridge.shared.getWeightUnit())) (optional)", text: $weight).keyboardType(.decimalPad)
                TextField("Duration (secs)", text: $duration).keyboardType(.numberPad)
                TextField("Rest (secs)", text: $rest).keyboardType(.numberPad)
                TextField("Target RPE (optional)", text: $rpe).keyboardType(.decimalPad)
            }
            .navigationTitle("Add Exercise")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Add") {
                        guard let ex = selectedExercise else { return }
                        onAdd(
                            ex.id,
                            Int(numSets) ?? 1,
                            Int(reps) ?? 0,
                            Double(weight) ?? 0.0,
                            Double(rpe) ?? 0.0,
                            Int(duration) ?? 0,
                            Int(rest) ?? 0
                        )
                        dismiss()
                    }
                    .disabled(selectedExercise == nil)
                }
            }
        }
    }
}

struct EditTemplateSetSheet: View {
    let set: TemplateSet
    let exercises: [Exercise]
    let onSave: (Int, Double, Double, Int, Int) -> Void

    @Environment(\.dismiss) private var dismiss
    @State private var reps = ""
    @State private var weight = ""
    @State private var duration = ""
    @State private var rest = ""
    @State private var rpe = ""

    var body: some View {
        let exerciseName = exercises.first(where: { $0.id == set.exerciseId })?.name ?? "Unknown"

        NavigationStack {
            Form {
                TextField("Reps", text: $reps).keyboardType(.numberPad)
                TextField("Weight (\(OwtBridge.shared.getWeightUnit()))", text: $weight).keyboardType(.decimalPad)
                TextField("Duration (secs)", text: $duration).keyboardType(.numberPad)
                TextField("Rest (secs)", text: $rest).keyboardType(.numberPad)
                TextField("RPE (optional)", text: $rpe).keyboardType(.decimalPad)
            }
            .navigationTitle(exerciseName)
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Save") {
                        onSave(Int(reps) ?? 0, Double(weight) ?? 0.0, Double(rpe) ?? 0.0, Int(duration) ?? 0, Int(rest) ?? 0)
                        dismiss()
                    }
                }
            }
        }
        .onAppear {
            let displayWeight = OwtBridge.shared.toDisplayWeight(set.weight)
            reps = set.reps > 0 ? "\(set.reps)" : ""
            weight = set.weight > 0 ? String(format: "%.1f", displayWeight) : ""
            duration = set.durationSecs > 0 ? "\(set.durationSecs)" : ""
            rest = set.restSecs > 0 ? "\(set.restSecs)" : ""
            rpe = set.rpe > 0 ? "\(set.rpe)" : ""
        }
    }
}
