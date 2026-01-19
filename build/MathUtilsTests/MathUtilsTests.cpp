#include <cmath>
#include <cstddef>
#include <iostream>
#include <numbers>

#include "MathUtils.h"

namespace {

int failures = 0;

void expectNear(double actual, double expected, double epsilon, const char* name) {
    if (std::isnan(actual) || std::isnan(expected) || std::fabs(actual - expected) > epsilon) {
        std::cerr << "FAIL " << name << ": expected " << expected << " got " << actual << " (eps " << epsilon << ")\n";
        ++failures;
    }
}

void expectDoubleEq(double actual, double expected, const char* name) { expectNear(actual, expected, 0.0, name); }

void expectLt(double lhs, double rhs, const char* name) {
    if (!(lhs < rhs)) {
        std::cerr << "FAIL " << name << ": expected " << lhs << " < " << rhs << "\n";
        ++failures;
    }
}

} // namespace

int main() {
    const double radius = 10.0;
    const double area = std::numbers::pi_v<double> * radius * radius;
    const double perimeter = 2.0 * std::numbers::pi_v<double> * radius;
    expectNear(MathUtils::calculateCircularity(area, perimeter), 1.0, 1e-12, "circularity(circle)=1");

    const double a = 20.0;
    const double b = 5.0;
    const double ellipse_area = std::numbers::pi_v<double> * a * b;
    const double h = std::pow((a - b), 2.0) / std::pow((a + b), 2.0);
    const double ellipse_perimeter = std::numbers::pi_v<double> * (a + b) *
                                     (1.0 + (3.0 * h) / (10.0 + std::sqrt(4.0 - 3.0 * h)));
    const double ellipse_expected =
        (4.0 * std::numbers::pi_v<double> * ellipse_area) / (ellipse_perimeter * ellipse_perimeter);
    expectNear(
        MathUtils::calculateCircularity(ellipse_area, ellipse_perimeter),
        ellipse_expected,
        1e-12,
        "circularity(ellipse) matches formula");
    expectLt(MathUtils::calculateCircularity(ellipse_area, ellipse_perimeter), 1.0, "circularity(ellipse) < 1");

    const double diameter = 12.3;
    const double roundtrip_diameter = MathUtils::diameterFromArea(MathUtils::areaFromDiameter(diameter));
    expectNear(roundtrip_diameter, diameter, 1e-12, "diameterFromArea(areaFromDiameter(d)) round-trip");

    expectDoubleEq(MathUtils::aspectRatio(10.0, 2.0), 5.0, "aspectRatio(10,2)=5");
    expectDoubleEq(MathUtils::aspectRatio(0.0, 2.0), 0.0, "aspectRatio(0,2)=0");
    expectDoubleEq(MathUtils::aspectRatio(10.0, 0.0), 0.0, "aspectRatio(10,0)=0");

    if (failures != 0) {
        return 1;
    }
    std::cout << "OK\n";
    return 0;
}

