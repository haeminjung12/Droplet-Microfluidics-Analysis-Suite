#include "FluorescenceQuantification.h"

#include <cmath>
#include <vector>

#include <gtest/gtest.h>
#include <opencv2/imgproc.hpp>

namespace {

std::vector<cv::Point> squareContour(int x0, int y0, int x1, int y1) {
    return {cv::Point(x0, y0), cv::Point(x1, y0), cv::Point(x1, y1), cv::Point(x0, y1)};
}

} // namespace

TEST(FluorescenceQuantification, LocalAnnulusComputesExpectedMetrics) {
    cv::Mat image(11, 11, CV_16UC1, cv::Scalar(100));
    const auto contour = squareContour(4, 4, 6, 6);

    cv::Mat droplet_mask = cv::Mat::zeros(image.size(), CV_8U);
    cv::drawContours(droplet_mask, std::vector<std::vector<cv::Point>>{contour}, 0, cv::Scalar(255), cv::FILLED);
    image.setTo(200, droplet_mask);

    cv::Mat all_droplets_mask = cv::Mat::zeros(image.size(), CV_8U);

    const auto metrics = FluorescenceQuantification::computeFluorescenceMetrics(
        image,
        contour,
        all_droplets_mask,
        {},
        FluorescenceQuantification::BackgroundOptions{.annulus_width = 1});

    EXPECT_NEAR(metrics.mean, 200.0F, 0.01F);
    EXPECT_NEAR(metrics.integrated, 1800.0F, 0.01F);
    EXPECT_NEAR(metrics.min, 200.0F, 0.01F);
    EXPECT_NEAR(metrics.max, 200.0F, 0.01F);
    EXPECT_NEAR(metrics.bg_corrected_mean, 100.0F, 0.01F);
    EXPECT_NEAR(metrics.sbr, 2.0F, 0.01F);
    EXPECT_EQ(metrics.bg_method, "local_annulus");
}

TEST(FluorescenceQuantification, FallsBackToGlobalBackgroundWhenAnnulusInvalid) {
    cv::Mat image(11, 11, CV_16UC1, cv::Scalar(100));
    const auto contour = squareContour(4, 4, 6, 6);

    cv::Mat droplet_mask = cv::Mat::zeros(image.size(), CV_8U);
    cv::drawContours(droplet_mask, std::vector<std::vector<cv::Point>>{contour}, 0, cv::Scalar(255), cv::FILLED);
    image.setTo(200, droplet_mask);

    cv::Mat all_droplets_mask(image.size(), CV_8U, cv::Scalar(255));
    cv::Mat global_bg_mask = cv::Mat::zeros(image.size(), CV_8U);
    cv::rectangle(global_bg_mask, cv::Rect(0, 0, 3, 3), cv::Scalar(255), cv::FILLED);

    const auto metrics = FluorescenceQuantification::computeFluorescenceMetrics(
        image,
        contour,
        all_droplets_mask,
        global_bg_mask,
        FluorescenceQuantification::BackgroundOptions{.annulus_width = 1});

    EXPECT_NEAR(metrics.bg_corrected_mean, 100.0F, 0.01F);
    EXPECT_NEAR(metrics.sbr, 2.0F, 0.01F);
    EXPECT_EQ(metrics.bg_method, "global_roi");
}

TEST(FluorescenceQuantification, ReportsFailedWhenNoValidBackgroundAvailable) {
    cv::Mat image(11, 11, CV_16UC1, cv::Scalar(100));
    const auto contour = squareContour(4, 4, 6, 6);

    cv::Mat droplet_mask = cv::Mat::zeros(image.size(), CV_8U);
    cv::drawContours(droplet_mask, std::vector<std::vector<cv::Point>>{contour}, 0, cv::Scalar(255), cv::FILLED);
    image.setTo(200, droplet_mask);

    cv::Mat all_droplets_mask(image.size(), CV_8U, cv::Scalar(255));

    const auto metrics = FluorescenceQuantification::computeFluorescenceMetrics(
        image,
        contour,
        all_droplets_mask,
        {},
        FluorescenceQuantification::BackgroundOptions{.annulus_width = 1});

    EXPECT_EQ(metrics.bg_method, "failed");
    EXPECT_NEAR(metrics.bg_corrected_mean, metrics.mean, 0.01F);
    EXPECT_TRUE(std::isnan(metrics.sbr));
}
