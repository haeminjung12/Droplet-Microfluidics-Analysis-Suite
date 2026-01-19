#include <gtest/gtest.h>

#include "ProgressCallback.h"

#include <cstddef>
#include <string>

namespace {
void runDummyWork(const ProgressCallback& callback) { callback(3, 10, "working"); }
}  // namespace

TEST(ProgressCallbackTests, InvokesCallbackWithExpectedArgs) {
    std::size_t observed_current = 0;
    std::size_t observed_total = 0;
    std::string observed_status;

    ProgressCallback callback = [&](std::size_t current, std::size_t total, const std::string& status) {
        observed_current = current;
        observed_total = total;
        observed_status = status;
    };

    runDummyWork(callback);

    EXPECT_EQ(observed_current, 3U);
    EXPECT_EQ(observed_total, 10U);
    EXPECT_EQ(observed_status, "working");
}

