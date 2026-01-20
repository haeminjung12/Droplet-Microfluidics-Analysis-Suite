#include "FluorescenceQuantification.h"

#include <cmath>
#include <limits>
#include <stdexcept>

#include <opencv2/imgproc.hpp>

// Spec: Docs/TECHSPEC_SPLIT/05_fluorescence.md (Local annulus + global ROI fallback).

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

void validateMask(const cv::Mat& mask, const char* name, const cv::Size& size) {
    if (mask.empty()) {
        return;
    }
    if (mask.channels() != 1) {
        throw std::invalid_argument(std::string(name) + " must be single-channel");
    }
    if (mask.depth() != CV_8U) {
        throw std::invalid_argument(std::string(name) + " must be CV_8U");
    }
    if (mask.size() != size) {
        throw std::invalid_argument(std::string(name) + " must match the image size");
    }
}

cv::Mat buildDropletMask(const cv::Size& size, const std::vector<cv::Point>& contour) {
    if (contour.empty()) {
        throw std::invalid_argument("droplet_contour must not be empty");
    }
    cv::Mat droplet_mask = cv::Mat::zeros(size, CV_8U);
    cv::drawContours(droplet_mask, std::vector<std::vector<cv::Point>>{contour}, 0, cv::Scalar(255), cv::FILLED);
    if (cv::countNonZero(droplet_mask) == 0) {
        throw std::invalid_argument("droplet_contour produced an empty mask");
    }
    return droplet_mask;
}

cv::Mat computeAnnulusMask(const cv::Mat& droplet_mask, int annulus_width) {
    const int kernel_size = annulus_width * 2 + 1;
    const auto kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(kernel_size, kernel_size));
    cv::Mat dilated;
    cv::dilate(droplet_mask, dilated, kernel);

    cv::Mat annulus;
    cv::subtract(dilated, droplet_mask, annulus);
    return annulus;
}

cv::Mat excludeOtherDroplets(const cv::Mat& annulus, const cv::Mat& all_droplets_mask) {
    if (all_droplets_mask.empty()) {
        return annulus;
    }
    cv::Mat valid_annulus;
    cv::bitwise_and(annulus, ~all_droplets_mask, valid_annulus);
    return valid_annulus;
}

float meanFromMask(const cv::Mat& image, const cv::Mat& mask) {
    const cv::Scalar mean = cv::mean(image, mask);
    return static_cast<float>(mean[0]);
}

double sumFromMask(const cv::Mat& image, const cv::Mat& mask) {
    cv::Mat masked;
    image.copyTo(masked, mask);
    return cv::sum(masked)[0];
}

} // namespace

namespace FluorescenceQuantification {

FluorescenceMetrics computeFluorescenceMetrics(
    const cv::Mat& fluor_image,
    const std::vector<cv::Point>& droplet_contour,
    const cv::Mat& all_droplets_mask,
    const cv::Mat& global_background_mask,
    const BackgroundOptions& options) {
    validateSingleChannelMat(fluor_image, "fluor_image");
    validateMask(all_droplets_mask, "all_droplets_mask", fluor_image.size());
    validateMask(global_background_mask, "global_background_mask", fluor_image.size());

    if (options.annulus_width <= 0) {
        throw std::invalid_argument("BackgroundOptions.annulus_width must be > 0");
    }
    if (options.min_annulus_pixels <= 0) {
        throw std::invalid_argument("BackgroundOptions.min_annulus_pixels must be > 0");
    }
    if (options.saturated_fraction_threshold < 0.0F || options.saturated_fraction_threshold > 1.0F) {
        throw std::invalid_argument("BackgroundOptions.saturated_fraction_threshold must be in [0, 1]");
    }

    const cv::Mat droplet_mask = buildDropletMask(fluor_image.size(), droplet_contour);

    FluorescenceMetrics metrics;
    metrics.mean = meanFromMask(fluor_image, droplet_mask);
    metrics.integrated = static_cast<float>(sumFromMask(fluor_image, droplet_mask));

    double min_val = 0.0;
    double max_val = 0.0;
    cv::minMaxLoc(fluor_image, &min_val, &max_val, nullptr, nullptr, droplet_mask);
    metrics.min = static_cast<float>(min_val);
    metrics.max = static_cast<float>(max_val);

    cv::Mat annulus = computeAnnulusMask(droplet_mask, options.annulus_width);
    cv::Mat valid_annulus = excludeOtherDroplets(annulus, all_droplets_mask);
    int annulus_pixel_count = cv::countNonZero(valid_annulus);

    if (annulus_pixel_count < options.min_annulus_pixels) {
        const int expanded_width = options.annulus_width * 2;
        annulus = computeAnnulusMask(droplet_mask, expanded_width);
        valid_annulus = excludeOtherDroplets(annulus, all_droplets_mask);
        annulus_pixel_count = cv::countNonZero(valid_annulus);
    }

    float background_mean = 0.0F;
    if (annulus_pixel_count >= options.min_annulus_pixels) {
        background_mean = meanFromMask(fluor_image, valid_annulus);
        metrics.bg_method = "local_annulus";
    } else if (!global_background_mask.empty()
               && cv::countNonZero(global_background_mask) >= options.min_annulus_pixels) {
        background_mean = meanFromMask(fluor_image, global_background_mask);
        metrics.bg_method = "global_roi";
    } else {
        metrics.bg_method = "failed";
        metrics.bg_corrected_mean = metrics.mean;
        metrics.sbr = std::numeric_limits<float>::quiet_NaN();
        return metrics;
    }

    metrics.bg_corrected_mean = metrics.mean - background_mean;
    if (metrics.bg_corrected_mean < 0.0F) {
        metrics.bg_corrected_mean = 0.0F;
        metrics.bg_corrected_negative_flag = true;
    }

    if (background_mean > 0.0F && std::isfinite(background_mean)) {
        metrics.sbr = metrics.mean / background_mean;
    } else {
        metrics.sbr = std::numeric_limits<float>::quiet_NaN();
    }

    const float max_value = (fluor_image.depth() == CV_8U) ? 255.0F : 65535.0F;
    cv::Mat saturated_mask = (fluor_image == max_value);
    const int droplet_pixel_count = cv::countNonZero(droplet_mask);
    if (droplet_pixel_count > 0) {
        const int saturated_count = cv::countNonZero(saturated_mask & droplet_mask);
        if (saturated_count > droplet_pixel_count * options.saturated_fraction_threshold) {
            metrics.saturated_flag = true;
        }
    }

    return metrics;
}

} // namespace FluorescenceQuantification
