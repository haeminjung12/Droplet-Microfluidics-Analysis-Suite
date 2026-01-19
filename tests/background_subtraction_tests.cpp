#include <cstddef>
#include <vector>

#include <gtest/gtest.h>

#include <opencv2/core.hpp>

#include "BackgroundSubtraction.h"

TEST(BackgroundSubtraction, NoneModeReturnsIdenticalImage) {
    cv::Mat input(8, 8, CV_8UC1, cv::Scalar(7));
    input.at<std::uint8_t>(2, 3) = 123;

    const auto output = subtractBackgroundNone(input);

    EXPECT_EQ(output.type(), input.type());
    EXPECT_EQ(output.size(), input.size());
    EXPECT_EQ(cv::countNonZero(output != input), 0);
}

TEST(BackgroundSubtraction, StaticMedianComputesConstantBackground) {
    std::vector<cv::Mat> frames;
    frames.reserve(60);

    for (int i = 0; i < 60; ++i) {
        cv::Mat frame(32, 32, CV_8UC1, cv::Scalar(10));
        const int x = i % 32;
        frame.at<std::uint8_t>(16, x) = 250;
        frames.push_back(frame);
    }

    const auto background = computeStaticMedianBackground(frames);

    EXPECT_EQ(background.type(), CV_8UC1);
    EXPECT_EQ(background.size(), cv::Size(32, 32));
    EXPECT_EQ(cv::countNonZero(background != cv::Scalar(10)), 0);

    const auto foreground = subtractBackgroundStatic(frames.front(), background);
    EXPECT_GT(cv::countNonZero(foreground), 0);
    EXPECT_EQ(foreground.at<std::uint8_t>(16, 0), 240);
}

TEST(BackgroundSubtraction, RunningBackgroundConvergesAndHighlightsMovingObject) {
    RunningExponentialBackgroundSubtractor subtractor(RunningExponentialBackgroundParams{.alpha = 0.05});

    cv::Mat last_foreground;
    for (int i = 0; i < 120; ++i) {
        cv::Mat frame(32, 32, CV_8UC1, cv::Scalar(0));
        const int x = i % 32;
        frame.at<std::uint8_t>(16, x) = 255;
        last_foreground = subtractor.apply(frame);
    }

    const auto background = subtractor.backgroundImage();
    ASSERT_FALSE(background.empty());

    EXPECT_LT(background.at<std::uint8_t>(0, 0), 5);
    EXPECT_GT(cv::countNonZero(last_foreground), 0);
}

