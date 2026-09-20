import SwiftUI

/// Create and rename share one presentation context, so a single piece of state
/// drives both — stacked `.alert` modifiers leave one of them unreachable.
private enum TemplatesAlert: Identifiable {
    case create
    case rename(WorkoutTemplate)

    var id: String {
        switch self {
        case .create: return "create"
        case .rename(let template): return "rename-\(template.id)"
        }
    }

    var confirmLabel: String {
        switch self {
        case .create: return "Create"
        case .rename: return "Rename"
        }
    }
}

struct TemplatesView: View {
    var onDismiss: () -> Void = {}

    @State private var templates: [WorkoutTemplate] = []
    @State private var path: [Int64] = []
    @State private var alert: TemplatesAlert?
    @State private var nameField = ""

    var body: some View {
        NavigationStack(path: $path) {
            content
                .navigationDestination(for: Int64.self) { id in
                    TemplateBuilderView(templateId: id, showsDoneButton: false)
                }
        }
    }

    private var content: some View {
        List {
            ForEach(templates) { template in
                NavigationLink(value: template.id) {
                    VStack(alignment: .leading, spacing: 2) {
                        Text(template.name).font(.headline)
                        Text("\(template.setCount) sets")
                            .font(.caption)
                            .foregroundStyle(.secondary)
                    }
                }
                .swipeActions(edge: .leading) {
                    Button {
                        nameField = template.name
                        alert = .rename(template)
                    } label: {
                        Label("Rename", systemImage: "pencil")
                    }
                    .tint(.blue)
                }
            }
            .onDelete { indexSet in
                for index in indexSet {
                    OwtBridge.shared.deleteTemplate(id: templates[index].id)
                }
                reload()
            }

            if templates.isEmpty {
                VStack(spacing: 16) {
                    Text("No templates yet. Tap + to create one, or load some recommended templates.")
                        .foregroundStyle(.secondary)
                    Button("Load recommended templates") {
                        OwtBridge.shared.seedDefaultTemplates()
                        reload()
                    }
                    .buttonStyle(.bordered)
                }
                .padding()
            }
        }
        .navigationTitle("Workout Templates")
        .navigationBarTitleDisplayMode(.inline)
        .toolbar {
            ToolbarItem(placement: .cancellationAction) {
                Button("Done") { onDismiss() }
            }
            ToolbarItem(placement: .primaryAction) {
                Button {
                    nameField = ""
                    alert = .create
                } label: {
                    Image(systemName: "plus")
                }
            }
        }
        .onAppear { reload() }
        // Set counts change while the builder is pushed, so refresh on the way back.
        .onChange(of: path) { _ in reload() }
        .alert(alertTitle, isPresented: Binding(
            get: { alert != nil },
            set: { if !$0 { alert = nil } }
        ), presenting: alert) { pending in
            TextField("Template name", text: $nameField)
            Button(pending.confirmLabel) {
                switch pending {
                case .create:
                    let id = OwtBridge.shared.createTemplate(name: nameField)
                    reload()
                    path.append(id)
                case .rename(let template):
                    OwtBridge.shared.updateTemplate(id: template.id, name: nameField, notes: template.notes)
                    reload()
                }
                nameField = ""
            }
            .disabled(nameField.isEmpty)
            Button("Cancel", role: .cancel) { nameField = "" }
        }
    }

    private var alertTitle: String {
        switch alert {
        case .rename: return "Rename Template"
        case .create, .none: return "New Template"
        }
    }

    private func reload() {
        templates = OwtBridge.shared.listTemplates()
    }
}
