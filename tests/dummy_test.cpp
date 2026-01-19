#include "libdroplet.h"

#include <gtest/gtest.h>

TEST(LibDropletAdd, AddsTwoIntegers) {
  EXPECT_EQ(droplet::add(1, 2), 3);
}
