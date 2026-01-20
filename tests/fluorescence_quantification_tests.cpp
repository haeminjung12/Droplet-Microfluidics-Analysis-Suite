#include "FluorescenceQuantification.h"

#include <gtest/gtest.h>

#include <opencv2/imgproc.hpp>

#include <cmath>

namespace {
std::vector<cv::Point> makeRectangleContour(int x, int y, int width, int height) {
    std::vector<cv::Point> contour;
    contour.emplace_back(x, y);
    contour.emplace_back(x + width - 1, y);
    contour.emplace_back(x + width - 1, y + height - 1);
    contour.emplace_back(x, y + height - 1);
    return contour;
}

void fillRectangle(cv::Mat& image, int x, int y, int width, int height, int value) {
    cv::rectangle(image, cv::Rect(x, y, width, height), cv::Scalar(value), cv::FILLED);
}
}  // namespace

TEST(FluorescenceQuantification, LocalAnnulusComputesMetrics) {
    cv::Mat image = cv::Mat::zeros(10, 10, CV_8U);
    image.setTo(2);
    fillRectangle(image, 3, 3, 4, 4, 10);

    const auto contour = makeRectangleContour(3, 3, 4, 4);

    cv::Mat all_droplets_mask = cv::Mat::zeros(image.size(), CV_8U);
    fillRectangle(all_droplets_mask, 3, 3, 4, 4, 255);

    FluorescenceMetrics metrics = computeFluorescenceLocalAnnulus(image, contour, all_droplets_mask, 1);

    EXPECT_FLOAT_EQ(metrics.mean, 10.0F);
    EXPECT_FLOAT_EQ(metrics.integrated, 160.0F);
    EXPECT_FLOAT_EQ(metrics.min, 10.0F);
    EXPECT_FLOAT_EQ(metrics.max, 10.0F);
    EXPECT_FLOAT_EQ(metrics.bg_corrected_mean, 8.0F);
    EXPECT_FLOAT_EQ(metrics.sbr, 5.0F);
    EXPECT_EQ(metrics.bg_method, "local_annulus");
}

TEST(FluorescenceQuantification, LocalAnnulusTooSmallFallsBack) {
    cv::Mat image = cv::Mat::zeros(3, 3, CV_8U);
    fillRectangle(image, 0, 0, 3, 3, 5);

    const auto contour = makeRectangleContour(0, 0, 3, 3);
    cv::Mat all_droplets_mask = cv::Mat::zeros(image.size(), CV_8U);
    fillRectangle(all_droplets_mask, 0, 0, 3, 3, 255);

    FluorescenceMetrics metrics = computeFluorescenceLocalAnnulus(image, contour, all_droplets_mask, 1);

    EXPECT_EQ(metrics.bg_method, "failed");
    EXPECT_FLOAT_EQ(metrics.bg_corrected_mean, metrics.mean);
    EXPECT_TRUE(std::isnan(metrics.sbr));
}

TEST(FluorescenceQuantification, GlobalRoiComputesMetrics) {
    cv::Mat image = cv::Mat::zeros(10, 10, CV_8U);
    image.setTo(2);
    fillRectangle(image, 3, 3, 4, 4, 12);

    const auto contour = makeRectangleContour(3, 3, 4, 4);
    cv::Mat background_mask = cv::Mat::zeros(image.size(), CV_8U);
    fillRectangle(background_mask, 0, 0, 2, 2, 255);

    FluorescenceMetrics metrics = computeFluorescenceGlobalRoi(image, contour, background_mask);

    EXPECT_FLOAT_EQ(metrics.mean, 12.0F);
    EXPECT_FLOAT_EQ(metrics.integrated, 192.0F);
    EXPECT_FLOAT_EQ(metrics.bg_corrected_mean, 10.0F);
    EXPECT_FLOAT_EQ(metrics.sbr, 6.0F);
    EXPECT_EQ(metrics.bg_method, "global_roi");
}
