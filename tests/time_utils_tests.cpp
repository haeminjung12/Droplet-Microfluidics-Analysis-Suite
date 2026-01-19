#include <cstddef>

#include <gtest/gtest.h>

#include "TimeUtils.h"

TEST(TimeUtils, InferredTimestampComputesSeconds) {
    EXPECT_DOUBLE_EQ(TimeUtils::inferredTimestamp(100, 50.0), 2.0);
}

TEST(TimeUtils, InferredTimestampWithZeroFpsReturnsZero) {
    EXPECT_DOUBLE_EQ(TimeUtils::inferredTimestamp(100, 0.0), 0.0);
}

TEST(TimeUtils, InferredTimestampWithNegativeFpsReturnsZero) {
    EXPECT_DOUBLE_EQ(TimeUtils::inferredTimestamp(100, -25.0), 0.0);
}

TEST(TimeUtils, InferredTimestampAtZeroFrameIsZero) {
    EXPECT_DOUBLE_EQ(TimeUtils::inferredTimestamp(0, 23.3), 0.0);
}

