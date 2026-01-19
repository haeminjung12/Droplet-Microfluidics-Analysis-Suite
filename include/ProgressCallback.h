#pragma once

#include <cstddef>
#include <functional>
#include <string>

using ProgressCallback = std::function<void(std::size_t current, std::size_t total, const std::string& status)>;

