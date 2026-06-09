#include "owt/converter.h"

namespace sf {

double kg_to_lbs(double kg) { return kg * KG_TO_LBS; }
double lbs_to_kg(double lbs) { return lbs / KG_TO_LBS; }

double to_display_weight(double stored_kg, const std::string& unit) {
    return unit == "lbs" ? kg_to_lbs(stored_kg) : stored_kg;
}

double to_storage_weight(double display_value, const std::string& unit) {
    return unit == "lbs" ? lbs_to_kg(display_value) : display_value;
}

} // namespace sf
