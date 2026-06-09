#include "owt/data_io.h"
#include <ostream>
#include <map>
#include <stdexcept>
#include <cctype>

namespace sf {

namespace {

std::string escape_json(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;
        }
    }
    return out;
}

// Minimal JSON value types for import parsing
enum class JType { Null, String, Number, Bool, Array, Object };

struct JValue {
    JType type = JType::Null;
    std::string str_val;
    double num_val = 0.0;
    bool bool_val = false;
    std::vector<JValue> arr;
    std::map<std::string, JValue> obj;

    const JValue& at(const std::string& key) const {
        static JValue null_val;
        auto it = obj.find(key);
        return it != obj.end() ? it->second : null_val;
    }

    std::string get_string(const std::string& key, const std::string& def = "") const {
        auto& v = at(key);
        return v.type == JType::String ? v.str_val : def;
    }

    double get_number(const std::string& key, double def = 0.0) const {
        auto& v = at(key);
        return v.type == JType::Number ? v.num_val : def;
    }

    int64_t get_int(const std::string& key, int64_t def = 0) const {
        auto& v = at(key);
        return v.type == JType::Number ? static_cast<int64_t>(v.num_val) : def;
    }

    const std::vector<JValue>& get_array(const std::string& key) const {
        static std::vector<JValue> empty;
        auto& v = at(key);
        return v.type == JType::Array ? v.arr : empty;
    }
};

class JParser {
    const std::string& src;
    size_t pos = 0;

    void skip_ws() {
        while (pos < src.size() && std::isspace(static_cast<unsigned char>(src[pos]))) pos++;
    }

    char peek() { skip_ws(); return pos < src.size() ? src[pos] : '\0'; }
    char next() { skip_ws(); return pos < src.size() ? src[pos++] : '\0'; }

    void expect(char c) {
        if (next() != c) throw std::runtime_error(std::string("Expected '") + c + "'");
    }

    std::string parse_string_raw() {
        expect('"');
        std::string result;
        while (pos < src.size() && src[pos] != '"') {
            if (src[pos] == '\\') {
                pos++;
                if (pos >= src.size()) break;
                switch (src[pos]) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    case '/': result += '/'; break;
                    default: result += src[pos];
                }
            } else {
                result += src[pos];
            }
            pos++;
        }
        if (pos < src.size()) pos++; // skip closing quote
        return result;
    }

public:
    explicit JParser(const std::string& s) : src(s) {}

    JValue parse() {
        char c = peek();
        if (c == '{') return parse_object();
        if (c == '[') return parse_array();
        if (c == '"') return parse_string();
        if (c == 't' || c == 'f') return parse_bool();
        if (c == 'n') return parse_null();
        return parse_number();
    }

private:
    JValue parse_object() {
        JValue val;
        val.type = JType::Object;
        expect('{');
        if (peek() != '}') {
            do {
                std::string key = parse_string_raw();
                expect(':');
                val.obj[key] = parse();
            } while (peek() == ',' && (pos++, true));
        }
        expect('}');
        return val;
    }

    JValue parse_array() {
        JValue val;
        val.type = JType::Array;
        expect('[');
        if (peek() != ']') {
            do {
                val.arr.push_back(parse());
            } while (peek() == ',' && (pos++, true));
        }
        expect(']');
        return val;
    }

    JValue parse_string() {
        JValue val;
        val.type = JType::String;
        val.str_val = parse_string_raw();
        return val;
    }

    JValue parse_number() {
        skip_ws();
        size_t start = pos;
        if (pos < src.size() && src[pos] == '-') pos++;
        while (pos < src.size() && (std::isdigit(static_cast<unsigned char>(src[pos])) || src[pos] == '.' || src[pos] == 'e' || src[pos] == 'E' || src[pos] == '+' || src[pos] == '-')) {
            if ((src[pos] == '+' || src[pos] == '-') && pos > start && src[pos-1] != 'e' && src[pos-1] != 'E') break;
            pos++;
        }
        JValue val;
        val.type = JType::Number;
        val.num_val = std::stod(src.substr(start, pos - start));
        return val;
    }

    JValue parse_bool() {
        JValue val;
        val.type = JType::Bool;
        if (src.substr(pos, 4) == "true") { val.bool_val = true; pos += 4; }
        else if (src.substr(pos, 5) == "false") { val.bool_val = false; pos += 5; }
        return val;
    }

    JValue parse_null() {
        if (src.substr(pos, 4) == "null") pos += 4;
        return JValue{};
    }
};

} // namespace

void export_to_json(Repository& repo, std::ostream& out, ExportScope scope) {
    bool include_exercises = scope == ExportScope::ExercisesAndWorkouts || scope == ExportScope::All;
    bool include_workouts = true;
    bool include_templates = scope == ExportScope::All;

    out << "{\n";
    bool needs_comma = false;

    if (include_exercises) {
        out << "  \"exercises\": [\n";
        auto exercises = repo.list_exercises();
        for (size_t i = 0; i < exercises.size(); i++) {
            auto& ex = exercises[i];
            out << "    {\"id\": " << ex.id
                << ", \"name\": \"" << escape_json(ex.name) << "\""
                << ", \"category\": \"" << escape_json(ex.category) << "\""
                << ", \"muscleGroup\": \"" << escape_json(ex.muscle_group) << "\""
                << ", \"type\": \"" << escape_json(ex.notes) << "\""
                << "}";
            if (i + 1 < exercises.size()) out << ",";
            out << "\n";
        }
        out << "  ]";
        needs_comma = true;
    }

    if (include_workouts) {
        if (needs_comma) out << ",";
        out << "\n  \"workouts\": [\n";
        auto workouts = repo.list_workouts(10000, 0);
        for (size_t i = 0; i < workouts.size(); i++) {
            auto& w = workouts[i];
            out << "    {\"id\": " << w.id
                << ", \"name\": \"" << escape_json(w.name) << "\""
                << ", \"startedAt\": \"" << escape_json(w.started_at) << "\""
                << ", \"finishedAt\": \"" << escape_json(w.finished_at) << "\""
                << ", \"sets\": [";

            auto sets = repo.get_sets_for_workout(w.id);
            for (size_t j = 0; j < sets.size(); j++) {
                auto& s = sets[j];
                out << "{\"exerciseId\": " << s.exercise_id
                    << ", \"order\": " << s.set_order
                    << ", \"reps\": " << s.reps.value_or(0)
                    << ", \"weight\": " << s.weight.value_or(0.0)
                    << ", \"rpe\": " << s.rpe.value_or(0.0)
                    << ", \"durationSecs\": " << s.duration_secs.value_or(0)
                    << ", \"restSecs\": " << s.rest_secs.value_or(0)
                    << "}";
                if (j + 1 < sets.size()) out << ", ";
            }
            out << "]}";
            if (i + 1 < workouts.size()) out << ",";
            out << "\n";
        }
        out << "  ]";
        needs_comma = true;
    }

    if (include_templates) {
        if (needs_comma) out << ",";
        out << "\n  \"templates\": [\n";
        auto templates = repo.list_templates();
        for (size_t i = 0; i < templates.size(); i++) {
            auto& t = templates[i];
            out << "    {\"id\": " << t.id
                << ", \"name\": \"" << escape_json(t.name) << "\""
                << ", \"sets\": [";

            auto sets = repo.get_template_sets(t.id);
            for (size_t j = 0; j < sets.size(); j++) {
                auto& s = sets[j];
                out << "{\"exerciseId\": " << s.exercise_id
                    << ", \"order\": " << s.set_order
                    << ", \"reps\": " << s.reps.value_or(0)
                    << ", \"weight\": " << s.weight.value_or(0.0)
                    << ", \"rpe\": " << s.rpe.value_or(0.0)
                    << ", \"durationSecs\": " << s.duration_secs.value_or(0)
                    << ", \"restSecs\": " << s.rest_secs.value_or(0)
                    << "}";
                if (j + 1 < sets.size()) out << ", ";
            }
            out << "]}";
            if (i + 1 < templates.size()) out << ",";
            out << "\n";
        }
        out << "  ]";
    }

    out << "\n}\n";
}

ExportScope detect_import_scope(const std::string& json) {
    JParser parser(json);
    auto root = parser.parse();

    bool has_exercises = root.obj.count("exercises") && !root.get_array("exercises").empty();
    bool has_templates = root.obj.count("templates") && !root.get_array("templates").empty();

    if (has_exercises && has_templates) return ExportScope::All;
    if (has_exercises) return ExportScope::ExercisesAndWorkouts;
    return ExportScope::History;
}

ImportSummary preview_import(Repository& repo, const std::string& json) {
    JParser parser(json);
    auto root = parser.parse();

    auto existing = repo.list_exercises();
    std::map<std::string, int64_t> existing_map;
    for (auto& ex : existing) existing_map[ex.name] = ex.id;

    ImportSummary summary;

    for (auto& ex : root.get_array("exercises")) {
        auto name = ex.get_string("name");
        if (existing_map.count(name)) summary.existing_exercises++;
        else summary.new_exercises++;
    }

    for (auto& wo : root.get_array("workouts")) {
        summary.workouts++;
        summary.workout_sets += static_cast<int>(wo.get_array("sets").size());
    }

    for (auto& t : root.get_array("templates")) {
        summary.templates++;
        summary.template_sets += static_cast<int>(t.get_array("sets").size());
    }

    return summary;
}

ImportResult import_from_json(Repository& repo, const std::string& json) {
    JParser parser(json);
    auto root = parser.parse();

    auto existing = repo.list_exercises();
    std::map<std::string, int64_t> existing_map;
    for (auto& ex : existing) existing_map[ex.name] = ex.id;

    ImportResult result;
    std::map<int64_t, int64_t> exercise_id_map;

    for (auto& ex_val : root.get_array("exercises")) {
        int64_t old_id = ex_val.get_int("id");
        auto name = ex_val.get_string("name");
        auto it = existing_map.find(name);
        if (it != existing_map.end()) {
            exercise_id_map[old_id] = it->second;
        } else {
            Exercise ex;
            ex.name = name;
            ex.category = ex_val.get_string("category");
            ex.muscle_group = ex_val.get_string("muscleGroup");
            ex.notes = ex_val.get_string("type", "weight");
            int64_t new_id = repo.add_exercise(ex);
            exercise_id_map[old_id] = new_id;
            existing_map[name] = new_id;
            result.exercises++;
        }
    }

    for (auto& wo : root.get_array("workouts")) {
        auto name = wo.get_string("name");
        int64_t workout_id = repo.start_workout(name);
        repo.finish_workout(workout_id);

        for (auto& s : wo.get_array("sets")) {
            int64_t old_exercise_id = s.get_int("exerciseId");
            auto it = exercise_id_map.find(old_exercise_id);
            if (it == exercise_id_map.end()) continue;

            WorkoutSet ws;
            ws.workout_id = workout_id;
            ws.exercise_id = it->second;
            ws.set_order = static_cast<int>(s.get_number("order", result.sets + 1));
            int reps = static_cast<int>(s.get_number("reps"));
            if (reps > 0) ws.reps = reps;
            double weight = s.get_number("weight");
            if (weight > 0) ws.weight = weight;
            double rpe = s.get_number("rpe");
            if (rpe > 0) ws.rpe = rpe;
            int dur = static_cast<int>(s.get_number("durationSecs"));
            if (dur > 0) ws.duration_secs = dur;
            int rest = static_cast<int>(s.get_number("restSecs"));
            if (rest > 0) ws.rest_secs = rest;
            repo.add_set(ws);
            result.sets++;
        }
        result.workouts++;
    }

    auto existing_templates = repo.list_templates();
    std::map<std::string, int64_t> existing_template_map;
    for (auto& et : existing_templates) existing_template_map[et.name] = et.id;

    for (auto& t : root.get_array("templates")) {
        auto tname = t.get_string("name");
        auto tmpl_it = existing_template_map.find(tname);
        if (tmpl_it != existing_template_map.end()) continue;

        WorkoutTemplate wt;
        wt.name = tname;
        int64_t template_id = repo.create_template(wt);
        existing_template_map[tname] = template_id;

        for (auto& s : t.get_array("sets")) {
            int64_t old_exercise_id = s.get_int("exerciseId");
            auto it = exercise_id_map.find(old_exercise_id);
            if (it == exercise_id_map.end()) continue;

            TemplateSet ts;
            ts.template_id = template_id;
            ts.exercise_id = it->second;
            ts.set_order = static_cast<int>(s.get_number("order", result.sets + 1));
            int reps = static_cast<int>(s.get_number("reps"));
            if (reps > 0) ts.reps = reps;
            double weight = s.get_number("weight");
            if (weight > 0) ts.weight = weight;
            double rpe = s.get_number("rpe");
            if (rpe > 0) ts.rpe = rpe;
            int dur = static_cast<int>(s.get_number("durationSecs"));
            if (dur > 0) ts.duration_secs = dur;
            int rest = static_cast<int>(s.get_number("restSecs"));
            if (rest > 0) ts.rest_secs = rest;
            repo.add_template_set(ts);
        }
        result.templates++;
    }

    return result;
}

} // namespace sf
