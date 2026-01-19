#pragma once

#include <cstddef>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>

namespace HashUtils {
    // Spec reference: Docs/TECHSPEC_SPLIT/06_data_formats.md (session JSON provenance: *_hash_sha256)

    std::string sha256Hex(std::span<const std::byte> data);
    std::string sha256Hex(std::string_view text);
    std::string sha256HexFile(const std::filesystem::path& file_path);
}

