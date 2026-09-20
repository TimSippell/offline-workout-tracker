import SwiftUI

enum Tab: String, CaseIterable {
    case exercises = "Exercises"
    case workout = "Workout"
    case history = "History"
    case progress = "Progress"
    case settings = "Settings"

    var icon: String {
        switch self {
        case .exercises: return "list.bullet"
        case .workout: return "figure.strengthtraining.traditional"
        case .history: return "clock"
        case .progress: return "chart.line.uptrend.xyaxis"
        case .settings: return "gearshape"
        }
    }
}

/// Every modal presented from the tab bar shares one presentation context, so
/// they must be driven by a single piece of state — stacking `.sheet` modifiers
/// makes only one of them reachable at a time.
enum RootSheet: Identifiable {
    case setup
    case templates
    case templateBuilder(Int64)

    var id: String {
        switch self {
        case .setup: return "setup"
        case .templates: return "templates"
        case .templateBuilder(let id): return "builder-\(id)"
        }
    }
}

struct ContentView: View {
    @State private var selectedTab: Tab = .workout
    @State private var sheet: RootSheet?

    var body: some View {
        TabView(selection: $selectedTab) {
            NavigationStack {
                ExercisesView(onNavigateToSetup: { sheet = .setup })
            }
            .tabItem { Label(Tab.exercises.rawValue, systemImage: Tab.exercises.icon) }
            .tag(Tab.exercises)

            NavigationStack {
                WorkoutView(
                    onManageTemplates: { sheet = .templates },
                    onEditTemplate: { id in sheet = .templateBuilder(id) }
                )
            }
            .tabItem { Label(Tab.workout.rawValue, systemImage: Tab.workout.icon) }
            .tag(Tab.workout)

            NavigationStack {
                HistoryView()
            }
            .tabItem { Label(Tab.history.rawValue, systemImage: Tab.history.icon) }
            .tag(Tab.history)

            NavigationStack {
                ProgressView()
            }
            .tabItem { Label(Tab.progress.rawValue, systemImage: Tab.progress.icon) }
            .tag(Tab.progress)

            NavigationStack {
                SettingsView(onNavigateToSetup: { sheet = .setup })
            }
            .tabItem { Label(Tab.settings.rawValue, systemImage: Tab.settings.icon) }
            .tag(Tab.settings)
        }
        .sheet(item: $sheet) { destination in
            switch destination {
            case .setup:
                NavigationStack {
                    SetupView(onFinish: { sheet = nil })
                }
            case .templates:
                TemplatesView(onDismiss: { sheet = nil })
            case .templateBuilder(let id):
                NavigationStack {
                    TemplateBuilderView(templateId: id, onDismiss: { sheet = nil })
                }
            }
        }
    }
}
