#pragma once

#include <string>

namespace sf {

constexpr double KG_TO_LBS = 2.20462;

double kg_to_lbs(double kg);
double lbs_to_kg(double lbs);
double to_display_weight(double stored_kg, const std::string& unit);
double to_storage_weight(double display_value, const std::string& unit);

} // namespace sf
