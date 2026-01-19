#pragma once

#include <cstddef>
#include <vector>

#include <opencv2/core.hpp>

enum class BackgroundSubtractionMethod {
    None,
    StaticMedian,
    RunningExponential,
};

struct StaticMedianBackgroundParams {
    std::size_t median_frame_count = 50;
};

struct RunningExponentialBackgroundParams {
    double alpha = 0.05;
};

class RunningExponentialBackgroundSubtractor {
public:
    explicit RunningExponentialBackgroundSubtractor(RunningExponentialBackgroundParams params = {});

    [[nodiscard]] cv::Mat apply(const cv::Mat& frame);
    [[nodiscard]] cv::Mat backgroundImage() const;
    void reset();

private:
    RunningExponentialBackgroundParams params_;
    cv::Mat background_accum_;
    int expected_type_ = -1;
    cv::Size expected_size_;
};

[[nodiscard]] cv::Mat subtractBackgroundNone(const cv::Mat& frame);
[[nodiscard]] cv::Mat subtractBackgroundStatic(const cv::Mat& frame, const cv::Mat& background);

[[nodiscard]] cv::Mat computeStaticMedianBackground(const std::vector<cv::Mat>& frames);

