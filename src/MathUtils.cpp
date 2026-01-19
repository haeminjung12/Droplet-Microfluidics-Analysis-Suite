#include "MathUtils.h"

#include <numbers>

namespace MathUtils {

double calculateCircularity(double area, double perimeter) {
    if (!(area > 0.0) || !(perimeter > 0.0)) {
        return 0.0;
    }

    const auto pi = std::numbers::pi_v<double>;
    const auto perimeter_sq = perimeter * perimeter;
    if (!(perimeter_sq > 0.0)) {
        return 0.0;
    }

    return (4.0 * pi * area) / perimeter_sq;
}

double areaFromDiameter(double diameter) {
    if (!(diameter > 0.0)) {
        return 0.0;
    }

    const auto radius = diameter / 2.0;
    return std::numbers::pi_v<double> * radius * radius;
}

double diameterFromArea(double area) {
    if (!(area > 0.0)) {
        return 0.0;
    }

    const auto diameter_sq = (4.0 * area) / std::numbers::pi_v<double>;
    if (!(diameter_sq > 0.0)) {
        return 0.0;
    }

    return std::sqrt(diameter_sq);
}

double aspectRatio(double major_axis_length, double minor_axis_length) {
    if (!(major_axis_length > 0.0) || !(minor_axis_length > 0.0)) {
        return 0.0;
    }

    return major_axis_length / minor_axis_length;
}

} // namespace MathUtils

