#include <cmath>
#include <cstddef>
#include <iostream>

#include "TimeUtils.h"

namespace {

int failures = 0;

void expectDoubleEq(double actual, double expected, const char* name) {
    if (std::isnan(actual) || std::isnan(expected) || actual != expected) {
        std::cerr << "FAIL " << name << ": expected " << expected << " got " << actual << "\n";
        ++failures;
    }
}

} // namespace

int main() {
    expectDoubleEq(TimeUtils::inferredTimestamp(100, 50.0), 2.0, "inferredTimestamp(100, 50.0)");
    expectDoubleEq(TimeUtils::inferredTimestamp(100, 0.0), 0.0, "inferredTimestamp(100, 0.0)");
    expectDoubleEq(TimeUtils::inferredTimestamp(100, -25.0), 0.0, "inferredTimestamp(100, -25.0)");
    expectDoubleEq(TimeUtils::inferredTimestamp(0, 23.3), 0.0, "inferredTimestamp(0, 23.3)");

    if (failures != 0) {
        return 1;
    }
    std::cout << "OK\n";
    return 0;
}

