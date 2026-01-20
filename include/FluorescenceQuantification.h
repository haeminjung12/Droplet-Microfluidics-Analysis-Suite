#pragma once

#include "DataModels.h"

#include <opencv2/core.hpp>

#include <vector>

FluorescenceMetrics computeFluorescenceLocalAnnulus(
    const cv::Mat& fluor_image,
    const std::vector<cv::Point>& droplet_contour,
    const cv::Mat& all_droplets_mask,
    int annulus_width = 5);

FluorescenceMetrics computeFluorescenceGlobalRoi(
    const cv::Mat& fluor_image,
    const std::vector<cv::Point>& droplet_contour,
    const cv::Mat& global_background_mask);
