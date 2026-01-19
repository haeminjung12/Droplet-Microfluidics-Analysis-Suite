#include "logging.h"

#include <mutex>
#include <vector>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace logging {
namespace {
    std::once_flag setup_once;
    std::shared_ptr<spdlog::logger> logger_instance;
}

void setup() {
    std::call_once(setup_once, []() {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("droplet_analyzer.log", true);

        std::vector<spdlog::sink_ptr> sinks;
        sinks.push_back(console_sink);
        sinks.push_back(file_sink);

        auto logger = std::make_shared<spdlog::logger>("droplet", sinks.begin(), sinks.end());
        logger->set_level(spdlog::level::info);
        logger->flush_on(spdlog::level::info);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

        spdlog::set_default_logger(logger);

        logger_instance = std::move(logger);
    });
}

std::shared_ptr<spdlog::logger> get_logger() {
    setup();
    return logger_instance;
}
}
