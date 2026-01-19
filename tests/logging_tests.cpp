#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "logging.h"

TEST(Logging, SetupCreatesConsoleAndFileSinks) {
    const auto log_path = std::filesystem::current_path() / "droplet_analyzer.log";
    std::error_code remove_error;
    std::filesystem::remove(log_path, remove_error);

    logging::setup();
    const auto logger = logging::get_logger();
    ASSERT_NE(logger, nullptr);

    bool has_file_sink = false;
    bool has_console_sink = false;
    for(const auto& sink : logger->sinks()) {
        if(dynamic_cast<spdlog::sinks::basic_file_sink_mt*>(sink.get()) != nullptr) {
            has_file_sink = true;
        }
        if(dynamic_cast<spdlog::sinks::stdout_color_sink_mt*>(sink.get()) != nullptr) {
            has_console_sink = true;
        }
    }

    EXPECT_TRUE(has_file_sink);
    EXPECT_TRUE(has_console_sink);

    logger->info("logging test message");
    logger->flush();

    EXPECT_TRUE(std::filesystem::exists(log_path));
}
