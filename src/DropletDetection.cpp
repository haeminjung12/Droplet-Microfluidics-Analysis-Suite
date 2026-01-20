#include "DropletDetection.h"

#include <algorithm>

#include <opencv2/imgproc.hpp>

#include "MathUtils.h"

namespace {
int ensureOddKernel(int value, int fallback) {
    if (value <= 0) {
        value = fallback;
    }
    if (value < 3) {
        value = 3;
    }
    if (value % 2 == 0) {
        value += 1;
    }
    return value;
}

cv::Mat toGrayscale8(const cv::Mat& frame) {
    cv::Mat gray;
    if (frame.channels() == 1) {
        gray = frame;
    } else {
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    }

    if (gray.depth() == CV_8U) {
        return gray;
    }

    cv::Mat converted;
    if (gray.depth() == CV_16U) {
        gray.convertTo(converted, CV_8U, 255.0 / 65535.0);
    } else {
        gray.convertTo(converted, CV_8U);
    }
    return converted;
}

cv::RotatedRect fitEllipseSafe(const std::vector<cv::Point>& contour, const cv::Rect& bounds) {
    if (contour.size() >= 5U) {
        return cv::fitEllipse(contour);
    }

    const cv::Point2f center(bounds.x + bounds.width / 2.0F, bounds.y + bounds.height / 2.0F);
    return cv::RotatedRect(center, cv::Size2f(static_cast<float>(bounds.width),
                                              static_cast<float>(bounds.height)),
                           0.0F);
}
} // namespace

std::vector<Detection> detectDroplets(const cv::Mat& frame, const DropletDetectionParams& params) {
    std::vector<Detection> detections;
    if (frame.empty()) {
        return detections;
    }

    int gaussian_kernel = params.gaussian_kernel_size;
    if (gaussian_kernel <= 1) {
        gaussian_kernel = 1;
    } else {
        gaussian_kernel = ensureOddKernel(gaussian_kernel, 5);
    }
    const int adaptive_block = ensureOddKernel(params.adaptive_block_size, 21);
    const int open_kernel = params.morph_open_kernel > 1 ? ensureOddKernel(params.morph_open_kernel, 3) : 0;
    const int close_kernel = params.morph_close_kernel > 1 ? ensureOddKernel(params.morph_close_kernel, 3) : 0;

    cv::Mat gray = toGrayscale8(frame);
    cv::Mat blurred = gray;
    if (params.gaussian_sigma > 0.0 || gaussian_kernel > 1) {
        cv::GaussianBlur(gray, blurred, cv::Size(gaussian_kernel, gaussian_kernel), params.gaussian_sigma);
    }

    cv::Mat binary;
    const int threshold_type = params.invert_threshold ? cv::THRESH_BINARY_INV : cv::THRESH_BINARY;
    cv::adaptiveThreshold(blurred, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, threshold_type,
                          adaptive_block, params.adaptive_c);

    if (open_kernel > 1) {
        cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(open_kernel, open_kernel));
        cv::morphologyEx(binary, binary, cv::MORPH_OPEN, element);
    }
    if (close_kernel > 1) {
        cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(close_kernel, close_kernel));
        cv::morphologyEx(binary, binary, cv::MORPH_CLOSE, element);
    }

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    detections.reserve(contours.size());
    for (const auto& contour : contours) {
        const double area = cv::contourArea(contour);
        if (area < params.min_area_px2) {
            continue;
        }
        if (params.max_area_px2 > 0.0 && area > params.max_area_px2) {
            continue;
        }

        const double perimeter = cv::arcLength(contour, true);
        const cv::Moments moments = cv::moments(contour);
        if (moments.m00 <= 0.0) {
            continue;
        }

        const cv::Rect bounds = cv::boundingRect(contour);
        const cv::RotatedRect ellipse = fitEllipseSafe(contour, bounds);
        const float major_axis = std::max(ellipse.size.width, ellipse.size.height);
        const float minor_axis = std::min(ellipse.size.width, ellipse.size.height);

        Detection detection{};
        detection.centroid = cv::Point2f(static_cast<float>(moments.m10 / moments.m00),
                                         static_cast<float>(moments.m01 / moments.m00));
        detection.area_px2 = static_cast<float>(area);
        detection.perimeter_px = static_cast<float>(perimeter);
        detection.diameter_eq_px = static_cast<float>(MathUtils::diameterFromArea(area));
        detection.major_axis_px = major_axis;
        detection.minor_axis_px = minor_axis;
        detection.angle_deg = ellipse.angle;
        detection.circularity = static_cast<float>(MathUtils::calculateCircularity(area, perimeter));
        detection.aspect_ratio = static_cast<float>(MathUtils::aspectRatio(major_axis, minor_axis));
        detection.bounding_box = bounds;
        detection.contour = contour;

        detections.push_back(std::move(detection));
    }

    return detections;
}
