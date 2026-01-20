#pragma once

#include <opencv2/core.hpp>

#include <vector>

#include "DataModels.h"

namespace FluorescenceQuantification {
    // Spec reference: Docs/TECHSPEC_SPLIT/05_fluorescence.md (background correction and SBR).

    struct BackgroundOptions {
        int annulus_width = 5;
        int min_annulus_pixels = 10;
        float saturated_fraction_threshold = 0.1F;
    };

    FluorescenceMetrics computeFluorescenceMetrics(
        const cv::Mat& fluor_image,
        const std::vector<cv::Point>& droplet_contour,
        const cv::Mat& all_droplets_mask,
        const cv::Mat& global_background_mask = cv::Mat(),
        const BackgroundOptions& options = {});
}
