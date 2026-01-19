#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "HashUtils.h"

TEST(HashUtils, Sha256HexFileMatchesKnownHash) {
    const auto temp_dir = std::filesystem::temp_directory_path();
    const auto test_path = temp_dir / "droplet_hashutils_test_hello_world.txt";

    {
        std::ofstream out(test_path, std::ios::binary | std::ios::trunc);
        ASSERT_TRUE(out.good());
        out << "hello world";
    }

    const auto digest = HashUtils::sha256HexFile(test_path);
    EXPECT_EQ(digest, "b94d27b9934d3e08a52e52d7da7dabfac484efe37a5380ee9088f7ace2efcde9");

    std::error_code remove_error;
    std::filesystem::remove(test_path, remove_error);
}

TEST(HashUtils, Sha256HexStringMatchesKnownHash) {
    const std::string json = R"({"a":1})";
    const auto digest = HashUtils::sha256Hex(json);
    EXPECT_EQ(digest, "015abd7f5cc57a2dd94b7590f04ad8084273905ee33ec5cebeae62276a97f862");
}

