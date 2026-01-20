#pragma once

#include <vector>

#include <opencv2/core.hpp>

#include "DataModels.h"

struct DropletDetectionParams {
    double gaussian_sigma = 1.0;
    int gaussian_kernel_size = 5;
    int adaptive_block_size = 21;
    double adaptive_c = 2.0;
    int morph_open_kernel = 3;
    int morph_close_kernel = 3;
    double min_area_px2 = 20.0;
    double max_area_px2 = 0.0;
    bool invert_threshold = false;
};

// Spec reference: Docs/TECHSPEC_SPLIT/03_functional_requirements.md (core droplet detection pipeline)
std::vector<Detection> detectDroplets(const cv::Mat& frame, const DropletDetectionParams& params);
