#include "DropletDetection.h"

#include <algorithm>
#include <array>
#include <cstdint>

#include <gtest/gtest.h>

#include <opencv2/imgproc.hpp>

namespace {
cv::Mat makeSyntheticCircles() {
    cv::Mat image(200, 200, CV_8U, cv::Scalar(200));
    const int radius = 20;
    cv::circle(image, cv::Point(40, 40), radius, cv::Scalar(30), cv::FILLED);
    cv::circle(image, cv::Point(120, 40), radius, cv::Scalar(30), cv::FILLED);
    cv::circle(image, cv::Point(40, 120), radius, cv::Scalar(30), cv::FILLED);
    cv::circle(image, cv::Point(120, 120), radius, cv::Scalar(30), cv::FILLED);
    cv::circle(image, cv::Point(80, 160), radius, cv::Scalar(30), cv::FILLED);
    return image;
}

DropletDetectionParams defaultParams() {
    DropletDetectionParams params{};
    params.gaussian_sigma = 0.0;
    params.gaussian_kernel_size = 1;
    params.adaptive_block_size = 15;
    params.adaptive_c = 2.0;
    params.morph_open_kernel = 1;
    params.morph_close_kernel = 1;
    params.min_area_px2 = 600.0;
    params.max_area_px2 = 0.0;
    params.invert_threshold = true;
    return params;
}

void addSaltPepperNoise(cv::Mat& image, int count, int seed) {
    cv::RNG rng(seed);
    for (int i = 0; i < count; ++i) {
        const int x = rng.uniform(0, image.cols);
        const int y = rng.uniform(0, image.rows);
        image.at<std::uint8_t>(y, x) = (i % 2 == 0) ? 0 : 255;
    }
}

std::vector<Detection> sortedByCentroidX(std::vector<Detection> detections) {
    std::sort(detections.begin(), detections.end(),
              [](const Detection& left, const Detection& right) {
                  if (left.centroid.x == right.centroid.x) {
                      return left.centroid.y < right.centroid.y;
                  }
                  return left.centroid.x < right.centroid.x;
              });
    return detections;
}
} // namespace

TEST(DropletDetection, FindsExpectedCircles) {
    const cv::Mat image = makeSyntheticCircles();

    const auto detections = sortedByCentroidX(detectDroplets(image, defaultParams()));

    ASSERT_EQ(detections.size(), 5U);

    const std::array<cv::Point2f, 5> expected_centroids = {
        cv::Point2f(40.0F, 40.0F),
        cv::Point2f(40.0F, 120.0F),
        cv::Point2f(80.0F, 160.0F),
        cv::Point2f(120.0F, 40.0F),
        cv::Point2f(120.0F, 120.0F),
    };

    const float expected_area = static_cast<float>(CV_PI * 20.0 * 20.0);
    const float expected_diameter = 40.0F;

    for (std::size_t i = 0; i < detections.size(); ++i) {
        const auto& detection = detections[i];
        EXPECT_NEAR(detection.centroid.x, expected_centroids[i].x, 1.0F);
        EXPECT_NEAR(detection.centroid.y, expected_centroids[i].y, 1.0F);
        EXPECT_NEAR(detection.area_px2, expected_area, expected_area * 0.05F);
        EXPECT_NEAR(detection.diameter_eq_px, expected_diameter, expected_diameter * 0.05F);
        EXPECT_GT(detection.circularity, 0.85F);
    }
}

TEST(DropletDetection, IgnoresSaltPepperNoise) {
    cv::Mat noisy = makeSyntheticCircles();
    addSaltPepperNoise(noisy, 600, 42);

    const auto detections = detectDroplets(noisy, defaultParams());

    EXPECT_EQ(detections.size(), 5U);
}
