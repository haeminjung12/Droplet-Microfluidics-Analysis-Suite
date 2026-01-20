#include "FluorescenceQuantification.h"

#include <opencv2/imgproc.hpp>

#include <cmath>
#include <limits>

namespace {
cv::Mat createDropletMask(const cv::Mat& image, const std::vector<cv::Point>& contour) {
    cv::Mat droplet_mask = cv::Mat::zeros(image.size(), CV_8U);
    if (!contour.empty()) {
        cv::drawContours(droplet_mask, std::vector<std::vector<cv::Point>>{contour}, 0, cv::Scalar(255), cv::FILLED);
    }
    return droplet_mask;
}

int countMaskedPixels(const cv::Mat& mask) {
    return cv::countNonZero(mask);
}

float computeMaskedMean(const cv::Mat& image, const cv::Mat& mask) {
    cv::Scalar mean_value = cv::mean(image, mask);
    return static_cast<float>(mean_value[0]);
}

float computeMaskedIntegrated(const cv::Mat& image, const cv::Mat& mask) {
    const int pixel_count = countMaskedPixels(mask);
    if (pixel_count <= 0) {
        return 0.0F;
    }
    const float mean_value = computeMaskedMean(image, mask);
    return mean_value * static_cast<float>(pixel_count);
}

void fillMinMax(const cv::Mat& image, const cv::Mat& mask, FluorescenceMetrics& metrics) {
    double min_value = 0.0;
    double max_value = 0.0;
    if (countMaskedPixels(mask) > 0) {
        cv::minMaxLoc(image, &min_value, &max_value, nullptr, nullptr, mask);
    }
    metrics.min = static_cast<float>(min_value);
    metrics.max = static_cast<float>(max_value);
}

void applyBackgroundCorrection(FluorescenceMetrics& metrics, float background_mean, const char* method) {
    metrics.bg_method = method;
    metrics.bg_corrected_mean = metrics.mean - background_mean;
    if (metrics.bg_corrected_mean < 0.0F) {
        metrics.bg_corrected_mean = 0.0F;
        metrics.bg_corrected_negative_flag = true;
    }
    if (background_mean > 0.0F) {
        metrics.sbr = metrics.mean / background_mean;
    } else {
        metrics.sbr = std::numeric_limits<float>::quiet_NaN();
    }
}

void setSaturationFlag(const cv::Mat& image, const cv::Mat& droplet_mask, FluorescenceMetrics& metrics) {
    const int droplet_pixel_count = countMaskedPixels(droplet_mask);
    if (droplet_pixel_count <= 0) {
        return;
    }

    const int depth = image.depth();
    float max_value = 0.0F;
    if (depth == CV_8U) {
        max_value = 255.0F;
    } else if (depth == CV_16U) {
        max_value = 65535.0F;
    } else {
        return;
    }

    cv::Mat saturated_mask = (image == max_value);
    cv::Mat saturated_in_droplet;
    cv::bitwise_and(saturated_mask, droplet_mask, saturated_in_droplet);
    const int saturated_count = countMaskedPixels(saturated_in_droplet);
    if (static_cast<float>(saturated_count) > static_cast<float>(droplet_pixel_count) * 0.1F) {
        metrics.saturated_flag = true;
    }
}
}  // namespace

FluorescenceMetrics computeFluorescenceLocalAnnulus(
    const cv::Mat& fluor_image,
    const std::vector<cv::Point>& droplet_contour,
    const cv::Mat& all_droplets_mask,
    int annulus_width) {
    // Spec: Docs/TECHSPEC_SPLIT/05_fluorescence.md
    FluorescenceMetrics metrics{};
    cv::Mat droplet_mask = createDropletMask(fluor_image, droplet_contour);

    metrics.mean = computeMaskedMean(fluor_image, droplet_mask);
    metrics.integrated = computeMaskedIntegrated(fluor_image, droplet_mask);
    fillMinMax(fluor_image, droplet_mask, metrics);

    cv::Mat dilated;
    cv::Mat annulus;
    cv::Mat valid_annulus;
    int annulus_pixel_count = 0;

    for (int attempt = 0; attempt < 2; ++attempt) {
        const int kernel_size = annulus_width * 2 + 1;
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(kernel_size, kernel_size));
        cv::dilate(droplet_mask, dilated, kernel);
        cv::subtract(dilated, droplet_mask, annulus);

        cv::Mat inverted_droplets;
        cv::bitwise_not(all_droplets_mask, inverted_droplets);
        cv::bitwise_and(annulus, inverted_droplets, valid_annulus);
        annulus_pixel_count = countMaskedPixels(valid_annulus);

        if (annulus_pixel_count >= 10) {
            break;
        }
        annulus_width *= 2;
    }

    if (annulus_pixel_count < 10) {
        metrics.bg_method = "failed";
        metrics.bg_corrected_mean = metrics.mean;
        metrics.sbr = std::numeric_limits<float>::quiet_NaN();
        return metrics;
    }

    const float background_mean = computeMaskedMean(fluor_image, valid_annulus);
    applyBackgroundCorrection(metrics, background_mean, "local_annulus");
    setSaturationFlag(fluor_image, droplet_mask, metrics);

    return metrics;
}

FluorescenceMetrics computeFluorescenceGlobalRoi(
    const cv::Mat& fluor_image,
    const std::vector<cv::Point>& droplet_contour,
    const cv::Mat& global_background_mask) {
    // Spec: Docs/TECHSPEC_SPLIT/05_fluorescence.md
    FluorescenceMetrics metrics{};
    cv::Mat droplet_mask = createDropletMask(fluor_image, droplet_contour);

    metrics.mean = computeMaskedMean(fluor_image, droplet_mask);
    metrics.integrated = computeMaskedIntegrated(fluor_image, droplet_mask);
    fillMinMax(fluor_image, droplet_mask, metrics);

    const int bg_pixel_count = countMaskedPixels(global_background_mask);
    if (bg_pixel_count < 1) {
        metrics.bg_method = "failed";
        metrics.bg_corrected_mean = metrics.mean;
        metrics.sbr = std::numeric_limits<float>::quiet_NaN();
        return metrics;
    }

    const float background_mean = computeMaskedMean(fluor_image, global_background_mask);
    applyBackgroundCorrection(metrics, background_mean, "global_roi");
    setSaturationFlag(fluor_image, droplet_mask, metrics);

    return metrics;
}
