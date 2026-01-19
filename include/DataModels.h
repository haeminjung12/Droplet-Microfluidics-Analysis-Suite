#pragma once

#include <opencv2/core.hpp>

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

struct FluorescenceMetrics {
    float mean = 0.0F;
    float integrated = 0.0F;
    float min = 0.0F;
    float max = 0.0F;
    float bg_corrected_mean = 0.0F;
    float sbr = 0.0F;
    std::string bg_method;
};

struct Detection {
    std::size_t droplet_id = 0;
    cv::Point2f centroid;
    float area_px2 = 0.0F;
    float perimeter_px = 0.0F;
    float diameter_eq_px = 0.0F;
    float major_axis_px = 0.0F;
    float minor_axis_px = 0.0F;
    float angle_deg = 0.0F;
    float circularity = 0.0F;
    float aspect_ratio = 0.0F;
    cv::Rect bounding_box;
    std::vector<cv::Point> contour;
    std::unordered_map<std::string, FluorescenceMetrics> fluorescence;
};

struct FrameDetections {
    std::size_t frame_index_logical = 0;
    double timestamp_inferred_s = 0.0;
    std::vector<Detection> detections;
};

struct Track {
    std::size_t track_id = 0;
    std::vector<std::size_t> droplet_ids;
    std::vector<std::size_t> frame_indices;
    std::vector<cv::Point2f> centroids;
    std::vector<double> timestamps;
    std::vector<float> velocities;
    bool crossed_line = false;
    std::size_t line_crossing_frame = 0;
};

