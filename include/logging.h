#pragma once

#include <memory>

#include <spdlog/spdlog.h>

namespace logging {
    void setup();
    std::shared_ptr<spdlog::logger> get_logger();
}
