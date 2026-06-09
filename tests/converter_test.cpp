#include <gtest/gtest.h>
#include <owt/converter.h>

TEST(Converter, KgToLbs) {
    EXPECT_NEAR(sf::kg_to_lbs(1.0), 2.20462, 0.0001);
    EXPECT_NEAR(sf::kg_to_lbs(100.0), 220.462, 0.001);
}

TEST(Converter, LbsToKg) {
    EXPECT_NEAR(sf::lbs_to_kg(2.20462), 1.0, 0.0001);
    EXPECT_NEAR(sf::lbs_to_kg(220.462), 100.0, 0.001);
}

TEST(Converter, RoundTrip) {
    double original = 85.5;
    EXPECT_NEAR(sf::lbs_to_kg(sf::kg_to_lbs(original)), original, 0.0001);
}

TEST(Converter, Zero) {
    EXPECT_DOUBLE_EQ(sf::kg_to_lbs(0.0), 0.0);
    EXPECT_DOUBLE_EQ(sf::lbs_to_kg(0.0), 0.0);
}

TEST(Converter, DisplayWeightKg) {
    EXPECT_DOUBLE_EQ(sf::to_display_weight(100.0, "kg"), 100.0);
}

TEST(Converter, DisplayWeightLbs) {
    EXPECT_NEAR(sf::to_display_weight(100.0, "lbs"), 220.462, 0.001);
}

TEST(Converter, StorageWeightKg) {
    EXPECT_DOUBLE_EQ(sf::to_storage_weight(100.0, "kg"), 100.0);
}

TEST(Converter, StorageWeightLbs) {
    EXPECT_NEAR(sf::to_storage_weight(220.462, "lbs"), 100.0, 0.001);
}

TEST(Converter, UnknownUnitTreatedAsKg) {
    EXPECT_DOUBLE_EQ(sf::to_display_weight(100.0, "stones"), 100.0);
    EXPECT_DOUBLE_EQ(sf::to_storage_weight(100.0, "stones"), 100.0);
}
