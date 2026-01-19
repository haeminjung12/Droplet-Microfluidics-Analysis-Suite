#pragma once

#include <cmath>

namespace MathUtils {
    // Spec reference: Docs/TECHSPEC_SPLIT/03_functional_requirements.md (core algorithm geometry helpers)

    double calculateCircularity(double area, double perimeter);
    double areaFromDiameter(double diameter);
    double diameterFromArea(double area);
    double aspectRatio(double major_axis_length, double minor_axis_length);
}

