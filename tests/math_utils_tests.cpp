#include <cmath>
#include <numbers>

#include <gtest/gtest.h>

#include "MathUtils.h"

TEST(MathUtils, CalculateCircularityReturnsOneForCircle) {
    const double radius = 10.0;
    const double area = std::numbers::pi_v<double> * radius * radius;
    const double perimeter = 2.0 * std::numbers::pi_v<double> * radius;

    EXPECT_NEAR(MathUtils::calculateCircularity(area, perimeter), 1.0, 1e-12);
}

TEST(MathUtils, CalculateCircularityMatchesEllipseEstimate) {
    const double a = 20.0;
    const double b = 5.0;

    const double area = std::numbers::pi_v<double> * a * b;

    const double h = std::pow((a - b), 2.0) / std::pow((a + b), 2.0);
    const double perimeter = std::numbers::pi_v<double> * (a + b) * (1.0 + (3.0 * h) / (10.0 + std::sqrt(4.0 - 3.0 * h)));

    const double expected = (4.0 * std::numbers::pi_v<double> * area) / (perimeter * perimeter);
    EXPECT_NEAR(MathUtils::calculateCircularity(area, perimeter), expected, 1e-12);
    EXPECT_LT(MathUtils::calculateCircularity(area, perimeter), 1.0);
}

TEST(MathUtils, AreaDiameterRoundTrip) {
    const double diameter = 12.3;
    const double area = MathUtils::areaFromDiameter(diameter);

    EXPECT_NEAR(MathUtils::diameterFromArea(area), diameter, 1e-12);
}

TEST(MathUtils, AspectRatioHandlesBasicCases) {
    EXPECT_DOUBLE_EQ(MathUtils::aspectRatio(10.0, 2.0), 5.0);
    EXPECT_DOUBLE_EQ(MathUtils::aspectRatio(0.0, 2.0), 0.0);
    EXPECT_DOUBLE_EQ(MathUtils::aspectRatio(10.0, 0.0), 0.0);
}

