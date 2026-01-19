#include "BackgroundSubtraction.h"

#include <algorithm>
#include <stdexcept>

#include <opencv2/core.hpp>

// Spec: Docs/TECHSPEC_SPLIT/07_gui_requirements.md (Background Subtraction).

namespace {

void validateSingleChannelMat(const cv::Mat& mat, const char* name) {
    if (mat.empty()) {
        throw std::invalid_argument(std::string(name) + " must not be empty");
    }
    if (mat.channels() != 1) {
        throw std::invalid_argument(std::string(name) + " must be single-channel");
    }
    if (mat.depth() != CV_8U && mat.depth() != CV_16U) {
        throw std::invalid_argument(std::string(name) + " must be CV_8U or CV_16U");
    }
}

void validateSameShapeAndType(const cv::Mat& a, const cv::Mat& b, const char* a_name, const char* b_name) {
    validateSingleChannelMat(a, a_name);
    validateSingleChannelMat(b, b_name);
    if (a.type() != b.type() || a.size() != b.size()) {
        throw std::invalid_argument(std::string(a_name) + " and " + b_name + " must have the same size and type");
    }
}

} // namespace

RunningExponentialBackgroundSubtractor::RunningExponentialBackgroundSubtractor(RunningExponentialBackgroundParams params)
    : params_(params) {
    if (!(params_.alpha > 0.0 && params_.alpha <= 1.0)) {
        throw std::invalid_argument("RunningExponentialBackgroundParams.alpha must be in (0, 1]");
    }
}

cv::Mat RunningExponentialBackgroundSubtractor::apply(const cv::Mat& frame) {
    validateSingleChannelMat(frame, "frame");

    if (background_accum_.empty()) {
        expected_type_ = frame.type();
        expected_size_ = frame.size();
        frame.convertTo(background_accum_, CV_32F);
        return cv::Mat::zeros(frame.size(), frame.type());
    }

    if (frame.type() != expected_type_ || frame.size() != expected_size_) {
        throw std::invalid_argument("frame must match the type and size of the first frame passed to apply()");
    }

    cv::Mat frame_f32;
    frame.convertTo(frame_f32, CV_32F);
    background_accum_ = (1.0 - params_.alpha) * background_accum_ + params_.alpha * frame_f32;

    cv::Mat background_u;
    background_accum_.convertTo(background_u, frame.type());

    cv::Mat foreground;
    cv::absdiff(frame, background_u, foreground);
    return foreground;
}

cv::Mat RunningExponentialBackgroundSubtractor::backgroundImage() const {
    if (background_accum_.empty()) {
        return {};
    }

    cv::Mat background_u;
    background_accum_.convertTo(background_u, expected_type_);
    return background_u;
}

void RunningExponentialBackgroundSubtractor::reset() {
    background_accum_.release();
    expected_type_ = -1;
    expected_size_ = {};
}

cv::Mat subtractBackgroundNone(const cv::Mat& frame) {
    validateSingleChannelMat(frame, "frame");
    return frame.clone();
}

cv::Mat subtractBackgroundStatic(const cv::Mat& frame, const cv::Mat& background) {
    validateSameShapeAndType(frame, background, "frame", "background");
    cv::Mat out;
    cv::absdiff(frame, background, out);
    return out;
}

cv::Mat computeStaticMedianBackground(const std::vector<cv::Mat>& frames) {
    if (frames.empty()) {
        throw std::invalid_argument("frames must not be empty");
    }

    validateSingleChannelMat(frames.front(), "frames[0]");
    const auto type = frames.front().type();
    const auto size = frames.front().size();

    for (std::size_t index = 1; index < frames.size(); ++index) {
        validateSameShapeAndType(frames.front(), frames[index], "frames[0]", "frames[i]");
    }

    cv::Mat background(size, type);
    std::vector<int> scratch;
    scratch.reserve(frames.size());

    for (int y = 0; y < size.height; ++y) {
        for (int x = 0; x < size.width; ++x) {
            scratch.clear();
            scratch.resize(frames.size());

            if (frames.front().depth() == CV_8U) {
                for (std::size_t i = 0; i < frames.size(); ++i) {
                    scratch[i] = frames[i].at<std::uint8_t>(y, x);
                }
            } else {
                for (std::size_t i = 0; i < frames.size(); ++i) {
                    scratch[i] = frames[i].at<std::uint16_t>(y, x);
                }
            }

            const auto median_index = scratch.begin() + static_cast<std::ptrdiff_t>(scratch.size() / 2);
            std::nth_element(scratch.begin(), median_index, scratch.end());
            const auto median_value = *median_index;

            if (frames.front().depth() == CV_8U) {
                background.at<std::uint8_t>(y, x) = static_cast<std::uint8_t>(median_value);
            } else {
                background.at<std::uint16_t>(y, x) = static_cast<std::uint16_t>(median_value);
            }
        }
    }

    return background;
}

